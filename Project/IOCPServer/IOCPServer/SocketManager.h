#pragma once

#include <WinSock2.h>
#include <vector>
#include <memory>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include "__NOH.h"
#include "Singleton.h"
#include "IOCPHandler.h"

namespace NOH
{
    class CSession;
    typedef struct st_SOCKET_INFO
    {
        friend class CAcceptWorker;
        friend class CLanWorker;
        template<class T> friend class CSocketManager;

    private:
        OVERLAPPED                  SocketOverlapped;
        SOCKET						Socket;
        CSession *                  pSession;
        SOCKET_TYPE                 SocketType;
        bool                        bFirst;
        char						cOutBuff[static_cast<int>(SESSION::BUFF_SOCKET_64)];

    public:
        const SOCKET        GetSocket(void) const               { return Socket; }
        const SOCKET_TYPE   GetSocketType(void) const           { return SocketType; }
        CSession &          GetSession(void)                    { return *pSession; }
        char *              GetOutBuff(void)                    { return cOutBuff; }
        void                SetSession(CSession & _pSession)    { pSession = &_pSession; }

        explicit st_SOCKET_INFO(SOCKET_TYPE _SocketType = SOCKET_TYPE::DEFAULT) : SocketOverlapped({ 0 }), Socket(INVALID_SOCKET), pSession(nullptr), SocketType(_SocketType), bFirst(true)
        {
            memset(&cOutBuff, 0, sizeof(cOutBuff));
        }
    } SOCKET_INFO;

    template<class T>
    class CSocketManager : public CSingleton<CSocketManager<T>>
    {
    public:
        CSocketManager();
        virtual ~CSocketManager();

        bool Initialize(const char & cIP, const WORD & wPort, const SOCKET_TYPE SocketType, const int iSocketCnt);
        bool AcceptEx(SOCKET_INFO & SocketInfo);
        bool DisconnectEx(SOCKET_INFO & SocketInfo);
        void DisconnectSocket(SOCKET_INFO & SocketInfo, const DISCONNECT_TYPE & Type);
        void Close(void);

        SOCKET GetListenSocket(void) { return m_ListenSocket; }
        SOCKET_INFO * GetEmptySocketInfo(void) { return m_spEmptySocketInfo.get(); }

    private:
        bool CreateLinstenSocket(const char & cIP, const WORD & wPort);
        bool CreateAcceptSocket(SOCKET_TYPE SocketType);

    private:
        HANDLE                                          m_hAcceptIOCP;
        SOCKET                                          m_ListenSocket;
        int                                             m_iSocketCnt;
        std::vector<std::unique_ptr<SOCKET_INFO>>       m_vecSocketInfo;
        std::unique_ptr<SOCKET_INFO>                    m_spEmptySocketInfo;
        LPFN_ACCEPTEX                                   m_lpfnAcceptEx;
    };

    template<class T>
    inline CSocketManager<T>::CSocketManager()
        : m_ListenSocket(INVALID_SOCKET), m_hAcceptIOCP(INVALID_HANDLE_VALUE), m_iSocketCnt(0), m_spEmptySocketInfo(std::make_unique<SOCKET_INFO>()), m_lpfnAcceptEx(nullptr)
    {

    }

    template<class T>
    inline CSocketManager<T>::~CSocketManager()
    {
        WSACleanup();
    }

    template<class T>
    inline bool CSocketManager<T>::Initialize(const char & cIP, const WORD & wPort, const SOCKET_TYPE SocketType, const int iSocketCnt)
    {
        m_iSocketCnt = iSocketCnt;

        if (!CreateLinstenSocket(cIP, wPort))
            return false;

        if (!CreateAcceptSocket(SocketType))
            return false;

        return true;
    }

    template<class T>
    inline bool CSocketManager<T>::AcceptEx(SOCKET_INFO & SocketInfo)
    {
        DWORD _dwbytes = 0;

        if (FALSE == m_lpfnAcceptEx(m_ListenSocket, SocketInfo.Socket, SocketInfo.cOutBuff, static_cast<int>(SESSION::BUFF_SOCKET_64) - ((sizeof (SOCKADDR_IN) + 16) * 2), sizeof (SOCKADDR_IN) + 16, sizeof (SOCKADDR_IN) + 16, &_dwbytes, &SocketInfo.SocketOverlapped))
        {
            int _ierrorcode = WSAGetLastError();

            if (ERROR_IO_PENDING != _ierrorcode)
            {
                WCHAR szError[200] = { 0 };
                swprintf_s(szError, L"SocketManager[AcceptEx Error] - ErrorCode: %d ListenSocket: %I64u, Socket: %I64u", _ierrorcode, static_cast<unsigned __int64>(m_ListenSocket), static_cast<unsigned __int64>(SocketInfo.Socket));
                SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"%s", szError);
                return false;
            }
        }

        InterlockedIncrement(&g_lRemainingSocketCount);
        return true;
    }

    template<class T>
    inline bool CSocketManager<T>::DisconnectEx(SOCKET_INFO & SocketInfo)
    {
        LPFN_DISCONNECTEX _lpfndisconnectex = nullptr;
        GUID _guiddisconnectex = WSAID_DISCONNECTEX;
        DWORD _dwbytes = 0;

        if (SOCKET_ERROR == WSAIoctl(SocketInfo.Socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &_guiddisconnectex, sizeof(_guiddisconnectex), &_lpfndisconnectex, sizeof(_lpfndisconnectex), &_dwbytes, nullptr, nullptr))
            return false;

        if (FALSE == _lpfndisconnectex(SocketInfo.Socket, &SocketInfo.SocketOverlapped, TF_REUSE_SOCKET, 0))
        {
            int _ierrorcode = WSAGetLastError();

            if (ERROR_IO_PENDING != _ierrorcode && WSAENOTCONN != _ierrorcode)
            {
                WCHAR szError[200] = { 0 };
                swprintf_s(szError, L"SocketManager[DisconnectEx Error]  ErrorCode: %d, Socket: %I64u", _ierrorcode, static_cast<unsigned __int64>(SocketInfo.Socket));
                SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"%s", szError);
                return false;
            }
        }

        return true;
    }

    template<class T>
    inline void CSocketManager<T>::DisconnectSocket(SOCKET_INFO & SocketInfo, const DISCONNECT_TYPE & Type)
    {
        switch (Type)
        {
        case DISCONNECT_TYPE::FORCED:
            shutdown(SocketInfo.Socket, SD_BOTH);
            break;
        case DISCONNECT_TYPE::GRACEFUL:
            shutdown(SocketInfo.Socket, SD_SEND);
            break;
        case DISCONNECT_TYPE::REUSESOCKET:
            DisconnectEx(SocketInfo);
            break;
        }
    }

    template<class T>
    inline void CSocketManager<T>::Close(void)
    {
        closesocket(m_ListenSocket);
        m_ListenSocket = INVALID_SOCKET;
    }

    
    template<class T>
    inline bool CSocketManager<T>::CreateLinstenSocket(const char & cIP, const WORD & wPort)
    {
        // 윈속 초기화
        WSADATA _wsadata;
        if (0 != WSAStartup(MAKEWORD(2, 2), &_wsadata))
            return false;

        m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

        if (INVALID_SOCKET == m_ListenSocket)
            return false;

        LINGER  _linger = { 0 };
        _linger.l_onoff = 1;  
        _linger.l_linger = 0;

        if (SOCKET_ERROR == setsockopt(m_ListenSocket, SOL_SOCKET, SO_LINGER, (CHAR*)&_linger, sizeof(_linger)))
            return false;

        bool _breuse = true;
        if (SOCKET_ERROR == setsockopt(m_ListenSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&_breuse), sizeof(_breuse)))
            return false;

        bool _bconditional = true;
        if (SOCKET_ERROR == setsockopt(m_ListenSocket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, reinterpret_cast<const char *>(&_bconditional), sizeof(_bconditional)))
            return false;

        bool _bnagle  = true;
        if (SOCKET_ERROR == setsockopt(m_ListenSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&_bnagle), sizeof(_bnagle)))
            return false;

        SOCKADDR_IN serveraddr;
        memset(&serveraddr, 0, sizeof(serveraddr));

        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = htons(wPort);

        InetPtonA(AF_INET, &cIP, &serveraddr.sin_addr.S_un.S_addr);

        if (SOCKET_ERROR == bind(m_ListenSocket, reinterpret_cast<SOCKADDR *>(&serveraddr), sizeof(serveraddr)))
            return false;

        // listen()
        if (SOCKET_ERROR == listen(m_ListenSocket, SOMAXCONN))
            return false;


        return true;
    }

    template<class T>
    inline bool CSocketManager<T>::CreateAcceptSocket(SOCKET_TYPE SocketType)
    {
        GUID _guidacceptex = WSAID_ACCEPTEX;
        DWORD _dwbytes = 0;
        SOCKET_INFO * _psocketinfo = nullptr;

        if (SOCKET_ERROR == WSAIoctl(m_ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &_guidacceptex, sizeof(_guidacceptex), &m_lpfnAcceptEx, sizeof(m_lpfnAcceptEx), &_dwbytes, nullptr, nullptr))
            return false;

        for (int _isocketidx = 0; _isocketidx < m_iSocketCnt; ++_isocketidx)
        {
            m_vecSocketInfo.emplace_back(std::move(std::make_unique<SOCKET_INFO>(SocketType)));
            _psocketinfo = m_vecSocketInfo[_isocketidx].get();

            SOCKET _socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

            if (INVALID_SOCKET != _socket)
                _psocketinfo->Socket = _socket;
            else
                return false;

            if (!AcceptEx(*_psocketinfo))
                return false;
        }

        return true;
    }

}
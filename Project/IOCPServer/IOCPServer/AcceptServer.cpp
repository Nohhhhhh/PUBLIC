#include "stdafx.h"
#include "AcceptServer.h"
#include "SessionManager.h"
#include "User.h"
#include "LanServer.h"

CLIENT::CAcceptServer::CAcceptServer()
    :m_ThreadType(NOH::THREAD_TYPE::THREAD_ACCEPT)
{
}

CLIENT::CAcceptServer::~CAcceptServer()
{
    m_mAcceptExSockAddrsInfo.clear();
}

void CLIENT::CAcceptServer::SetAcceptExSockAddrInfo(SOCKET ListenSocket, NOH::SOCKET_TYPE SocketType)
{
    m_mAcceptExSockAddrsInfo.insert(std::make_pair(SocketType, std::move(std::make_unique<NOH::ACCEPTEX_SOCKADDRS_INFO>(ListenSocket, nullptr))));
}

bool CLIENT::CAcceptServer::InitSessionInfo(NOH::SOCKET_INFO & SocketInfo, NOH::CSession & Session, const int iSessionIdx)
{
    NOH::SESSION_CODE _sessioncode = 0;

    _sessioncode = INPUT_ARRAYIDX(_sessioncode, iSessionIdx);
    _sessioncode = INPUT_SESSIONID(_sessioncode, InterlockedIncrement64(reinterpret_cast<volatile LONG64 *>(NOH::CSessionManager<CUser>::GetInstance()->GetSessionID())));

    SocketInfo.SetSession(Session);
    Session.SetSessionInfo(SocketInfo, _sessioncode);

    switch (SocketInfo.GetSocketType())
    {
    case NOH::SOCKET_TYPE::DEFAULT:
    {
        return false;
    }
    case NOH::SOCKET_TYPE::USER:
    {
        InterlockedIncrement(&g_lCurrentUserCount);
        Session.OnClientJoin(NOH::PQCS_TYPE::JOIN);
    }
    }

    return true;
}

bool CLIENT::CAcceptServer::CreateAcceptExSockAddr(void)
{
    GUID _guidacceptexsockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
    DWORD _dwbytes = 0;

    for (const auto& _info : m_mAcceptExSockAddrsInfo)
    {
        if (SOCKET_ERROR == WSAIoctl(_info.second->ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &_guidacceptexsockaddrs, sizeof(_guidacceptexsockaddrs), &_info.second->lpfnGetAcceptExSockaddrs, sizeof(LPFN_GETACCEPTEXSOCKADDRS), &_dwbytes, nullptr, nullptr))
            return false;
    }

    return true;
}

bool CLIENT::CAcceptServer::AcceptExSockAddr(NOH::SOCKET_INFO & SocketInfo, NOH::CSession & Session)
{

    SOCKADDR_IN *_lplocalsockaddr = nullptr, *_lpremotesockaddr = nullptr;
    int _ilocalsockaddrlen = 0, _remotesockaddrlen = 0;
    const SOCKET _socket = SocketInfo.GetSocket();

    m_mAcceptExSockAddrsInfo.find(SocketInfo.GetSocketType())->second->lpfnGetAcceptExSockaddrs(
        SocketInfo.GetOutBuff(),
        static_cast<int>(NOH::SESSION::BUFF_SOCKET_64) - ((sizeof (SOCKADDR_IN) + 16) * 2),
        sizeof (SOCKADDR_IN) + 16,
        sizeof (SOCKADDR_IN) + 16,
        reinterpret_cast<SOCKADDR **>(&_lplocalsockaddr),
        &_ilocalsockaddrlen,
        reinterpret_cast<SOCKADDR **>(&_lpremotesockaddr),
        &_remotesockaddrlen
    );

    // IP, PORT 얻기
    Session.SetAddress(_lpremotesockaddr);

    // 리슨 소켓 속성 상속
    if (setsockopt(_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<const char *>(&m_mAcceptExSockAddrsInfo[SocketInfo.GetSocketType()].get()->ListenSocket), sizeof(SOCKET)))
        return false;

    return true;
}

NOH::CSession * CLIENT::CAcceptServer::GetSession(const NOH::SOCKET_TYPE SocketType, int * ipSessionIdx)
{
    switch (SocketType)
    {
    case NOH::SOCKET_TYPE::DEFAULT:
    {
        return nullptr;
    }
    case NOH::SOCKET_TYPE::USER:
    {
        NOH::CSessionManager<CUser> *_psessionmanager = NOH::CSessionManager<CUser>::GetInstance();

        // User Idx 뽑기
        if (!_psessionmanager->Get(ipSessionIdx))
            return nullptr;

        return _psessionmanager->GetSession(*ipSessionIdx);
    }
    }

    return nullptr;
}

bool CLIENT::CAcceptServer::CheckSessionBan(const NOH::CSession & Session)
{
    return NOH::CSessionManager<CUser>::GetInstance()->CheckSessionBan(Session);
}

bool CLIENT::CAcceptServer::AssociateAcceptSocket(NOH::SOCKET_INFO & SocketInfo)
{
    switch (SocketInfo.GetSocketType())
    {
    case NOH::SOCKET_TYPE::DEFAULT:
    {
        return false;
    }
    case NOH::SOCKET_TYPE::USER:
    {

        if (nullptr == CreateIoCompletionPort(reinterpret_cast<HANDLE>(SocketInfo.GetSocket()), NOH::CIOCPHandler<CLanServer>::GetInstance()->GetIOCPHandle(), reinterpret_cast<ULONG_PTR>(&SocketInfo), static_cast<DWORD>(0)))
            return false;
        else
            break;
    }
    }

    return true;
}

void CLIENT::CAcceptServer::Error(const WCHAR & szError)
{
    SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"%s", szError);
}

void CLIENT::CAcceptServer::Error(const WCHAR & szError, NOH::SOCKET_INFO & SocketInfo)
{
    NOH::CSocketManager<CUser>::GetInstance()->AcceptEx(SocketInfo);

    if ('\0' != (&szError)[0])
        SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"%s", szError);
}

void CLIENT::CAcceptServer::Error(const WCHAR & szError, NOH::SOCKET_INFO & SocketInfo, const int iSessionIdx)
{
    // TODO - 에러 메시지 출력 또는 로그
    if (-1 != iSessionIdx)
    {
        NOH::CSessionManager<CUser>::GetInstance()->GetSession(iSessionIdx)->ClearCompletionKey();
        NOH::CSessionManager<CUser>::GetInstance()->Put(iSessionIdx);
    }

    NOH::CSocketManager<CUser>::GetInstance()->DisconnectEx(SocketInfo);
    NOH::CSocketManager<CUser>::GetInstance()->AcceptEx(SocketInfo);

    SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"%s", szError);
}

#pragma once

#include "AcceptWorker.h"
#include <unordered_map>

namespace NOH
{
    class CSession;
}

namespace CLIENT
{
    class CAcceptServer : public NOH::CAcceptWorker
    {
    public:
        CAcceptServer();
        ~CAcceptServer();

        void SetAcceptExSockAddrInfo(SOCKET ListenSocket, NOH::SOCKET_TYPE SocketType);

    private:
        virtual bool            InitSessionInfo(NOH::SOCKET_INFO & SocketInfo, NOH::CSession & Session, const int iSessionIdx) override;
        virtual bool            CreateAcceptExSockAddr(void) override;
        virtual bool            AcceptExSockAddr(NOH::SOCKET_INFO & SocketInfo, NOH::CSession & Session) override;
        virtual NOH::CSession * GetSession(const NOH::SOCKET_TYPE SocketType, int *ipSessionIdx) override;
        virtual bool            CheckSessionBan(const NOH::CSession & Session) override;
        virtual bool            AssociateAcceptSocket(NOH::SOCKET_INFO & SocketInfo) override;
   		virtual void            Error(const WCHAR & szError) override;
        virtual void            Error(const WCHAR & szError, NOH::SOCKET_INFO & SocketInfo) override;
        virtual void            Error(const WCHAR & szError, NOH::SOCKET_INFO & pSocketInfo, const int iSessionIdx) override;

        virtual const DWORD              GetThreadIdx(void) override { return *static_cast<DWORD *>(TlsGetValue(m_dwProfileTlsIndex)); }
        virtual const NOH::THREAD_TYPE & GetThreadType(void) override { return m_ThreadType; }

    private:
        std::unordered_map<NOH::SOCKET_TYPE, std::unique_ptr<NOH::ACCEPTEX_SOCKADDRS_INFO>> m_mAcceptExSockAddrsInfo;   // 서버(유저, 기타 서버) ListenSocket, ListenSocket에 해당하는 Accept
        NOH::THREAD_TYPE                                                                    m_ThreadType;
    };
}
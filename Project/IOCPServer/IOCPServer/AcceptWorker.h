#pragma once

#include "__NOH.h"
#include "Thread.h"
#include "SocketManager.h"

namespace NOH
{
    typedef struct st_ACCEPTEX_SOCKADDRS_INFO
    {
        SOCKET                      ListenSocket;
        LPFN_GETACCEPTEXSOCKADDRS   lpfnGetAcceptExSockaddrs;

        st_ACCEPTEX_SOCKADDRS_INFO(SOCKET _ListenSocket, LPFN_GETACCEPTEXSOCKADDRS _lpfnGetAcceptExSockaddrs)
            : ListenSocket(_ListenSocket), lpfnGetAcceptExSockaddrs(_lpfnGetAcceptExSockaddrs) {}

    } ACCEPTEX_SOCKADDRS_INFO;

    class CSession;
    class CAcceptWorker : public CThread
    {
        template<class T> friend class CIOCPHandler;

    public:
        CAcceptWorker();
        virtual ~CAcceptWorker() = 0;

        virtual void Run(void) override;
        virtual void Close(void) override;
        virtual void SetIOCPHandle(HANDLE hIOCP) override { m_hIOCP = hIOCP; }

    private:
        virtual bool       InitSessionInfo(SOCKET_INFO & SocketInfo, CSession & Session, const int iSessionIdx) = 0;
        virtual bool       CreateAcceptExSockAddr(void) = 0;
        virtual bool       AcceptExSockAddr(SOCKET_INFO & SocketInfo, CSession & Session) = 0;
        virtual CSession * GetSession(const SOCKET_TYPE SocketType, int *ipSessionIdx) = 0;
        virtual bool       CheckSessionBan(const CSession & Session) = 0;
        virtual bool       AssociateAcceptSocket(SOCKET_INFO & SocketInfo) = 0;
        virtual void       Error(const WCHAR & szError) = 0;
        virtual void       Error(const WCHAR & szError, NOH::SOCKET_INFO & SocketInfo) = 0;
        virtual void       Error(const WCHAR & szError, NOH::SOCKET_INFO & SocketInfo, const int iSessionIdx) = 0;

        virtual const DWORD         GetThreadIdx(void) = 0;
        virtual const THREAD_TYPE & GetThreadType(void) = 0;

    protected:
        HANDLE              m_hIOCP;
        DWORD               m_dwThreadIdx;
        //LPFN_GETACCEPTEXSOCKADDRS   m_lpfnGetAcceptExSockaddrs;
        //std::vector<std::unique_ptr<LPFN_GETACCEPTEXSOCKADDRS>>    m_vlpfnGetAcceptExSockaddrs;
    };
}
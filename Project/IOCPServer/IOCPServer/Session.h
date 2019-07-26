#pragma once

#include <WinSock2.h>
#include <string>

#include "__NOH.h"
#include "Object.h"
#include "SessionManager.h"


namespace NOH
{
    typedef struct st_IO_INFO
    {
        LONGLONG			llIOCnt;        // 0이면 종료
        LONGLONG			llReleaseFlag;  // 1이면 release 진입. 세션 접속 때 0으로 초기화

        st_IO_INFO() : llIOCnt(0), llReleaseFlag(1) {}
        void Initialize(void) { llIOCnt = 0; llReleaseFlag = 1; }
    } IO_INFO;

    class CStreamQueue;
    class CPacket;
    template<class T> class CQueue_LF;
    class CSession : public CObject
    {
        friend class CLanWorker;
    private:
        typedef struct st_TCP_KEEPALIVE
        {
            ULONG  ulOnOff;
            ULONG  keepalivetime;
            ULONG  keepaliveinterval;

            st_TCP_KEEPALIVE() : ulOnOff(0), keepalivetime(0), keepaliveinterval(0) {}
        } KEEPALIVE;

    public:
        CSession();
        virtual ~CSession() = 0;

        void Initialize(void);
        void Close(void);

        void SetAddress(const SOCKADDR_IN * lpRemoteSockAddr);
        void SetSessionInfo(SOCKET_INFO & CompletionKey, const SESSION_CODE SessionCode);
        void ClearCompletionKey(void);

        bool OnRecv(CPacket & RecvPacket);
        void OnSend(DWORD dwBytesTransfered);
        void OnRelease(void);
        void OnClientJoin(PQCS_TYPE PQCSType);

        SOCKET_INFO *               GetCompletionKey(void)                          { return m_pCompletionKey; }
        SOCKET                      GetSocket(void)                                 { return m_Socket; }
        WCHAR *                     GetSessionIP(void)                              { return m_spIP.get(); }
        const WORD                  GetSessionPort(void)                            { return m_wPort; }
        OVERLAPPED *                GetRecvOverlapped(void)                         { return &m_RecvOverlapped; }
        OVERLAPPED *                GetSendOverlapped(void)                         { return &m_SendOverlapped; }
        CStreamQueue *              GetRecvQueue(void)                              { return m_spRecvQ.get(); }
        CQueue_LF<CPacket *> *      GetSendQueue(void)                              { return m_spSendQ.get(); }
        IO_INFO *                   GetIOInfo(void)                                 { return m_spIOInfo.get(); }
        LONG *                      GetSendIOCnt(void)                              { return &m_lSendIOCnt; }
        LONG *                      GetSendPQCSFlag(void)                           { return &m_lSendPQCSFlag; }

    protected:
        virtual bool SendPacket(CPacket & SendPacket, bool *bpSendPacket, const PQCS_TYPE PQCSType) = 0;
        virtual void Release(void) = 0;
        virtual bool PacketParsing(CPacket & RecvPacket, const PROTOCOL Protocol) = 0;
        virtual bool ClientJoin(const PQCS_TYPE PQCSType) = 0;

    protected:
        SESSION_CODE						    m_SessionCode;	        // 상위 2byte 배열 idx, 하위 6byte는 sessionid

        SOCKET_INFO							    *m_pCompletionKey;
        SOCKET                                  m_Socket;

        std::unique_ptr<WCHAR []>		        m_spIP;
        WORD								    m_wPort;

        OVERLAPPED							    m_RecvOverlapped;
        OVERLAPPED							    m_SendOverlapped;

        std::unique_ptr<CStreamQueue>		    m_spRecvQ;
        std::unique_ptr<CQueue_LF<CPacket *>>	m_spSendQ;

        unique_ptr_aligned<IO_INFO>             m_spIOInfo;
        LONG				                    m_lSendIOCnt;           // 1: sned 불가능 0: send 가능
        LONG				                    m_lSendPQCSFlag;        // 1이면 PQCS 호출

        LONG								    m_lPacketCnt;

        DWORD                                   m_dwCurTlsIndex;
    };
}
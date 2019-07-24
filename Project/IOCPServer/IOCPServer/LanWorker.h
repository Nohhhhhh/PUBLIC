#pragma once

#include "__NOH.h"
#include "Thread.h"

namespace NOH
{
#define dfLAN_WSABUF_MAX_SIZE		200

    class CLanWorker : public CThread
    {
        template<class T> friend class CIOCPHandler;

    protected:
        typedef struct st_LAN_HEADER
        {
            WORD	wPacketSize;
            st_LAN_HEADER() : wPacketSize(0) {}
        } LAN_HEADER;

    public:
        CLanWorker();
        virtual ~CLanWorker() = 0;

        virtual void Run() override;
        virtual void Close() override;
        virtual void SetIOCPHandle(HANDLE hIOCP) override { m_hIOCP = hIOCP; }

    private:
        virtual bool CompleteRecv(CSession & Session, DWORD & dwBytesTransfered) = 0;
        virtual bool CompleteSend(CSession & Session, DWORD & dwBytesTransfered) = 0;
                     
        virtual bool RecvPost(CSession & Session, const bool & bAccept = false) = 0;
        virtual bool SendPost(CSession & Session) = 0;
                     
        virtual void Error(WCHAR *szError) = 0;
                     
        virtual bool PQCSTypeParsing(SOCKET_INFO & SocketInfo, const PQCS_TYPE PQCSType) = 0;
                     
        virtual void DisconnectSocket(CSession & Session, const DISCONNECT_TYPE Type) = 0;
        virtual void ReUseSocket(SOCKET_INFO & SocketInfo) = 0;

        virtual const DWORD         GetThreadIdx(void) = 0;
        virtual const THREAD_TYPE & GetThreadType(void) = 0;

    private:
        HANDLE			m_hIOCP;
        DWORD           m_dwThreadIdx;
    };
}
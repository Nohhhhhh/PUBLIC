#pragma once

#include "LanWorker.h"

namespace CLIENT
{
    class CLanServer : public NOH::CLanWorker
    {
    public:
        CLanServer()
            : m_ThreadType(NOH::THREAD_TYPE::THREAD_LAN) {};
        ~CLanServer() {};

    private:
        virtual bool CompleteRecv(NOH::CSession & Session, DWORD & dwBytesTransfered) override;
        virtual bool CompleteSend(NOH::CSession & Session, DWORD & dwBytesTransfered) override;

        virtual bool RecvPost(NOH::CSession & Session, const bool & bAccept = false) override;
        virtual bool SendPost(NOH::CSession & Session) override;

        virtual void Error(WCHAR *szError) override;

        virtual bool PQCSTypeParsing(NOH::SOCKET_INFO & SocketInfo, const NOH::PQCS_TYPE PQCSType) override;

        virtual void DisconnectSocket( NOH::CSession & Session, const NOH::DISCONNECT_TYPE Type) override;
        virtual void ReUseSocket(NOH::SOCKET_INFO & SocketInfo) override;

        virtual const DWORD                 GetThreadIdx(void) override { return *static_cast<DWORD *>(TlsGetValue(m_dwProfileTlsIndex)); }
        virtual const NOH::THREAD_TYPE &    GetThreadType(void) override { return m_ThreadType; }

    protected:
        NOH::THREAD_TYPE m_ThreadType;
    };
}
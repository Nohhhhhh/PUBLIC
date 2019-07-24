#pragma once

#include <WinSock2.h>

#include "__NOH.h"

namespace NOH
{
    class CThread
    {
    public:
        CThread();
        virtual ~CThread() = 0;
        virtual void Run(void) = 0;
        virtual void Close(void) = 0;
        virtual void SetIOCPHandle(HANDLE hIOCP) = 0;

        void Begin(void);
        void End(void);
        void SetStartFlag(void);

    private:
        static DWORD WINAPI HandleRunner(LPVOID param);

    protected:
        bool    m_bStart;
        DWORD   m_dwProfileTlsIndex;

    private:
        HANDLE  m_hThread;
        UINT    m_uiThreadID;
    };
}
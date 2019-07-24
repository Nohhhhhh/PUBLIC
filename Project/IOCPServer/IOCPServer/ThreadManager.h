#pragma once

#include <WinSock2.h>
#include <list>

#include "Singleton.h"

namespace NOH
{
    using PTHREAD_START = unsigned (__stdcall *) (void *);

    class CThreadManager : public CSingleton<CThreadManager>
    {
    public:
        CThreadManager();
        ~CThreadManager();

    public:
        void    KillThread( HANDLE hThread );
        HANDLE  Spawn( LPTHREAD_START_ROUTINE startAddress, LPVOID param, UINT& uiThreadID );
        void    Close( void );

    private:
        std::list<HANDLE>   m_listThreadHandle;
    };

}
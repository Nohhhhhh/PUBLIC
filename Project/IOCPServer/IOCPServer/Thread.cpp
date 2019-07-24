#include "Thread.h"
#include "ThreadManager.h"
#include <process.h>

NOH::CThread::CThread()
    :m_hThread(INVALID_HANDLE_VALUE), m_uiThreadID(0), m_bStart(false), m_dwProfileTlsIndex(0)
{
}

NOH::CThread::~CThread()
{
}

void NOH::CThread::Begin(void)
{
    if ( m_bStart )
        return;

	m_hThread = CThreadManager::GetInstance()->Spawn( CThread::HandleRunner, this, m_uiThreadID );
}

void NOH::CThread::End(void)
{
    if ( !m_bStart )
        return;

    m_bStart = false;
    CThreadManager::GetInstance()->KillThread( m_hThread );  
    m_hThread = INVALID_HANDLE_VALUE;
}

void NOH::CThread::SetStartFlag(void)
{
    m_bStart = true;
}

DWORD NOH::CThread::HandleRunner(LPVOID param)
{
    CThread * pThread = reinterpret_cast<CThread *>( param );
    pThread->Run();

    return 0;
}

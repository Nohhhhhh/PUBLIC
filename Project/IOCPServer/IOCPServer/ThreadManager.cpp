#include "ThreadManager.h"
#include <algorithm>
#include <process.h>

NOH::CThreadManager::CThreadManager()
{
}

NOH::CThreadManager::~CThreadManager()
{
}

void NOH::CThreadManager::KillThread(HANDLE hThread)
{
    // 기존 코드
    /*std::list<HANDLE>::iterator pos, posPrev;

	pos = m_listThreadHandle.begin();
	while( m_listThreadHandle.end() != pos )
	{
		if( *pos != hThread )
		{
			++pos;
			continue;
		}
		else
		{
			CloseHandle( hThread );
			m_listThreadHandle.erase( pos );
			return;
		}
	}*/

    // 변경 코드
    auto& Handle = std::find_if( m_listThreadHandle.begin(), m_listThreadHandle.end(), [ &hThread ]( auto& h ){ return h == hThread; } );

     if ( Handle != m_listThreadHandle.end() )
     {
         // IOCP 쓰레드니까, 5초 대기
         WaitForSingleObject( hThread, 5000 );
         CloseHandle( hThread );
         m_listThreadHandle.erase( Handle );
     }
}


HANDLE NOH::CThreadManager::Spawn(LPTHREAD_START_ROUTINE startAddress, LPVOID param, UINT & uiThreadID)
{
	HANDLE hThread = (HANDLE)_beginthreadex( nullptr,
										0,
										reinterpret_cast<PTHREAD_START>( startAddress ), 
										param, 
										0, 
										&uiThreadID );

	m_listThreadHandle.emplace_back( hThread );
	return hThread;
}

void NOH::CThreadManager::Close(void)
{
    // 기존 코드
    /*std::list<HANDLE>::iterator pos = m_listThreadHandle.begin();

    while( m_listThreadHandle.end() != pos )
	{
		WaitForSingleObject( *pos, 5000 );
		CloseHandle( *pos );
		++pos;
	}

	m_listThreadHandle.clear();*/
    
    if ( !m_listThreadHandle.empty() )
    {
        // 변경 코드
        std::for_each( m_listThreadHandle.begin(), m_listThreadHandle.end(), []( auto& h ){
		    CloseHandle( h );
            WaitForSingleObject( h, 5000 );
        } );
    }

    m_listThreadHandle.clear();
}



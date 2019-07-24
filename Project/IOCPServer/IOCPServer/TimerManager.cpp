#include "LockGuard.h"
#include "TimerManager.h"
#include "Profiler.h"
#include "Global.h"

#include <future>
#include <algorithm>

void NOH::CTimerManager::CTimer::Initialize(void)
{
    m_TimerList.clear();
    InitializeSRWLock( &m_SRWLock );

    LARGE_INTEGER lFrequency;
    QueryPerformanceFrequency( &lFrequency );
	m_dMilliFrequency = static_cast<double>( lFrequency.QuadPart / 1000.0f );
    m_dSecondFrequency = static_cast<double>( lFrequency.QuadPart );
    Begin();
    SetStartFlag();
}

void NOH::CTimerManager::CTimer::Run(void)
{
    m_dwProfileTlsIndex = TlsAlloc();
    DWORD _dwthreadidx = NOH::CProfiler::GetInstance()->GetThreadIdx(GetThreadType()) - 1;
    TlsSetValue(m_dwProfileTlsIndex, &_dwthreadidx);

    bool _bcallfunc = false;

    LARGE_INTEGER _lcurrenttime;
    LARGE_INTEGER _lcurrenttime1;

    // 대기
    while( !m_bStart )
        Sleep(100);

    while ( m_bStart )
    {
        GetLocalTime( &m_LocalTime );

        QueryPerformanceCounter( &_lcurrenttime );

        auto pos = m_TimerList.begin();

        while ( pos != m_TimerList.end() )
        {
            LONGLONG _lldiff = (_lcurrenttime.QuadPart - pos->lInitTime.QuadPart) / m_dSecondFrequency;
            _bcallfunc = true;

            if ( TIMER_TYPE::FLAT_TIMER == pos->TimerType )
            {
                if ( _lldiff >= static_cast<LONGLONG>( pos->TimeValue ) && 0 == m_LocalTime.wSecond )
                    pos->lInitTime = _lcurrenttime;
                else
                    _bcallfunc = false;
            }


            if ( _bcallfunc )
            {
                std::async( std::launch::async, pos->Func );     // 호출과 동시에 리턴 콜백함수는 계속 진행
                if ( TIMER_TYPE::INSTANT_TIMER == pos->TimerType )
                    m_TimerList.erase(pos++);
            }
            else
            {
                ++pos;
            }

        }

        Sleep(100);
    }
}

void NOH::CTimerManager::CTimer::Close(void)
{
    End();
}

void NOH::CTimerManager::CTimer::RegisterTimer(const TIMERFUNC & Func, const TIME_VALUE TimeValue, const TIMER_TYPE TimerType)
{
    CLockGuard lockguard( LOCK_TYPE::SRWLOCK_EXCLUSIVE, &m_SRWLock );

    LARGE_INTEGER lInitTime;
    QueryPerformanceCounter( &lInitTime );

    m_TimerList.emplace_back( std::move( TIMER_INFO( Func, TimeValue, TimerType, lInitTime ) ) );
}


void NOH::CTimerManager::CTimer::RemoveTimer(const TIMERFUNC & func)
{
    CLockGuard lockguard( LOCK_TYPE::SRWLOCK_EXCLUSIVE, &m_SRWLock );

    m_TimerList.remove_if( [&func]( TIMER_INFO &t )
    {
        return t.Func == func;
    } );
}

void NOH::CTimerManager::CTimer::RemoveAllTimer(void)
{
    CLockGuard lockguard( LOCK_TYPE::SRWLOCK_EXCLUSIVE, &m_SRWLock );

    m_TimerList.clear();
}

void NOH::CTimerManager::Initialize(void)
{
    m_spTimer.get()->Initialize();
}

void NOH::CTimerManager::Close(void)
{
    auto _timerptr = m_spTimer.release();
}

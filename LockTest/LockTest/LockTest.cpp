// LockTest.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>
#include <process.h>
#include <mutex>
#include <chrono>
#include <iostream>
#include <atomic>

void atomiclockedfunc(LPVOID lpThreadParameter);
void interlockedfunc(LPVOID lpThreadParameter);
void srwlockedfunc(LPVOID lpThreadParameter);
void cslockedfunc(LPVOID lpThreadParameter);
void mxlockedfunc(LPVOID lpThreadParameter);

unsigned int uiCount = 0;
std::atomic<unsigned int> aCount(0);

CRITICAL_SECTION time11;
CRITICAL_SECTION CS;
SRWLOCK SRW;
SRWLOCK SRW2;
std::mutex mtx;
std::chrono::system_clock::time_point start;
std::chrono::microseconds ms;

LARGE_INTEGER		                        m_lFrequency;
double				                        m_dMicroFrequency;
LARGE_INTEGER		                        lStartTime;
unsigned __int64	ui64TotalTime;

int ithreadcnt = 0;
int iCount = 0;
int testcnt = 0;

int locktype = 1;

int main()
{
    timeBeginPeriod(1);
    InitializeCriticalSection(&time11);
    InitializeCriticalSection(&CS);
    InitializeSRWLock(&SRW);
    InitializeSRWLock(&SRW2);

    memset( &m_lFrequency, 0, sizeof(LARGE_INTEGER) );
    QueryPerformanceFrequency( &m_lFrequency );
    // 나노 세컨드: 10억분의 1초. 마이크로 세컨드: 100만분의 1초. 밀리 세컨드. 1천분의 1초
    m_dMicroFrequency = static_cast<double>( m_lFrequency.QuadPart / 10000.0f );

    std::cout << std::endl << "Thread Count - input: ";
    std::cin >> testcnt;

    switch (locktype)
    {
    case 1:
        QueryPerformanceCounter( &lStartTime );
        for (int i = 0; i < testcnt; ++i)
            _beginthread(atomiclockedfunc, 0, (void *)nullptr);

        while(iCount != testcnt)
        {
            Sleep(1000);
        }
        
        printf("atomic: %lf\n", ui64TotalTime / testcnt / m_dMicroFrequency);
        ui64TotalTime = 0;
        ithreadcnt = 0;
        iCount = 0;
        uiCount = 0;
        Sleep(2000);
    case 2:
        QueryPerformanceCounter( &lStartTime );
        for (int i = 0; i < testcnt; ++i)
            _beginthread(interlockedfunc, 0, (void *)nullptr);

        while(iCount != testcnt)
        {
            Sleep(1000);
        }

         printf("interlock: %lf\n", ui64TotalTime / testcnt / m_dMicroFrequency);
         ui64TotalTime = 0;
        ithreadcnt = 0;
        iCount = 0;
        uiCount = 0;
        Sleep(2000);
    case 3:
        QueryPerformanceCounter( &lStartTime );
        for (int i = 0; i < testcnt; ++i)
            _beginthread(srwlockedfunc, 0, (void *)nullptr);

        while(iCount != testcnt)
        {
            Sleep(1000);
        }

         printf("srwlock: %lf\n", ui64TotalTime / testcnt / m_dMicroFrequency);
         ui64TotalTime = 0;
        ithreadcnt = 0;
        iCount = 0;
        uiCount = 0;
        Sleep(2000);
    case 4:
        QueryPerformanceCounter( &lStartTime );
        for (int i = 0; i < testcnt; ++i)
            _beginthread(cslockedfunc, 0, (void *)nullptr);

        while(iCount != testcnt)
        {
            Sleep(1000);
        }

        printf("critical: %lf\n", ui64TotalTime / testcnt / m_dMicroFrequency);
        ui64TotalTime = 0;
        ithreadcnt = 0;
        iCount = 0;
        uiCount = 0;
        Sleep(2000);
    case 5:
        QueryPerformanceCounter( &lStartTime );
        for (int i = 0; i < testcnt; ++i)
            _beginthread(mxlockedfunc, 0, (void *)nullptr);

        while(iCount != testcnt)
        {
            Sleep(1000);
        }

        printf("mutex: %lf\n", ui64TotalTime / testcnt / m_dMicroFrequency);
        ui64TotalTime = 0;
        ithreadcnt = 0;
        iCount = 0;
        uiCount = 0;
        Sleep(2000);
    }

    DeleteCriticalSection(&CS);
    timeEndPeriod(1);
    system("pause");
    return 0;
}

void interlockedfunc(LPVOID lpThreadParameter)
{
    AcquireSRWLockExclusive(&SRW2);
    ++ithreadcnt;
    ReleaseSRWLockExclusive(&SRW2);

    while(testcnt != ithreadcnt)
        Sleep(1);

    while(1)
    {
        if (10000000 <= InterlockedIncrement(&uiCount))
        {
            break;
        };
    }

    AcquireSRWLockExclusive(&SRW2);
    LARGE_INTEGER _lendtime;
    QueryPerformanceCounter( &_lendtime );
	unsigned __int64 _ui64time = _lendtime.QuadPart - lStartTime.QuadPart;
    ui64TotalTime += _ui64time;
    ++iCount;
    ReleaseSRWLockExclusive(&SRW2);
    
    return;
}

void srwlockedfunc(LPVOID lpThreadParameter)
{
    AcquireSRWLockExclusive(&SRW2);
    ++ithreadcnt;
    ReleaseSRWLockExclusive(&SRW2);

    while(testcnt != ithreadcnt)
        Sleep(1);

    while(1)
    {
        AcquireSRWLockExclusive(&SRW);
        if (10000000 <= ++uiCount)
        {
            ReleaseSRWLockExclusive(&SRW);
            break;
        }
        ReleaseSRWLockExclusive(&SRW);
    }

    AcquireSRWLockExclusive(&SRW2);
    LARGE_INTEGER _lendtime;
    QueryPerformanceCounter( &_lendtime );
	unsigned __int64 _ui64time = _lendtime.QuadPart - lStartTime.QuadPart;
    ui64TotalTime += _ui64time;
    ++iCount;
    ReleaseSRWLockExclusive(&SRW2);
    
    return;
}

void cslockedfunc(LPVOID lpThreadParameter)
{
    AcquireSRWLockExclusive(&SRW2);
    ++ithreadcnt;
    ReleaseSRWLockExclusive(&SRW2);

    while(testcnt != ithreadcnt)
        Sleep(1);

    while(1)
    {
        EnterCriticalSection(&CS);
        if (10000000 <= ++uiCount)
        {
            LeaveCriticalSection(&CS);
            break;
        }
        LeaveCriticalSection(&CS);
    }

    AcquireSRWLockExclusive(&SRW2);
    LARGE_INTEGER _lendtime;
    QueryPerformanceCounter( &_lendtime );
	
	unsigned __int64 _ui64time = _lendtime.QuadPart - lStartTime.QuadPart;
    ui64TotalTime += _ui64time;
    ++iCount;
    ReleaseSRWLockExclusive(&SRW2);
    
    return;
}

void mxlockedfunc(LPVOID lpThreadParameter)
{
    AcquireSRWLockExclusive(&SRW2);
    ++ithreadcnt;
    ReleaseSRWLockExclusive(&SRW2);

    while(testcnt != ithreadcnt)
        Sleep(1);

    while(1)
    {
        mtx.lock();
        if (10000000 <= ++uiCount)
        {
            mtx.unlock();
            break;
        }
        mtx.unlock();
    }

    AcquireSRWLockExclusive(&SRW2);
    LARGE_INTEGER _lendtime;
    QueryPerformanceCounter( &_lendtime );
	unsigned __int64 _ui64time = _lendtime.QuadPart - lStartTime.QuadPart;
    ui64TotalTime += _ui64time;
    ++iCount;
    ReleaseSRWLockExclusive(&SRW2);
    
    return;
}

void atomiclockedfunc(LPVOID lpThreadParameter)
{
    AcquireSRWLockExclusive(&SRW2);
    ++ithreadcnt;
    ReleaseSRWLockExclusive(&SRW2);

    while(testcnt != ithreadcnt)
        Sleep(1);

    while(1)
    {
        if (10000000 <= std::atomic_fetch_add(&aCount, 1))
        {
            break;
        };
    }

    AcquireSRWLockExclusive(&SRW2);
    LARGE_INTEGER _lendtime;
    QueryPerformanceCounter( &_lendtime );
	unsigned __int64 _ui64time = _lendtime.QuadPart - lStartTime.QuadPart;
    ui64TotalTime += _ui64time;
    ++iCount;
    ReleaseSRWLockExclusive(&SRW2);
    
    return;
}

/*
사용 방법:
이 클래스를 사용하려는 클래스의 멤버로 CRITICAL_SECTION / SRWLOCK 을 가져야 한다.
CS인 경우, CS를 멤버로 가지고 있는 클래스에서 InitializeCriticalSection() 과 DeleteCriticalSection() 호출을 담당한다.
SRWLOCK인 경우, SRW을 멤버로 가지고 있는 클래스에서 InitializeSRWLock() 호출을 담당한다.
*/

#pragma once

#include "__NOH.h"
#include <mutex>

namespace NOH
{
    class CLockGuard
    {
    public:
        CLockGuard( LOCK_TYPE LockType, CRITICAL_SECTION *pCS );
        CLockGuard( LOCK_TYPE LockType, SRWLOCK *pSRWL );
        CLockGuard( LOCK_TYPE LockType, std::mutex *pMX );
        ~CLockGuard( void );

    private:
        LOCK_TYPE           m_LockType;
        CRITICAL_SECTION    *m_pCS;
        SRWLOCK             *m_pSRW;
        std::mutex          *m_pMX;
    };
}
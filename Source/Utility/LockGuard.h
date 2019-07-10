/*
��� ���:
�� Ŭ������ ����Ϸ��� Ŭ������ ����� CRITICAL_SECTION / SRWLOCK �� ������ �Ѵ�.
CS�� ���, CS�� ����� ������ �ִ� Ŭ�������� InitializeCriticalSection() �� DeleteCriticalSection() ȣ���� ����Ѵ�.
SRWLOCK�� ���, SRW�� ����� ������ �ִ� Ŭ�������� InitializeSRWLock() ȣ���� ����Ѵ�.
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
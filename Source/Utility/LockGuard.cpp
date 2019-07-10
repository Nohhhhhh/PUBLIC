#include "LockGuard.h"

NOH::CLockGuard::CLockGuard(LOCK_TYPE LockType, CRITICAL_SECTION *pCS)
    : m_LockType(LockType), m_pCS(pCS), m_pSRW(nullptr), m_pMX(nullptr)
{
    EnterCriticalSection( pCS );
}

NOH::CLockGuard::CLockGuard(LOCK_TYPE LockType, SRWLOCK *pSRWL)
    : m_LockType(LockType), m_pSRW(pSRWL), m_pCS(nullptr), m_pMX(nullptr)
{
    switch ( LockType )
    {
    case LOCK_TYPE::SRWLOCK_SHARED:
        AcquireSRWLockShared( pSRWL );
        break;
    case LOCK_TYPE::SRWLOCK_EXCLUSIVE:
        AcquireSRWLockExclusive( pSRWL );
        break;
    }
}

NOH::CLockGuard::CLockGuard(LOCK_TYPE LockType, std::mutex * pMX)
    : m_LockType(LockType), m_pMX(pMX), m_pSRW(nullptr), m_pCS(nullptr)
{
    m_pMX->lock();
}

NOH::CLockGuard::~CLockGuard(void)
{
    switch ( m_LockType )
    {
    case LOCK_TYPE::CRITICALSECTION:
    {
        LeaveCriticalSection( m_pCS );
        break;
    }
    case LOCK_TYPE::SRWLOCK_SHARED:
    {
        ReleaseSRWLockShared( m_pSRW );
        break;
    }
    case LOCK_TYPE::SRWLOCK_EXCLUSIVE:
    {
        ReleaseSRWLockExclusive( m_pSRW );
        break;
    }
    case LOCK_TYPE::MUTEX:
    {
        m_pMX->unlock();
        break;
    }
    }
}

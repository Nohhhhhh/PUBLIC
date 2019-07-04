#pragma once

#include "stdafx.h"

struct SpinWait
{
	struct _SYSINFO : public SYSTEM_INFO
	{
		_SYSINFO() { GetSystemInfo(this); }
	};

	static _SYSINFO s_si;
	// cpu의 개수를 획득하기 위해 SYSTEM_INFO 구조체를 상속하고 정적 멤버로 선언한다. 정적 멤버이므로 SpinWait 구조체 선언과 함께 자동으로 생성자가 호출됨에 따라 시스템 정보의 획득이 가능하다.

	const int YIELD_THRESHLOD = 25;	// 스핀 수행 한계치
	const int MAX_SPIN_INTERVAL = 32;	// 최대 스핀 인터벌
	const int SLEEP_0_TIMES = 2;		// Sleep(0) 호출을 위한 빈도 기준치
	const int SLEEP_1_TIMES = 10;	// Sleep(1) 호출을 위한 빈도 기준치
	// 스핀 대기를 위한 상수의 정의다. 이 값들을 적당히 변경함에 따라 더 최적화된 스핀락을 구현할 수 있다.

	int m_count;	// 스핀 횟수를 저장하는 멤버 변수다.

	SpinWait()
	{
		m_count = 0;
	}

#define NextSpinWillYield (s_si.dwNumberOfProcessors == 1 || m_count >= YIELD_THRESHLOD)	// YieldProcessor의 호출 여부를 판별하는 매크로

	void SpinOnce()
	{
		if (NextSpinWillYield)
		{
			// 단일 CPU거나 스핀 횟수(m_count)가 YIELD_THRESHHOLD 이상일 경우네는 스핀을 수행하지 않고 Sleep이나 SwitchToThread를 호출한다.

			int yieldSoFar = (m_count >= YIELD_THRESHLOD ? m_count - YIELD_THRESHLOD : m_count);

			if ((yieldSoFar % SLEEP_1_TIMES) == SLEEP_0_TIMES - 1)
			{
				Sleep(0);
			}
			else if ((yieldSoFar % SLEEP_1_TIMES) == SLEEP_1_TIMES - 1)
			{
				Sleep(1);
			}
			else
				SwitchToThread();
			// 조건에 따라 WitchToThrad 또는 Sleep(0), Sleep(1)을 호출한다. 빈도 조정을 위해 매크로 정의를 ㅂ녀경할 수 있다.

		}
		else
		{
			// 다중 CPU일 경우에는 우선 스핀을 수행한다.
			int nLoopCnt = (int)(m_count * ((float)MAX_SPIN_INTERVAL / YIELD_THRESHLOD)) + 1;
			while (nLoopCnt-- > 0)
				YieldProcessor();
			// m_count 값을 기준으로 적절한 스핀 횟수를 구해 루프를 돌면서 YieldProcessor를 호출한다.
		}

		m_count = (m_count == INT_MAX ? YIELD_THRESHLOD : m_count + 1);
		// 누적 스핀 횟수를 보관하기 위해 m_count의 필드값을 증가시킨다.
	}

	void Reset() { m_count = 0; }
	int Count() { return m_count; }
};

SpinWait::_SYSINFO SpinWait::s_si;

class SpinLock
{
	volatile DWORD m_state;    // 본 스핀락을 획득한 스레드의 ID를 보관하는 멤버 필드

	const LONG LOCK_AVAIL = 0;    // 본 스핀락 소유한 스페드가 없음을 의미하는 상수다. m_state 필드 값이 LOCK_AVAIL일 때 본 스핀락은 소유 가능한 상태가 된다.

public:
	SpinLock()
	{
		m_state = LOCK_AVAIL;
	}

public:
	void Acquire()
	{
		DWORD dwCurThrId = GetCurrentThreadId();
		if (dwCurThrId == m_state)
			throw HRESULT_FROM_WIN32(ERROR_INVALID_OWNER);    // 호출 스레드가 스핀락을 소유한 상태면 예외를 던진다.

		if (InterlockedCompareExchange(&m_state, dwCurThrId, LOCK_AVAIL) != LOCK_AVAIL)
		{
			// 스핀락을 소유한 스레드가 이미 존재할 경우에는 스핀락 회득을 위해 스핀 체크를 시도한다.
			SpinWait sw;    // 스핀 대기를 담당하는 구조체를 선언한다.

			do
			{
				do
				{
					sw.SpinOnce();
				} while (m_state != LOCK_AVAIL);        //  m_state가 LOCK_AVAIL이 될 때까지 스핀 체크를 수행한다.
			} while (InterlockedCompareExchange(&m_state, dwCurThrId, LOCK_AVAIL) != LOCK_AVAIL);
			// 호출한 스레드가 스핀락을 실제로 소유할 때까지, 즉 m_state를 호출 스레드의 ID로 설정할 수 있을 때까지 위의 과정을 반복한다.   
		}
	}


	void Release()
	{
		DWORD dwCurThrId = GetCurrentThreadId();

		if (dwCurThrId != m_state)
			throw HRESULT_FROM_WIN32(ERROR_INVALID_OWNER);    // 호출 스레드가 스핀락을 소유한 상태가 아니면 예외를 던진다.

		InterlockedExchange(&m_state, LOCK_AVAIL);    // m_state를 LOCK_AVAIL로 설정함으로써 스핀락 상태를 소유 가능 상태로 만든다.
	}
};
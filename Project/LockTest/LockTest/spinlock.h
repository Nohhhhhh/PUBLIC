#pragma once

#include "stdafx.h"

struct SpinWait
{
	struct _SYSINFO : public SYSTEM_INFO
	{
		_SYSINFO() { GetSystemInfo(this); }
	};

	static _SYSINFO s_si;
	// cpu�� ������ ȹ���ϱ� ���� SYSTEM_INFO ����ü�� ����ϰ� ���� ����� �����Ѵ�. ���� ����̹Ƿ� SpinWait ����ü ����� �Բ� �ڵ����� �����ڰ� ȣ��ʿ� ���� �ý��� ������ ȹ���� �����ϴ�.

	const int YIELD_THRESHLOD = 25;	// ���� ���� �Ѱ�ġ
	const int MAX_SPIN_INTERVAL = 32;	// �ִ� ���� ���͹�
	const int SLEEP_0_TIMES = 2;		// Sleep(0) ȣ���� ���� �� ����ġ
	const int SLEEP_1_TIMES = 10;	// Sleep(1) ȣ���� ���� �� ����ġ
	// ���� ��⸦ ���� ����� ���Ǵ�. �� ������ ������ �����Կ� ���� �� ����ȭ�� ���ɶ��� ������ �� �ִ�.

	int m_count;	// ���� Ƚ���� �����ϴ� ��� ������.

	SpinWait()
	{
		m_count = 0;
	}

#define NextSpinWillYield (s_si.dwNumberOfProcessors == 1 || m_count >= YIELD_THRESHLOD)	// YieldProcessor�� ȣ�� ���θ� �Ǻ��ϴ� ��ũ��

	void SpinOnce()
	{
		if (NextSpinWillYield)
		{
			// ���� CPU�ų� ���� Ƚ��(m_count)�� YIELD_THRESHHOLD �̻��� ���״� ������ �������� �ʰ� Sleep�̳� SwitchToThread�� ȣ���Ѵ�.

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
			// ���ǿ� ���� WitchToThrad �Ǵ� Sleep(0), Sleep(1)�� ȣ���Ѵ�. �� ������ ���� ��ũ�� ���Ǹ� ������� �� �ִ�.

		}
		else
		{
			// ���� CPU�� ��쿡�� �켱 ������ �����Ѵ�.
			int nLoopCnt = (int)(m_count * ((float)MAX_SPIN_INTERVAL / YIELD_THRESHLOD)) + 1;
			while (nLoopCnt-- > 0)
				YieldProcessor();
			// m_count ���� �������� ������ ���� Ƚ���� ���� ������ ���鼭 YieldProcessor�� ȣ���Ѵ�.
		}

		m_count = (m_count == INT_MAX ? YIELD_THRESHLOD : m_count + 1);
		// ���� ���� Ƚ���� �����ϱ� ���� m_count�� �ʵ尪�� ������Ų��.
	}

	void Reset() { m_count = 0; }
	int Count() { return m_count; }
};

SpinWait::_SYSINFO SpinWait::s_si;

class SpinLock
{
	volatile DWORD m_state;    // �� ���ɶ��� ȹ���� �������� ID�� �����ϴ� ��� �ʵ�

	const LONG LOCK_AVAIL = 0;    // �� ���ɶ� ������ ����尡 ������ �ǹ��ϴ� �����. m_state �ʵ� ���� LOCK_AVAIL�� �� �� ���ɶ��� ���� ������ ���°� �ȴ�.

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
			throw HRESULT_FROM_WIN32(ERROR_INVALID_OWNER);    // ȣ�� �����尡 ���ɶ��� ������ ���¸� ���ܸ� ������.

		if (InterlockedCompareExchange(&m_state, dwCurThrId, LOCK_AVAIL) != LOCK_AVAIL)
		{
			// ���ɶ��� ������ �����尡 �̹� ������ ��쿡�� ���ɶ� ȸ���� ���� ���� üũ�� �õ��Ѵ�.
			SpinWait sw;    // ���� ��⸦ ����ϴ� ����ü�� �����Ѵ�.

			do
			{
				do
				{
					sw.SpinOnce();
				} while (m_state != LOCK_AVAIL);        //  m_state�� LOCK_AVAIL�� �� ������ ���� üũ�� �����Ѵ�.
			} while (InterlockedCompareExchange(&m_state, dwCurThrId, LOCK_AVAIL) != LOCK_AVAIL);
			// ȣ���� �����尡 ���ɶ��� ������ ������ ������, �� m_state�� ȣ�� �������� ID�� ������ �� ���� ������ ���� ������ �ݺ��Ѵ�.   
		}
	}


	void Release()
	{
		DWORD dwCurThrId = GetCurrentThreadId();

		if (dwCurThrId != m_state)
			throw HRESULT_FROM_WIN32(ERROR_INVALID_OWNER);    // ȣ�� �����尡 ���ɶ��� ������ ���°� �ƴϸ� ���ܸ� ������.

		InterlockedExchange(&m_state, LOCK_AVAIL);    // m_state�� LOCK_AVAIL�� ���������ν� ���ɶ� ���¸� ���� ���� ���·� �����.
	}
};
#include "stdafx.h"
#include "ResourceMonitor.h"

#include <strsafe.h>

NOH::CResourceMonitor::CResourceMonitor()
{
	// CPU 코어 개수를 구한다.
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_iCoreNum = si.dwNumberOfProcessors;
	m_bShutDown = false;
}

NOH::CResourceMonitor::~CResourceMonitor()
{
	
}

void NOH::CResourceMonitor::Stop()
{
	m_bShutDown = true;

	PdhRemoveCounter(m_pdh_Counter_Idle);
	PdhRemoveCounter(m_pdh_Counter_CPU_Total);
	PdhRemoveCounter(m_pdh_Counter_CPU_Core1);
	PdhRemoveCounter(m_pdh_Counter_CPU_Core2);
	PdhRemoveCounter(m_pdh_Counter_CPU_Core3);
	PdhRemoveCounter(m_pdh_Counter_CPU_Core4);

	PdhRemoveCounter(m_pdh_Counter_Memory_Available);
	PdhRemoveCounter(m_pdh_Counter_Nonpaged_Memory_Available);
	PdhRemoveCounter(m_pdh_Counter_Network_Receive);
	PdhRemoveCounter(m_pdh_Counter_Network_Send);
	PdhCloseQuery(m_pdh_Query);
}


///////////////////////////////////////////////////////////////
// 성능 모니터 PDH 준비.
///////////////////////////////////////////////////////////////
bool NOH::CResourceMonitor::PDH_Initial(void)
{
	PDH_STATUS Status;

	int iCnt = 0;
	bool bErr = false;
	WCHAR *szCur = nullptr;
	WCHAR *szCounters = nullptr;
	WCHAR *szInterfaces = nullptr;
	DWORD dwCounterSize = 0, dwInterfaceSize = 0;
	WCHAR szQuery[1024] = { 0 };
	
	// Creates a new query that is used to manage the collection of performance data. 
	Status = PdhOpenQuery(NULL, NULL, &m_pdh_Query);

	// Adds the specified counter to the query. 
	Status = PdhAddCounter(m_pdh_Query, L"\\Processor(_Total)\\% Idle Time", NULL, &m_pdh_Counter_Idle);
    Status = PdhAddCounter(m_pdh_Query, L"\\Processor(_Total)\\% User Time", NULL, &m_pdh_Counter_User);
    Status = PdhAddCounter(m_pdh_Query, L"\\Processor(_Total)\\% Privileged Time", NULL, &m_pdh_Counter_Kernel);
	Status = PdhAddCounter(m_pdh_Query, L"\\Processor(_Total)\\% Processor Time", NULL, &m_pdh_Counter_CPU_Total);
	Status = PdhAddCounter(m_pdh_Query, L"\\Processor(0)\\% Processor Time", NULL, &m_pdh_Counter_CPU_Core1);
	Status = PdhAddCounter(m_pdh_Query, L"\\Processor(1)\\% Processor Time", NULL, &m_pdh_Counter_CPU_Core2);
	Status = PdhAddCounter(m_pdh_Query, L"\\Processor(2)\\% Processor Time", NULL, &m_pdh_Counter_CPU_Core3);
	Status = PdhAddCounter(m_pdh_Query, L"\\Processor(3)\\% Processor Time", NULL, &m_pdh_Counter_CPU_Core4);

	Status = PdhAddCounter(m_pdh_Query, L"\\Memory\\Available Bytes", NULL, &m_pdh_Counter_Memory_Available);
	Status = PdhAddCounter(m_pdh_Query, L"\\Memory\\Pool Nonpaged Bytes", NULL, &m_pdh_Counter_Nonpaged_Memory_Available);

	Status = PdhAddCounter(m_pdh_Query, L"\\Network Interface(*)\\Bytes Received/sec", NULL, &m_pdh_Counter_Network_Receive);
	Status = PdhAddCounter(m_pdh_Query, L"\\Network Interface(*)\\Bytes Sent/sec", NULL, &m_pdh_Counter_Network_Send);

	// PDH enum Object를 사용하는 방법
	// 모든 이더넷 이름이 나오지만 실제 사용중인 이더넷, 가상이더넷 등등은 확인 불가

	// PdhEnumObjectItems 를 통해서 "NetworkInterface" 항목에서 얻을 수 있는
	// 측정항목(Counters) / 인터페이스 항목(Interfaces) 를 얻음. 그런데 그 개수나 길이를 모르기 때문에
	// 먼저 버퍼의 길이를 알기 위해서 Out Buffer 인자들을 NULL 포인터로 넣어서 사이즈만 확인.
	PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0);

	szCounters = new WCHAR[dwCounterSize];
	szInterfaces = new WCHAR[dwInterfaceSize];

	// 버퍼의 동적할당 후 다시 호출!
	// szCounters와 szInterfaces 버퍼에는 여러 개의 문자열이 들어온다. 2차원 배열도 아니고, 그냥 NULL 포인터로 끝나는 문자열들이
	// dwCounterSize, dwInterfaceSize 길이만큼 줄줄이 들어있다.
	// 이를 묹열 단위로 끊어서 개수를 확인 해야 함. ex) aaa\0bbb\0ccc\0ddd

	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
	{
		delete[] szCounters;
		delete[] szInterfaces;
		return false;
	}

	iCnt = 0;
	szCur = szInterfaces;

	// szInterfaces 에서 문자열 단위로 끊으면서, 이름을 복사 받는다.
	for (; *szCur != L'\0' && iCnt < df_PDH_ETHERNET_MAX; szCur += wcslen(szCur) + 1, ++iCnt)
	{
		m_EthernetStruct[iCnt]._bUse = true;
		
		wcscpy_s(m_EthernetStruct[iCnt]._szName, szCur);

		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Received/sec", szCur);
		PdhAddCounter(m_pdh_Query, szQuery, NULL, &m_EthernetStruct[iCnt]._pdh_Counter_Network_RecvBytes);

		StringCbPrintf(szQuery, sizeof(WCHAR) * 1024, L"\\Network Interface(%s)\\Bytes Sent/sec", szCur);
		PdhAddCounter(m_pdh_Query, szQuery, NULL, &m_EthernetStruct[iCnt]._pdh_Counter_Network_SendBytes);
	}


	PdhCollectQueryData(m_pdh_Query);

	return true;
}


/////////////////////////////////////////////////////////////////////
// System PerformanceUpdate()
//
// PDH 쿼리 갱신.  시스템 정보 얻기 전 호출 해주어야 함.
/////////////////////////////////////////////////////////////////////
bool NOH::CResourceMonitor::SystemPDH_Update(void)
{

	PDH_STATUS Status;
	PDH_FMT_COUNTERVALUE CounterValue;

	PdhCollectQueryData(m_pdh_Query);

	m_pdh_Value_Idle = 0;
    m_pdh_Value_User = 0;
    m_pdh_Value_Kernel = 0;
	m_pdh_Value_CPU_Total = 0;
	m_pdh_Value_CPU_Core1 = 0;
	m_pdh_Value_CPU_Core2 = 0;
	m_pdh_Value_CPU_Core3 = 0;
	m_pdh_Value_CPU_Core4 = 0;

	m_pdh_value_Memory_Available = 0;
	m_pdh_value_Network_RecvBytes = 0;
	m_pdh_value_Network_SendBytes = 0;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_Idle, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_Idle = CounterValue.doubleValue;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_User, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_User = CounterValue.doubleValue;

    Status = PdhGetFormattedCounterValue(m_pdh_Counter_Kernel, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_Kernel = CounterValue.doubleValue;

    Status = PdhGetFormattedCounterValue(m_pdh_Counter_CPU_Total, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_CPU_Total = CounterValue.doubleValue;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_CPU_Core1, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_CPU_Core1 = CounterValue.doubleValue;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_CPU_Core2, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_CPU_Core2 = CounterValue.doubleValue;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_CPU_Core3, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_CPU_Core3 = CounterValue.doubleValue;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_CPU_Core4, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_Value_CPU_Core4 = CounterValue.doubleValue;

    
	Status = PdhGetFormattedCounterValue(m_pdh_Counter_Memory_Available, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_value_Memory_Available = CounterValue.doubleValue;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_Nonpaged_Memory_Available, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_value_Nonpaged_Memory_Available = CounterValue.doubleValue;

	Status = PdhGetFormattedCounterValue(m_pdh_Counter_Network_Receive, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_value_Network_Receive = CounterValue.doubleValue;
	
	Status = PdhGetFormattedCounterValue(m_pdh_Counter_Network_Send, PDH_FMT_DOUBLE, NULL, &CounterValue);
	if (Status == 0)
		m_pdh_value_Network_Send = CounterValue.doubleValue;

	// PDH_Initial에서 만들어진 PDH 카운터를 다룬 PDH 카운터와 같은 방법으로 사용.
	// 이터넷 개수만큼 돌면서 총 합을 뽑음.
	for (int iCnt = 0; iCnt < df_PDH_ETHERNET_MAX; ++iCnt)
	{
		if (m_EthernetStruct[iCnt]._bUse)
		{
			Status = PdhGetFormattedCounterValue(m_EthernetStruct[iCnt]._pdh_Counter_Network_RecvBytes, PDH_FMT_DOUBLE, NULL, &CounterValue);
			if (Status == 0)
				m_pdh_value_Network_RecvBytes += CounterValue.doubleValue;

			Status = PdhGetFormattedCounterValue(m_EthernetStruct[iCnt]._pdh_Counter_Network_SendBytes, PDH_FMT_DOUBLE, NULL, &CounterValue);
			if (Status == 0)
				m_pdh_value_Network_SendBytes += CounterValue.doubleValue;
		}
	}

	return true;
}

void NOH::CResourceMonitor::OutputToConsole_ServerData(void)
{
	system("cls");
	wprintf(L"====================    COMMON   ====================\n");
    wprintf(L"CPU       Usage  : %10.1f %%\n", m_pdh_Value_CPU_Total);
    wprintf(L"User      Usage  : %10.1f %%\n", m_pdh_Value_User);
    wprintf(L"Kernel    Usage  : %10.1f %%\n", m_pdh_Value_Kernel);

	wprintf(L"Available Memory : %10.1f Mbs\n",GetSystem_Memory_Available() / 1024 / 1024);
	wprintf(L"Nonpaged  Memory : %10.1f Mbs\n", GetSystem_Nonpapged_Memory_Available() / 1024 / 1024);
	wprintf(L"Network   Recv   : %10.1f Kbs/sec\n", GetSystem_Network_Receive() / 1024);
	wprintf(L"Network   Send   : %10.1f Kbs/sec\n", GetSystem_Network_Send() / 1024);
}
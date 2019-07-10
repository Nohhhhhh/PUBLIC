#include "stdafx.h"
#include "ResourceMonitor.h"

#include <strsafe.h>

NOH::CResourceMonitor::CResourceMonitor()
{
	// CPU �ھ� ������ ���Ѵ�.
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
// ���� ����� PDH �غ�.
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

	// PDH enum Object�� ����ϴ� ���
	// ��� �̴��� �̸��� �������� ���� ������� �̴���, �����̴��� ����� Ȯ�� �Ұ�

	// PdhEnumObjectItems �� ���ؼ� "NetworkInterface" �׸񿡼� ���� �� �ִ�
	// �����׸�(Counters) / �������̽� �׸�(Interfaces) �� ����. �׷��� �� ������ ���̸� �𸣱� ������
	// ���� ������ ���̸� �˱� ���ؼ� Out Buffer ���ڵ��� NULL �����ͷ� �־ ����� Ȯ��.
	PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0);

	szCounters = new WCHAR[dwCounterSize];
	szInterfaces = new WCHAR[dwInterfaceSize];

	// ������ �����Ҵ� �� �ٽ� ȣ��!
	// szCounters�� szInterfaces ���ۿ��� ���� ���� ���ڿ��� ���´�. 2���� �迭�� �ƴϰ�, �׳� NULL �����ͷ� ������ ���ڿ�����
	// dwCounterSize, dwInterfaceSize ���̸�ŭ ������ ����ִ�.
	// �̸� �L�� ������ ��� ������ Ȯ�� �ؾ� ��. ex) aaa\0bbb\0ccc\0ddd

	if (PdhEnumObjectItems(NULL, NULL, L"Network Interface", szCounters, &dwCounterSize, szInterfaces, &dwInterfaceSize, PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS)
	{
		delete[] szCounters;
		delete[] szInterfaces;
		return false;
	}

	iCnt = 0;
	szCur = szInterfaces;

	// szInterfaces ���� ���ڿ� ������ �����鼭, �̸��� ���� �޴´�.
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
// PDH ���� ����.  �ý��� ���� ��� �� ȣ�� ���־�� ��.
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

	// PDH_Initial���� ������� PDH ī���͸� �ٷ� PDH ī���Ϳ� ���� ������� ���.
	// ���ͳ� ������ŭ ���鼭 �� ���� ����.
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
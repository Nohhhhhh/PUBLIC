#pragma once

// Performance Data Helper
#include <Pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")

#define df_PDH_ETHERNET_MAX		8

//-----------------------------------------------
// ���ͳ� �ϳ��� ���� Send, Recv PDH ���� ����
//-----------------------------------------------
struct st_ETHERNET
{
	bool	_bUse;
	WCHAR	_szName[128];

	PDH_HCOUNTER	_pdh_Counter_Network_RecvBytes;
	PDH_HCOUNTER	_pdh_Counter_Network_SendBytes;
};

namespace NOH
{
    class CUsageCPU;
	class CResourceMonitor
	{
	public:

		CResourceMonitor();
		virtual ~CResourceMonitor();

		void Stop();

		///////////////////////////////////////////////////////////////
		// ���ҽ� ����͸� PDH �غ�.
		///////////////////////////////////////////////////////////////
		bool PDH_Initial(void);

		///////////////////////////////////////////////////////////////
		// System PerformanceUpdate()
		//
		// PDH ���� ����.  �ý��� ���� ��� �� ȣ�� ���־�� ��.
		///////////////////////////////////////////////////////////////
		bool SystemPDH_Update(void);

		double			GetSystem_Idle(void) { return m_pdh_Value_Idle; }
        double			GetSystem_User(void) { return m_pdh_Value_User; }
        double			GetSystem_Kernel(void) { return m_pdh_Value_Kernel; }
		double			GetSystem_CPU_Total(void) { return m_pdh_Value_CPU_Total; }
		double			GetSystem_CPU_Core1(void) { return m_pdh_Value_CPU_Core1; }
		double			GetSystem_CPU_Core2(void) { return m_pdh_Value_CPU_Core2; }
		double			GetSystem_CPU_Core3(void) { return m_pdh_Value_CPU_Core3; }
		double			GetSystem_CPU_Core4(void) { return m_pdh_Value_CPU_Core4; }

		double			GetSystem_Memory_Available(void) { return m_pdh_value_Memory_Available; }

		double			GetSystem_Nonpapged_Memory_Available(void) { return m_pdh_value_Nonpaged_Memory_Available; }

		double			GetSystem_Network_Receive(void) { return m_pdh_value_Network_RecvBytes; }

		double			GetSystem_Network_Send(void) { return m_pdh_value_Network_SendBytes; }

	private:
		int				m_iCoreNum;

		PDH_HQUERY		m_pdh_Query;

		PDH_HCOUNTER	m_pdh_Counter_Idle;				// ��ü Idle ����
        PDH_HCOUNTER	m_pdh_Counter_User;			    // ��ü User ����
        PDH_HCOUNTER	m_pdh_Counter_Kernel;			// ��ü Kernel ����
		PDH_HCOUNTER	m_pdh_Counter_CPU_Total;			// ��ü CPU ����
		PDH_HCOUNTER	m_pdh_Counter_CPU_Core1;			// �ھ CPU ����
		PDH_HCOUNTER	m_pdh_Counter_CPU_Core2;
		PDH_HCOUNTER	m_pdh_Counter_CPU_Core3;
		PDH_HCOUNTER	m_pdh_Counter_CPU_Core4;

		double			m_pdh_Value_Idle;
        double			m_pdh_Value_User;
        double			m_pdh_Value_Kernel;
		double			m_pdh_Value_CPU_Total;
		double			m_pdh_Value_CPU_Core1;
		double			m_pdh_Value_CPU_Core2;
		double			m_pdh_Value_CPU_Core3;
		double			m_pdh_Value_CPU_Core4;

		PDH_HCOUNTER	m_pdh_Counter_Memory_Available;		// ��밡�� ���� �޸�
				
		double			m_pdh_value_Memory_Available;

		PDH_HCOUNTER	m_pdh_Counter_Nonpaged_Memory_Available;		// nonpaged memory bytes

		double			m_pdh_value_Nonpaged_Memory_Available;

		PDH_HCOUNTER	m_pdh_Counter_Network_Receive;		// ��Ʈ��ũ �۽� �� bytes/sec
		double			m_pdh_value_Network_Receive;
		PDH_HCOUNTER	m_pdh_Counter_Network_Send;		    // ��Ʈ��ũ ���� �� bytes/sec
		double			m_pdh_value_Network_Send;

		st_ETHERNET		m_EthernetStruct[df_PDH_ETHERNET_MAX];	// ��ī�� �� PDH ����
		double			m_pdh_value_Network_RecvBytes;			// �� Recv Bytes ��� �̴����� Recv ��ġ �ջ�
		double			m_pdh_value_Network_SendBytes;			// �� Send Bytes ��� �̴����� Send ��ġ �ջ�

	public:
		void OutputToConsole_ServerData(void);

	public:
		bool					m_bShutDown;

	};
}

#pragma once

// Performance Data Helper
#include <Pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")

#define df_PDH_ETHERNET_MAX		8

//-----------------------------------------------
// 이터넷 하나에 대한 Send, Recv PDH 쿼리 정보
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
		// 리소스 모니터링 PDH 준비.
		///////////////////////////////////////////////////////////////
		bool PDH_Initial(void);

		///////////////////////////////////////////////////////////////
		// System PerformanceUpdate()
		//
		// PDH 쿼리 갱신.  시스템 정보 얻기 전 호출 해주어야 함.
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

		PDH_HCOUNTER	m_pdh_Counter_Idle;				// 전체 Idle 사용률
        PDH_HCOUNTER	m_pdh_Counter_User;			    // 전체 User 사용률
        PDH_HCOUNTER	m_pdh_Counter_Kernel;			// 전체 Kernel 사용률
		PDH_HCOUNTER	m_pdh_Counter_CPU_Total;			// 전체 CPU 사용률
		PDH_HCOUNTER	m_pdh_Counter_CPU_Core1;			// 코어별 CPU 사용률
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

		PDH_HCOUNTER	m_pdh_Counter_Memory_Available;		// 사용가능 실제 메모리
				
		double			m_pdh_value_Memory_Available;

		PDH_HCOUNTER	m_pdh_Counter_Nonpaged_Memory_Available;		// nonpaged memory bytes

		double			m_pdh_value_Nonpaged_Memory_Available;

		PDH_HCOUNTER	m_pdh_Counter_Network_Receive;		// 네트워크 송신 량 bytes/sec
		double			m_pdh_value_Network_Receive;
		PDH_HCOUNTER	m_pdh_Counter_Network_Send;		    // 네트워크 수신 량 bytes/sec
		double			m_pdh_value_Network_Send;

		st_ETHERNET		m_EthernetStruct[df_PDH_ETHERNET_MAX];	// 랜카드 별 PDH 정보
		double			m_pdh_value_Network_RecvBytes;			// 총 Recv Bytes 모든 이더넷의 Recv 수치 합산
		double			m_pdh_value_Network_SendBytes;			// 총 Send Bytes 모든 이더넷의 Send 수치 합산

	public:
		void OutputToConsole_ServerData(void);

	public:
		bool					m_bShutDown;

	};
}

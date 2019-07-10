// SystemMonitor.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "ResourceMonitor.h"

int main()
{
    NOH::CResourceMonitor *pResourceMonitor = new NOH::CResourceMonitor;

	if(!pResourceMonitor->PDH_Initial())
		return 0;

	while (!pResourceMonitor->m_bShutDown)
	{
		pResourceMonitor->SystemPDH_Update();

		pResourceMonitor->OutputToConsole_ServerData();

		Sleep(1000);
	}

    return 0;
}


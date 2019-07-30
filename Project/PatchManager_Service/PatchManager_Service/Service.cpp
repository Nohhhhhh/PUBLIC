#include "stdafx.h"
#include "Service.h"

CService::CService()
{
    m_SCManager = NULL;
    m_SCService = NULL;
    m_dwError = 0;
    bOpenFlag = false;

    // 1. 서비스 매니저 오픈
    m_SCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (NULL == m_SCManager)
        m_dwError = GetLastError();
    else
        bOpenFlag = true;
}

CService::~CService()
{
    CloseServiceHandle(m_SCManager);
    // 서비스 핸들 모두 닫기
    CloseServiceHandle(m_SCService);
}

int CService::ServiceCreate(WCHAR * wServiceName, WCHAR * wDisplayServiceName, WCHAR * wExePath, WCHAR * wDescription)
{
    // 1. 오픈한 서비스 핸들로 서비스 생성
    // SERVICE_INTERACTIVE_PROCESS 는 서비스 -> 자신의 서비스 속성 -> 로그온 탭 -> 로컬시스템계정 (서비스와 테스크톱 상호 작용 허용) 이 체크되어있는 상태와 동일
    m_SCService = CreateService(
        m_SCManager,
        wServiceName,
        wDisplayServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        wExePath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if (NULL == m_SCService)
    {
        DWORD dwError = GetLastError();
        return dwError;
    }

    SERVICE_DESCRIPTION SVDesc;
    SVDesc.lpDescription = wDescription;

    // 2. 생성된 서비스에 서비스 설명 내용을 변경
    BOOL bServiceConfig = ChangeServiceConfig2(m_SCService, SERVICE_CONFIG_DESCRIPTION, &SVDesc);

    if (FALSE == bServiceConfig)
    {
        CloseServiceHandle(m_SCService);
        m_SCService = NULL;

        DWORD dwError = GetLastError();
        return dwError;
    }

    CloseServiceHandle(m_SCService);
    m_SCService = NULL;

    return ERROR_SUCCESS;
}

int CService::ServiceDelete(WCHAR * wServiceName)
{
    // 1. 서비스 이름을 이용해서 특정 서비스를 오픈
    m_SCService = OpenService(
        m_SCManager,
        wServiceName,
        SERVICE_ALL_ACCESS);

    if (NULL == m_SCService)
    {
        DWORD dwError = GetLastError();
        return dwError;
    }

    // 2. 특정 서비스를 삭제
    BOOL bDelete = DeleteService(m_SCService);

    if (FALSE == bDelete)
    {
        DWORD dwError = GetLastError();
        CloseServiceHandle(m_SCService);
        m_SCService = NULL;
        return dwError;
    }

    CloseServiceHandle(m_SCService);
    m_SCService = NULL;

    return ERROR_SUCCESS;
}

int CService::ServiceStart(WCHAR * wServiceName)
{
    // 1. 서비스 이름으로 특정 서비스를 오픈
    m_SCService = OpenService(
        m_SCManager,
        wServiceName,
        SERVICE_ALL_ACCESS);

    if (NULL == m_SCService)
    {
        DWORD dwError = GetLastError();
        return dwError;
    }

    // 2. 서비스 시작
    BOOL bStart = StartService(m_SCService, 0, NULL);

    if (FALSE == bStart)
    {
        DWORD dwError = GetLastError();

        CloseServiceHandle(m_SCService);
        m_SCService = NULL;
        return dwError;
    }

    CloseServiceHandle(m_SCService);
    m_SCService = NULL;
    return ERROR_SUCCESS;

}

int CService::ServiceStop(WCHAR * wServiceName)
{
    // 1. 서비스 이름으로 특정 서비스를 오픈
    m_SCService = OpenService(
        m_SCManager,
        wServiceName,
        SERVICE_ALL_ACCESS);

    if (NULL == m_SCService)
    {
        DWORD dwError = GetLastError();
        return dwError;
    }

    SERVICE_STATUS SS;

    // 2. 서비스의 상태를 요청
    BOOL bQuery = QueryServiceStatus(m_SCService, &SS);

    if (FALSE == bQuery)
    {
        DWORD dwError = GetLastError();
        CloseServiceHandle(m_SCService);
        m_SCService = NULL;
        return dwError;
    }

    do
    {
        // 3. 서비스 상태가 STOP 상태가 아니라면 STOP을 요청
        if (SERVICE_STOPPED != SS.dwCurrentState)
        {
            BOOL bControl = ControlService(m_SCService, SERVICE_CONTROL_STOP, &SS);
 
            if (FALSE == bControl)
            {
                DWORD dwError = GetLastError();
                CloseServiceHandle(m_SCService);
                m_SCService = NULL;
                return dwError;
            }

            bQuery = QueryServiceStatus(m_SCService, &SS);

            if (FALSE == bQuery)
            {
                DWORD dwError = GetLastError();
                CloseServiceHandle(m_SCService);
                m_SCService = NULL;
                return dwError;
            }
        }
    } while (SERVICE_STOPPED != SS.dwCurrentState);

    CloseServiceHandle(m_SCService);
    m_SCService = NULL;

    return ERROR_SUCCESS;
}

DWORD CService::GetCurServiceStatus(WCHAR * wServiceName, WCHAR * wBuffer)
{
    // 1. 서비스 이름으로 특정 서비스를 오픈
    m_SCService = OpenService(
        m_SCManager,
        wServiceName,
        SERVICE_ALL_ACCESS);

    if (NULL == m_SCService)
    {
        DWORD dwError = GetLastError();
        return dwError;
    }

    SERVICE_STATUS SS;

    // 2. 서비스의 상태를 요청
    BOOL bQuery = QueryServiceStatus(m_SCService, &SS);

    if (FALSE == bQuery)
    {
        DWORD dwError = GetLastError();
        CloseServiceHandle(m_SCService);
        m_SCService = NULL;
        return dwError;
    }

    CloseServiceHandle(m_SCService);
    m_SCService = NULL;

    switch (SS.dwCurrentState)
    {
    case SERVICE_RUNNING:
        swprintf_s(wBuffer, 128, L"실행 상태");
        break;
    case SERVICE_STOPPED:
        swprintf_s(wBuffer, 128, L"중지 상태");
        break;
    default:
        swprintf_s(wBuffer, 128, L"서비스 해제 후, 다시 진행 요망");
        break;
    }

    return SS.dwCurrentState;
}

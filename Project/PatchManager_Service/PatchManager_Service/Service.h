#pragma once

class CService
{
public:
    CService();
    virtual ~CService();

    int ServiceCreate(WCHAR * wServiceName, WCHAR * wDisplayServiceName, WCHAR * wExePath, WCHAR * wDescription);
    int ServiceDelete(WCHAR * wServiceName);
    int ServiceStart(WCHAR * wServiceName);
    int ServiceStop(WCHAR * wServiceName);
    DWORD GetCurServiceStatus(WCHAR * wServiceName, WCHAR * wBuffer);

    bool        bOpenFlag;
    DWORD       m_dwError;

    SC_HANDLE   m_SCManager;
    SC_HANDLE   m_SCService;
};
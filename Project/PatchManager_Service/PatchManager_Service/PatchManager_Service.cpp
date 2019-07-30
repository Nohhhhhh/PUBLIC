// PatchManager_Service.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "Service.h"

CService g_Service;

int main()
{
    _wsetlocale(LC_ALL, L"korean");

    WCHAR wDirBuffer[256];

    GetCurrentDirectoryW(256, wDirBuffer);

    if (true == g_Service.bOpenFlag)
    {
        swprintf_s(wDirBuffer, 256, L"%s\\PatchManager_Server.exe", wDirBuffer);
        wprintf(L"-> %s 서비스 매니저 오픈 성공!\n", wDirBuffer);
        int iRetVal = g_Service.ServiceCreate(L"PatchManager_Server", L"PatchManager_Server", wDirBuffer, L"패치서버");

        switch (iRetVal)
        {
        case ERROR_SUCCESS:
        case ERROR_SERVICE_EXISTS:
            wprintf(L"-> PatchManager_Server.exe 서비스 등록\n");
            break;
        /*case ERROR_SERVICE_EXISTS:
            wprintf(L"-> 이미 PatchManager_Server.exe 서비스 등록\n");
            break;*/
        default:
            wprintf(L"-> PatchManager_Server.exe 서비스 등록 실패!\n");
            return 0;
        }

        wprintf(L"\n...\n");

        bool bKeyFlag = false;
        WCHAR wBuffer[128];

        while (!bKeyFlag)
        {
            Sleep(1000);

            wmemset(wBuffer, 0, 128);
            DWORD dwCurStatus = g_Service.GetCurServiceStatus(L"PatchManager_Server", wBuffer);

            system("cls");

            wprintf(L"\n*** 서비스 수행 가능 목록 *** (현재 서비스: %s)\n", wBuffer);
            wprintf(L" 1. 서비스 실행\n");
            wprintf(L" 2. 서비스 중지\n");
            wprintf(L" 0. 서비스 해제\n");

            if (_kbhit())
            {
                WCHAR wcControlKey = _getwch();

                switch (wcControlKey)
                {
                case L'1':
                    if (SERVICE_STOPPED == dwCurStatus)
                    {
                        wprintf(L"\n: PatchManager_Server.exe 시작!\n");
                        g_Service.ServiceStart(L"PatchManager_Server");
                    }
                    Sleep(500);
                    break;
                case L'2':
                    if (SERVICE_RUNNING == dwCurStatus)
                    {
                        wprintf(L"\n: PatchManager_Server.exe 중지!\n");
                        g_Service.ServiceStop(L"PatchManager_Server");
                    }
                    Sleep(500);
                    break;
                case L'0':
                    if (SERVICE_STOPPED == dwCurStatus)
                    {
                        wprintf(L"\n: PatchManager_Server.exe 서비스 해제\n");
                        g_Service.ServiceDelete(L"PatchManager_Server");
                        bKeyFlag = true;
                    }
                    else
                    {
                        wprintf(L"\n: PatchManager_Server.exe가 실행 중...\n");
                    }
                    Sleep(500);
                    break;
                }
            }
        }
    }
    else
    {
        wprintf(L"-> PatchManager_Server.exe 서비스 매니저 오픈 실패!\n");
    }

    return 0;
}
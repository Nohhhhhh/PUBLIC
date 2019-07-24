#include "Dump.h"

#include <DbgHelp.h>
#include <stdio.h>
#include <crtdbg.h>
#include <Psapi.h>

long NOH::CDump::m_DumpCount;
LPTOP_LEVEL_EXCEPTION_FILTER NOH::CDump::PreviousExceptionFilter;
NOH::CDump::CDump()
{
    m_DumpCount = 0;
    PreviousExceptionFilter = nullptr;

	_invalid_parameter_handler oldHandler, newHandler;
	newHandler = myInvaildParameterHandler;

	// CRT가 잘못된 인수를 발견할 때 호출할 함수를 설정
	// return : A pointer to the invalid parameter handler before the call
	oldHandler = _set_invalid_parameter_handler(newHandler);

	// CRT 오류 메시지 표시 중단하고 바로 덤프로 남도록 함
	_CrtSetReportMode(_CRT_WARN, 0);
	_CrtSetReportMode(_CRT_ASSERT, 0);
	_CrtSetReportMode(_CRT_ERROR, 0);

	// 순수 가상 함수 호출에 대한 오류 처리
	// pure virtual function called 에러 핸들러를 사용자 정의 함수로 우회
	_set_purecall_handler(myPurecallHandler);

	SetHandlerDump();
}

NOH::CDump::~CDump()
{
    SetUnhandledExceptionFilter(PreviousExceptionFilter);
}

LONG NOH::CDump::MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
{
    int iWorkingMemory = 0;
	SYSTEMTIME stNowTime;

	long DumpCount = InterlockedIncrement(&m_DumpCount);

	// 현재 프로세스의 메모리 사용량 얻기
	HANDLE hProcess = 0;
	PROCESS_MEMORY_COUNTERS pmc;

	hProcess = GetCurrentProcess();

	if (nullptr == hProcess)
		return 0;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);

	CloseHandle(hProcess);

	// 현재 날짜와 시간을 알아온다.
	WCHAR filename[MAX_PATH];

	// 현재 시간 구하기
	GetLocalTime(&stNowTime);

	wsprintf(filename, L"DUMP_%d%02d%02d_%02dh%02dm_%ld_%dMB.dmp", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, DumpCount, iWorkingMemory);

	/*wprintf(L"\n\n\n!!! Crash Error !!!  %d.%d.%d / %d:%d:%d \n", stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
	wprintf(L"Now Save dump file ...\n");*/

	HANDLE hDumpFile = CreateFile(
        filename,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		// _MINIDUMP_EXCEPTION_INFORMATION
		// Contains the exception information written to the minidump file
		_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

		MinidumpExceptionInformation.ThreadId = GetCurrentThreadId();
		MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
		MinidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(
            GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&MinidumpExceptionInformation,
			nullptr,
			nullptr);

		CloseHandle(hDumpFile);

		wprintf(L"CrashDump Save Finish !");
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void NOH::CDump::SetHandlerDump()
{
    PreviousExceptionFilter = SetUnhandledExceptionFilter(MyExceptionFilter);
}

void NOH::CDump::myInvaildParameterHandler(const wchar_t * expression, const wchar_t * function, const wchar_t * file, unsigned int lien, uintptr_t pReserverd)
{
    Crash();
}

void NOH::CDump::_custom_Report_hook(int ireposttype, char * message, int * returnvalue)
{
    Crash();
}

void NOH::CDump::myPurecallHandler(void)
{
    Crash();
}

void NOH::CDump::Crash(void)
{
	int *p = nullptr;
	*p = 0;
}
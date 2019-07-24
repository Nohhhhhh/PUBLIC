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

	// CRT�� �߸��� �μ��� �߰��� �� ȣ���� �Լ��� ����
	// return : A pointer to the invalid parameter handler before the call
	oldHandler = _set_invalid_parameter_handler(newHandler);

	// CRT ���� �޽��� ǥ�� �ߴ��ϰ� �ٷ� ������ ������ ��
	_CrtSetReportMode(_CRT_WARN, 0);
	_CrtSetReportMode(_CRT_ASSERT, 0);
	_CrtSetReportMode(_CRT_ERROR, 0);

	// ���� ���� �Լ� ȣ�⿡ ���� ���� ó��
	// pure virtual function called ���� �ڵ鷯�� ����� ���� �Լ��� ��ȸ
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

	// ���� ���μ����� �޸� ��뷮 ���
	HANDLE hProcess = 0;
	PROCESS_MEMORY_COUNTERS pmc;

	hProcess = GetCurrentProcess();

	if (nullptr == hProcess)
		return 0;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
		iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);

	CloseHandle(hProcess);

	// ���� ��¥�� �ð��� �˾ƿ´�.
	WCHAR filename[MAX_PATH];

	// ���� �ð� ���ϱ�
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
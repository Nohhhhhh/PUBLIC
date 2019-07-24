/*
    CDump::GetInstance()->Crash();
*/
#pragma once

#include <WinSock2.h>
#include "Singleton.h"

#define CRASH() NOH::CDump::GetInstance()->Crash();

namespace NOH
{
    class CDump : public CSingleton<CDump>
	{
	public:
		// Identifies the type of information that will be written to the minidump file by the MiniDumpWriteDump function.
		CDump();
        ~CDump();

	public:
        static long WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer);
		static void SetHandlerDump();

		// Invalid Parameter handler
		static void myInvaildParameterHandler(const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int lien, uintptr_t pReserverd);
		static void _custom_Report_hook(int ireposttype, char *message, int *returnvalue);
		static void myPurecallHandler(void);
        
		static void Crash(void);

    private:
        static long m_DumpCount;
        static LPTOP_LEVEL_EXCEPTION_FILTER PreviousExceptionFilter;
	};
}
#pragma once

#include <WinSock2.h>
#include "Singleton.h"
#include "client\windows\handler\exception_handler.h"
#define CRASH_B() NOH::CBreakpad::GetInstance()->Crash();


namespace google_breakpad
{
    class ExceptionHandler;
    class CrashGenerationServer;
}

namespace NOH
{
    class CBreakpad : public CSingleton<CBreakpad>
	{
	public:
		// Identifies the type of information that will be written to the minidump file by the MiniDumpWriteDump function.
		CBreakpad();
        ~CBreakpad();

	public:
        static bool MiniDump_CallBack(const wchar_t* wcDumpPath, const wchar_t* wcMiniDumpID, void* Context, EXCEPTION_POINTERS* pExInfo, MDRawAssertionInfo* pAssertion, bool bSucceeded);
        
        static void CrashServerStart();
        static void CrashServerStop();

    private:
        static google_breakpad::ExceptionHandler *m_pHandler;
        static google_breakpad::CrashGenerationServer* m_pCrashServer;
        static wchar_t *m_pPipeName;
	};
}
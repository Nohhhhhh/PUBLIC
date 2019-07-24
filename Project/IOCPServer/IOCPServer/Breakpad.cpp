#include "Breakpad.h"
#include "client\windows\common\ipc_protocol.h"
#include "client\windows\sender\crash_report_sender.h"
#include "client\windows\crash_generation\crash_generation_server.h"

wchar_t *NOH::CBreakpad::m_pPipeName;
google_breakpad::ExceptionHandler *NOH::CBreakpad::m_pHandler;
google_breakpad::CrashGenerationServer *NOH::CBreakpad::m_pCrashServer;

static size_t kCustomInfoCount = 2;
static google_breakpad::CustomInfoEntry kCustomInfoEntries[] = {
    google_breakpad::CustomInfoEntry(L"prod", L"noh_test"),
    google_breakpad::CustomInfoEntry(L"ver", L"1.0"),
};

NOH::CBreakpad::CBreakpad()
{
    m_pPipeName = L"\\\\.\\pipe\\BreakpadCrashServices";

    google_breakpad::CustomClientInfo CustomInfo = {kCustomInfoEntries, kCustomInfoCount};
    m_pHandler = new google_breakpad::ExceptionHandler(L"C:\\NOH\\", 
                                                        NULL, 
                                                        NOH::CBreakpad::MiniDump_CallBack, 
                                                        NULL, 
                                                        google_breakpad::ExceptionHandler::HANDLER_ALL, 
                                                        MiniDumpNormal, 
                                                        m_pPipeName, 
                                                        &CustomInfo);
    
}

NOH::CBreakpad::~CBreakpad()
{
    CrashServerStop();
}

bool NOH::CBreakpad::MiniDump_CallBack(const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
{
    WCHAR* wcpText = new TCHAR[256];
    wcpText[0] = _T('\0');

    int result = swprintf_s(wcpText,
        256,
        TEXT("Dump generation request %s\r\n"),
        succeeded ? TEXT("succeeded") : TEXT("failed"));

    if (result == -1) {
        delete[] wcpText;
    }

    return succeeded;
}

void NOH::CBreakpad::CrashServerStart()
{
    if (m_pCrashServer) {
        return;
    }

    std::wstring wstrDumpPath = L"C:\\DUMP\\";

    if (_wmkdir(wstrDumpPath.c_str()) && (errno != EEXIST)) {
        //MessageBoxW(NULL, L"Unable to create dump directory", L"Dumper", MB_OK);
        return;
    }

    m_pCrashServer = new google_breakpad::CrashGenerationServer(m_pPipeName,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        true,
        &wstrDumpPath);

    if (!m_pCrashServer->Start()) {
        //MessageBoxW(NULL, L"Unable to start server", L"Dumper", MB_OK);
        delete m_pCrashServer;
        m_pCrashServer = nullptr;
    }
}

void NOH::CBreakpad::CrashServerStop()
{
    delete m_pCrashServer;
    m_pCrashServer = nullptr;
}

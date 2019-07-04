/*
- 사용 방법
.h 파일
extern NOH::CSystemLog   g_LogCore;
extern NOH::CSystemLog   g_LogGameServer;

.cpp 파일
// 타이머 시작 - 반드시 타이머 부터 시작해야 함.
NOH::CSystemLog   g_LogCore( PATH_LOGFILE, *L"LOG_CORE", NOH::LOG_LEVEL::LEVEL_SYSTEM, NOH::LOG_WRITE_MODE::WRITE_FILE );
NOH::CSystemLog   g_LogGameServer( PATH_LOGFILE, *L"LOG_GAMESERVER", NOH::LOG_LEVEL::LEVEL_SYSTEM, NOH::LOG_WRITE_MODE::WRITE_FILE );

- 실제 사용
SYSLOG(g_LogCore, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"test log %s", w.c_str());
SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"test log %d", iiiii );
*/

#pragma once

#include <atlbase.h>
#include <memory>

#include "__NOH.h"


#define SYSLOG(object, level, formatstring, ...) object.Log(level, formatstring, __VA_ARGS__)
//#define SYSLOG_HEX(type, level, logstring, byte, len) g_SystemLog->LogHex(type, level, logstring, byte, len)
//#define SYSLOG_SESSIONKEY(type, level, logstring, sessionkey1, sessionkey2) g_SystemLog->LogSessionKey(type, level, logstring, sessionkey1, sessionkey2)
//#define SYSLOG_DIRECTORY(Dir) g_SystemLog->SetLogDirectory(Dir)

namespace NOH
{
    class CFileStream;
    class CSystemLog
    {
    public:
        CSystemLog( const WCHAR &LogPath, const WCHAR &FolderName, LOG_LEVEL LogLevel, LOG_WRITE_MODE WriteMode );
        ~CSystemLog();

        void Log( LOG_LEVEL LogLevel, LPCWSTR Format, ... );

    private:
        void Initialize( void );
        int CheckLogLevel( LOG_LEVEL LogLevel, WCHAR *wpLogLevel );
        void LogConsole( const WCHAR &wLogLevel, const WCHAR &wBuffer );
        void LogText( const WCHAR &wLogLevel, WCHAR &wBuffer);
        void LogDB(void) {}       // TODO
        bool Open( LPCWSTR FileName );
        void Write( LPVOID wBuffer, DWORD & dwBufferLen );
        void Close( void );

    private:
        std::unique_ptr<CFileStream>    m_spFileStream;

        std::unique_ptr<WCHAR[]>        m_spFullName;
        std::unique_ptr<WCHAR[]>        m_spDirectory;
		const WCHAR                     &m_wPath;
        const WCHAR                     &m_wFolderName;

        LOG_LEVEL                       m_LogLevel;
        LOG_WRITE_MODE                  m_WriteMode;
        ULONGLONG                       m_ullLogNo;

        SRWLOCK                         m_SRWLock;

        WORD                            m_wLastDay;
    };
}
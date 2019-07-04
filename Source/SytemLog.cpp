#include "SytemLog.h"
#include "LockGuard.h"
#include "TimerManager.h"
#include "FileStream.h"
#include "Dump.h"

#include <algorithm>
#include <ShlObj.h>
#include <Strsafe.h>


NOH::CSystemLog::CSystemLog(const WCHAR &LogPath, const WCHAR &FolderName, LOG_LEVEL LogLevel, LOG_WRITE_MODE WriteMode)
    : m_spFileStream(std::make_unique<CFileStream>()), m_spFullName(std::make_unique<WCHAR[]>( static_cast<int>( COMMON::BUF_256 ) )), m_spDirectory(std::make_unique<WCHAR []>( static_cast<int>( COMMON::BUF_256 ) )), m_wPath(LogPath), m_wFolderName(FolderName), m_LogLevel(LogLevel), m_WriteMode(WriteMode), m_ullLogNo(0), m_wLastDay(0)
{
    Initialize();
}

NOH::CSystemLog::~CSystemLog()
{
}

void NOH::CSystemLog::Log(LOG_LEVEL LogLevel, LPCWSTR Format, ...)
{
    WCHAR _wloglevel[ static_cast<int>(COMMON::BUF_32) ] = { 0 };

    // 로그레벨 체크
    if ( !CheckLogLevel( LogLevel, _wloglevel) )
        return;

    // 로그 만들기
    WCHAR _wvabuff[static_cast<size_t>( COMMON::BUF_1024 )] = { 0 };

    va_list _va;
	// 가변 인자 바로 앞에 인자.
	va_start( _va, Format );
	// 유니코드 기반에서는 아래의 함수를 사용해야 한다.
	StringCchVPrintf(_wvabuff, static_cast<size_t>( COMMON::BUF_1024 ), Format, _va);
	va_end(_va);

    // 콘솔 로그 남기기
	if ( ( static_cast<int>( m_WriteMode) & static_cast<int>( LOG_WRITE_MODE::CONSOLE ) ) == static_cast<int>( LOG_WRITE_MODE::CONSOLE ) )
	{
	    LogConsole( *_wloglevel, *_wvabuff );
    }

    // 파일 로그 남기기
	if ( ( static_cast<int>( m_WriteMode ) & static_cast<int>( LOG_WRITE_MODE::WRITE_FILE ) ) == static_cast<int>( LOG_WRITE_MODE::WRITE_FILE ) )
	{
        LogText( *_wloglevel, *_wvabuff );
    }


    // DB 로그 남기기
	if ( ( static_cast<int>( m_WriteMode ) & static_cast<int>( LOG_WRITE_MODE::WRITE_DB ) ) == static_cast<int>( LOG_WRITE_MODE::WRITE_DB ) )
	{
        // TODO
    }

}

void NOH::CSystemLog::Initialize(void)
{
    wmemset(m_spFullName.get(), 0, static_cast<size_t>(COMMON::BUF_256));
    wmemset(m_spDirectory.get(), 0, static_cast<size_t>(COMMON::BUF_128));

    swprintf_s(m_spDirectory.get(), static_cast<size_t>(COMMON::BUF_256), L"%s\\%s\\", &m_wPath, &m_wFolderName );

    SHCreateDirectoryExW( nullptr, reinterpret_cast<LPCWSTR>( m_spDirectory.get() ), nullptr );
}

int NOH::CSystemLog::CheckLogLevel(LOG_LEVEL LogLevel, WCHAR *wpLogLevel)
{
    int _iretval = 0;

    if ( ( static_cast<int>( m_LogLevel ) & static_cast<int>( LogLevel ) ) == static_cast<int>( LogLevel ) )
	{
		switch (LogLevel)
		{
		case LOG_LEVEL::LEVEL_DEBUG:
            wcsncpy_s( wpLogLevel, static_cast<size_t>(COMMON::BUF_32), L"  DEBUG", _TRUNCATE );
			break;
		case LOG_LEVEL::LEVEL_WARNING:
            wcsncpy_s( wpLogLevel, static_cast<size_t>(COMMON::BUF_32), L"WARNING", _TRUNCATE );
			break;
		case LOG_LEVEL::LEVEL_ERROR:
            wcsncpy_s( wpLogLevel, static_cast<size_t>(COMMON::BUF_32), L"  ERROR", _TRUNCATE );
			break;
		case LOG_LEVEL::LEVEL_SYSTEM:
            wcsncpy_s( wpLogLevel, static_cast<size_t>(COMMON::BUF_32), L" SYSTEM", _TRUNCATE );
			break;
		}

        return true;
	}

    return false;
}

void NOH::CSystemLog::LogConsole(const WCHAR &wLogLevel, const WCHAR & wBuffer)
{
    // 날짜 얻기
	WORD _wyear = CTimerManager::GetInstance()->GetYear();
	WORD _wmonth = CTimerManager::GetInstance()->GetMonth();
	WORD _wday = CTimerManager::GetInstance()->GetDay();
	WORD _whour = CTimerManager::GetInstance()->GetHour();
	WORD _wminute = CTimerManager::GetInstance()->GetMinute();
	WORD _wsecond = CTimerManager::GetInstance()->GetSecond();

    wprintf_s(L"[%s] [%d-%02d-%02d %02d:%02d:%02d] %s\n", &wLogLevel, _wyear, _wmonth, _wday, _whour, _wminute, _wsecond, &wBuffer);
}

void NOH::CSystemLog::LogText(const WCHAR &wLogLevel, WCHAR & wBuffer)
{
    // 날짜 얻기
	WORD _wyear = CTimerManager::GetInstance()->GetYear();
	WORD _wmonth = CTimerManager::GetInstance()->GetMonth();
	WORD _wday = CTimerManager::GetInstance()->GetDay();
	WORD _whour = CTimerManager::GetInstance()->GetHour();
	WORD _wminute = CTimerManager::GetInstance()->GetMinute();
	WORD _wsecond = CTimerManager::GetInstance()->GetSecond();

    if ( _wday != m_wLastDay )
    {
        WCHAR _wfilename[64] = { 0 };

        // 파일 이름 만들기
        HRESULT _hresult = StringCbPrintf( _wfilename, sizeof(_wfilename), L"%d%02d%02d.txt", _wyear, _wmonth, _wday );

	    // 매크로
	    if ( FAILED (_hresult) )
		    return;

        wmemset(m_spFullName.get(), 0, static_cast<size_t>(COMMON::BUF_256));
        swprintf_s(m_spFullName.get(), static_cast<size_t>(COMMON::BUF_256), L"%s%s", m_spDirectory.get(), _wfilename );

		m_wLastDay = _wday;
    }

    CLockGuard lockgaurd( LOCK_TYPE::SRWLOCK_EXCLUSIVE, &m_SRWLock );

    // 파일 오픈
	Close();
    int _iopencnt = 0;
    while( 5 != _iopencnt )
    {
        if ( !Open( reinterpret_cast<LPCWSTR>( m_spFullName.get() ) ) )
            ++_iopencnt;
        else
            break;
    }
    
    if ( 5 == _iopencnt )
        return;

    WCHAR _woutputbuff[static_cast<size_t>( COMMON::BUF_2048 )] = { 0 };
	swprintf_s(_woutputbuff, L"[%s] [%d-%02d-%02d %02d:%02d:%02d / %010llu] %s\r\n", &wLogLevel, _wyear, _wmonth, _wday, _whour, _wminute, _wsecond, m_ullLogNo++, &wBuffer);
	
    DWORD _dwbufflen = static_cast<DWORD>( wcsnlen_s(_woutputbuff, static_cast<size_t>( COMMON::BUF_2048 ) ) * sizeof(WCHAR) ); 
    Write( reinterpret_cast<LPVOID>( _woutputbuff ),  _dwbufflen );

	Close();
}

bool NOH::CSystemLog::Open(LPCWSTR FileName)
{
	if ( !m_spFileStream.get()->Open(FileName, GENERIC_WRITE, OPEN_ALWAYS) )
		return false;

    return true;
}

void NOH::CSystemLog::Write(LPVOID wBuffer, DWORD & dwBufferLen)
{
    if( !m_spFileStream.get()->Seek( 0, FILE_END ) )
		return;

    DWORD _dwbyteswritten = 0;
    m_spFileStream.get()->Write( wBuffer, dwBufferLen, _dwbyteswritten );

    if ( dwBufferLen != _dwbyteswritten )
        CRASH();
}

void NOH::CSystemLog::Close(void)
{
	m_spFileStream.get()->Close();
}

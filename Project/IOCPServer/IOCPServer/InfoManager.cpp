#include "__NOH.h"
#include "InfoManager.h"
#include "SytemLog.h"
#include "TextParser.h"

CLIENT::CInfoManager::CInfoManager()
    : m_spTextParser(std::make_unique<NOH::CTextParser>()), m_spInfoManager(std::make_unique<INFO_MANAGER>()), m_spFilePath(std::make_unique<WCHAR []>( static_cast<int>( NOH::COMMON::BUF_256 ) ))
{
}

CLIENT::CInfoManager::~CInfoManager()
{
}

bool CLIENT::CInfoManager::Initialize(void)
{
    if ( !LoadSystemInfo( PATH_SYSTEM_CONFIG ) )
        return false;

    return true;
}

bool CLIENT::CInfoManager::LoadSystemInfo(const WCHAR &wFileName)
{
    if ( !m_spTextParser.get()->LoadFile( wFileName ) )
	{
		// 실패 로그 & 콘솔 출력
        SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"Fail - Open File [%s]", &wFileName );
		return false;
	}
    
    if ( !m_spTextParser.get()->ProvideArea( *L"SYSTEM" ) )
	{
		// 실패 로그 & 콘솔 출력
        SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"Fail - Read Area [%s] - SYSTEM", &wFileName );
		return false;
	}

    INFO_SYSTEM * _pinfosystem = &m_spInfoManager.get()->SystemInfo;

    bool _bflag1 = m_spTextParser.get()->GetValue( *L"VERSION", &_pinfosystem->iVersion );
    bool _bflag2 = m_spTextParser.get()->GetValue( *L"CLIENT_MAX", &_pinfosystem->iClient_Max) ;
    bool _bflag3 = m_spTextParser.get()->GetValue( *L"PACKET_CODE", &_pinfosystem->iPacket_Code );
    bool _bflag4 = m_spTextParser.get()->GetValue( *L"PACKET_KEY1", &_pinfosystem->iPacket_Key1 );
    bool _bflag5 = m_spTextParser.get()->GetValue( *L"PACKET_KEY2", &_pinfosystem->iPacket_Key2 );
    bool _bflag6 = m_spTextParser.get()->GetValue( *L"PLAYER_TIMEOUT", &_pinfosystem->iPlayer_Timeout );
    bool _bflag7 = m_spTextParser.get()->GetValue( *L"SESSIONKEY_TIMEOUT", &_pinfosystem->iSessionKey_Timeout );
    bool _bflag8 = m_spTextParser.get()->GetValue( *L"HEARTBEAT_TICK", &_pinfosystem->iHeartBeat_Tick );
    bool _bflag9 = m_spTextParser.get()->GetValue( *L"LOG_LEVEL", static_cast<int>( sizeof(_pinfosystem->wLog_Level) ), _pinfosystem->wLog_Level );

    if ( !(_bflag1 & _bflag2 & _bflag3 & _bflag4 & _bflag5 & _bflag6 & _bflag7 & _bflag8 & _bflag9) )
	{
		// 실패 로그 & 콘솔 출력
        SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"Fail - Read Value [%s]", &wFileName );
		return false;
	}

    return true;
}

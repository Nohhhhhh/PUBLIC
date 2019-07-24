// __main.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "Global.h"
#include "Noh.h"
#include "ItemBox.h"
#include "Dump.h"
// 시스템 로그 초기화
NOH::CSystemLog     g_LogCore( PATH_LOGFILE, *L"LOG_CORE", NOH::LOG_LEVEL::LEVEL_SYSTEM, NOH::LOG_WRITE_MODE::WRITE_FILE );
NOH::CSystemLog     g_LogGameServer( PATH_LOGFILE, *L"LOG_GAMESERVER", NOH::LOG_LEVEL::LEVEL_SYSTEM, NOH::LOG_WRITE_MODE::WRITE_FILE );
LONG                g_lCurrentUserCount = 0;
LONG                g_lRemainingSocketCount = 0;
LONG                g_lAcceptTPS = 0;
LONG                g_lRecvPacketTPS = 0;
LONG                g_lSendPacketTPS = 0;

int main()
{
    CLIENT::CNoh _noh;

    timeBeginPeriod(1);

    if ( _noh.Initialize() )
        _noh.Start();

    timeEndPeriod(1);

    return 0;
}
#pragma once

#include <WinSock2.h>
#include <memory>

#include "Global.h"
#include "Singleton.h"
//#include "TextParser.h"

namespace NOH 
{
    class CTextParser;
}

namespace CLIENT
{
    typedef struct st_INFO_SYSTEM
    {
	    int		                    iVersion;
	    int		                    iClient_Max;
	    int		                    iPacket_Code;
	    int		                    iPacket_Key1;
	    int		                    iPacket_Key2;
	    int		                    iPlayer_Timeout;
	    int		                    iSessionKey_Timeout;
	    int		                    iHeartBeat_Tick;
	    WCHAR                       wLog_Level[static_cast<int>( NOH::COMMON::BUF_16 )];

        st_INFO_SYSTEM()
            : iVersion(0), iClient_Max(0), iPacket_Code(0), iPacket_Key1(0), iPacket_Key2(0), iPlayer_Timeout(0), iSessionKey_Timeout(0), iHeartBeat_Tick(0) 
        {
            wmemset(wLog_Level, 0, static_cast<size_t>( NOH::COMMON::BUF_16 ));
        };
    } INFO_SYSTEM;

    typedef struct st_INFO_MANAGER
    {
        INFO_SYSTEM    SystemInfo;

        st_INFO_MANAGER() {}
    } INFO_MANAGER;

    class CInfoManager : public NOH::CSingleton<CInfoManager>
    {
    public:
        CInfoManager();
        ~CInfoManager();

        bool Initialize( void );

    private:
        bool LoadSystemInfo( const WCHAR &wFileName );

    private:
        std::unique_ptr<NOH::CTextParser>   m_spTextParser;                     
        std::unique_ptr<INFO_MANAGER>       m_spInfoManager;
        std::unique_ptr<WCHAR []>           m_spFilePath;
    };
}
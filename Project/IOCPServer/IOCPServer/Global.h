#pragma once

#include "__NOH.h"
#include "SytemLog.h"

#define PROFILIE_CHECK
#define PATH_LOGFILE			*L"C:\\NOH\\Log"
#define PATH_SYSTEM_CONFIG      *L".\\INFO\\system.conf"
#define PATH_PROFILER           *L"C:\\NOH\\Profiler"

#define LOOPBACK_IP             *"0.0.0.0"
#define LOOPBACK_PORT           6000
#define SESSION_COUNT           1

extern NOH::CSystemLog      g_LogCore;
extern NOH::CSystemLog      g_LogGameServer;
extern LONG                 g_lCurrentUserCount;
extern LONG                 g_lRemainingSocketCount;
extern LONG                 g_lAcceptTPS;
extern LONG                 g_lRecvPacketTPS;
extern LONG                 g_lSendPacketTPS;

namespace CLIENT
{
    enum class ITEMBOX_TYPE
    {
        BOX1 = 0,

    };
}
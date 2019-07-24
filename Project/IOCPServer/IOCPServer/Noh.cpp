#include "Noh.h"
#include "ThreadManager.h"
#include "SessionManager.h"
#include "IOCPHandler.h"
#include "LanServer.h"
#include "AcceptServer.h"
#include "Session.h"
#include "SocketManager.h"
#include "User.h"
#include "TimerManager.h"
#include "Packet.h"
#include "Dump.h"
#include "InfoManager.h"
#include "Profiler.h"

#include <conio.h>

CLIENT::CNoh::CNoh()
    : m_bService(true), m_bKeyLock(true), m_bRun(false)
{
}

CLIENT::CNoh::~CNoh()
{
}

bool CLIENT::CNoh::Initialize(void)
{
    // ############## 시작 처리 ##############
    // 메모리 풀 할당
    NOH::CPacket::AllocMemoryPool(300);

    // 타이머 시작
    NOH::CTimerManager::GetInstance()->Initialize();

    // Info 파일 로드
    if (!CLIENT::CInfoManager::GetInstance()->Initialize())
        return false;

    // 프로파일러 초기화
    NOH::CProfiler::GetInstance()->Initialize(PATH_PROFILER);

    // 세션 매니저에서 유저 관리 ( 세션 미리 할당 및 소켓 초기화 ) 
    NOH::CSessionManager<CLIENT::CUser>::GetInstance()->Initiailize(SESSION_COUNT);

    // IOCP 생성/등록 및 worker 스레드 생성
    if (!NOH::CIOCPHandler<CLIENT::CLanServer>::GetInstance()->Initialize())
        return false;

    // 유저 소켓 할당 
    if (!NOH::CSocketManager<CLIENT::CUser>::GetInstance()->Initialize(LOOPBACK_IP, LOOPBACK_PORT, NOH::SOCKET_TYPE::USER, SESSION_COUNT))
        return false;

    // IOCP 생성/등록 및 accept 스레드 생성
    if (NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->Initialize(1))
    {
        // 리슨 소켓 IOCP을 등록 - 단, 순서는 SocketManager 초기화 순서
        // 예) CSocketManager가 CUser(유저), CServer(로그인, 채팅 서버 등) 타입으로 초기화되야 할 때, 
        // SOCKET_TYPE 1이 CUser고 SOCKET_TYPE 2가 CServer 라면, CUSer에 필요한 정보로 아래 함수를 호출하고,
        // 그 뒤에 CServer에 대한 아래 함수를 호출해야 한다.
        NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->AssociateListenSocket(reinterpret_cast<HANDLE>(NOH::CSocketManager<CLIENT::CUser>::GetInstance()->GetListenSocket()), NOH::SOCKET_TYPE::USER);
    }
    else
        return false;

    // 임시
    NOH::CIOCPHandler<CLIENT::CLanServer>::GetInstance()->RunThread();
    NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->RunThread();
    m_bRun = true;

    return true;
}

void CLIENT::CNoh::Start(void)
{
    while (m_bService)
    {
        KeyDetection();
        OutputServerDataToConsole();
        g_lAcceptTPS = g_lRecvPacketTPS = g_lSendPacketTPS = 0;
        Sleep(1000);
    }
}

void CLIENT::CNoh::Stop(void)
{
    // ############## 종료 처리 ##############

    // 1. accept iocp 및 스레드 종료
    NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->Close();

    // 2. 리슨 소켓 종료
    NOH::CSocketManager<CLIENT::CUser>::GetInstance()->Close();

    // 3. 세션 종료
    NOH::CSessionManager<CLIENT::CUser>::GetInstance()->Close();

    // 4. worker iocp 및 스레드 종료
    NOH::CIOCPHandler<CLIENT::CLanServer>::GetInstance()->Close();

    // 5. 모든 thread 정리
    NOH::CThreadManager::GetInstance()->Close();

    // 6. 타이머 
    NOH::CTimerManager::GetInstance()->Close();

    // 7. Packet 제거
    NOH::CPacket::DeleteMemoryPool();
}

void CLIENT::CNoh::KeyDetection(void)
{
    if (_kbhit())
    {
        char cControlKey = _getch();

        if ((false == m_bKeyLock && 'r' == cControlKey) || (false == m_bKeyLock && 'R' == cControlKey))
        {
            if (false == m_bRun)
            {
                SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"################ RUN ################");
                NOH::CIOCPHandler<CLIENT::CLanServer>::GetInstance()->RunThread();
                NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->RunThread();

                m_bRun = true;
            }

            m_bKeyLock = true;
        }
        else if ((false == m_bKeyLock && 'q' == cControlKey) || (false == m_bKeyLock && 'q' == cControlKey))
        {
            if (true == m_bRun)
            {
                SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"################ STOP ################");
                Stop();
                m_bRun = false;
                m_bService = false;
            }

            m_bKeyLock = true;
        }
        else if ('u' == cControlKey || 'U' == cControlKey)
        {
            m_bKeyLock = false;
        }
        else if ('p' == cControlKey || 'P' == cControlKey)
        {
            NOH::CProfiler::GetInstance()->SaveData();
            m_bKeyLock = true;
        }

    }
}

void CLIENT::CNoh::OutputServerDataToConsole(void)
{
    if (m_bRun)
    {
        wprintf(L"====================================================\n");
        wprintf(L"                    Run LanServer                   \n");
        wprintf(L"====================================================\n");
    }
    else
    {
        wprintf(L"====================================================\n");
        wprintf(L"                    Stop LanServer                  \n");
        wprintf(L"====================================================\n");
    }

    wprintf(L"\n");
    wprintf(L"Packet  - Chunk           Count : %ld\n", NOH::CPacket::GetChunkCount());
    wprintf(L"Packet  - Using Block     Count : %ld\n", NOH::CPacket::GetUsingBlockCount());
    wprintf(L"Packet  - Using Chunk     Count : %ld\n", NOH::CPacket::GetUsingChunkCount());
    wprintf(L"User    - Current         Count : %ld\n", g_lCurrentUserCount);
    wprintf(L"Socket  - Remaining       Count : %ld / 6000\n", g_lRemainingSocketCount);
    wprintf(L"Session - Accept            TPS : %ld\n", g_lAcceptTPS);
    wprintf(L"Packet  - Recv              TPS : %ld\n", g_lRecvPacketTPS);
    wprintf(L"Packet  - Send              TPS : %ld\n", g_lSendPacketTPS);
    wprintf(L"====================================================\n\n");
}

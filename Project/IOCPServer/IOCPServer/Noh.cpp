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
    // ############## ���� ó�� ##############
    // �޸� Ǯ �Ҵ�
    NOH::CPacket::AllocMemoryPool(300);

    // Ÿ�̸� ����
    NOH::CTimerManager::GetInstance()->Initialize();

    // Info ���� �ε�
    if (!CLIENT::CInfoManager::GetInstance()->Initialize())
        return false;

    // �������Ϸ� �ʱ�ȭ
    NOH::CProfiler::GetInstance()->Initialize(PATH_PROFILER);

    // ���� �Ŵ������� ���� ���� ( ���� �̸� �Ҵ� �� ���� �ʱ�ȭ ) 
    NOH::CSessionManager<CLIENT::CUser>::GetInstance()->Initiailize(SESSION_COUNT);

    // IOCP ����/��� �� worker ������ ����
    if (!NOH::CIOCPHandler<CLIENT::CLanServer>::GetInstance()->Initialize())
        return false;

    // ���� ���� �Ҵ� 
    if (!NOH::CSocketManager<CLIENT::CUser>::GetInstance()->Initialize(LOOPBACK_IP, LOOPBACK_PORT, NOH::SOCKET_TYPE::USER, SESSION_COUNT))
        return false;

    // IOCP ����/��� �� accept ������ ����
    if (NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->Initialize(1))
    {
        // ���� ���� IOCP�� ��� - ��, ������ SocketManager �ʱ�ȭ ����
        // ��) CSocketManager�� CUser(����), CServer(�α���, ä�� ���� ��) Ÿ������ �ʱ�ȭ�Ǿ� �� ��, 
        // SOCKET_TYPE 1�� CUser�� SOCKET_TYPE 2�� CServer ���, CUSer�� �ʿ��� ������ �Ʒ� �Լ��� ȣ���ϰ�,
        // �� �ڿ� CServer�� ���� �Ʒ� �Լ��� ȣ���ؾ� �Ѵ�.
        NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->AssociateListenSocket(reinterpret_cast<HANDLE>(NOH::CSocketManager<CLIENT::CUser>::GetInstance()->GetListenSocket()), NOH::SOCKET_TYPE::USER);
    }
    else
        return false;

    // �ӽ�
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
    // ############## ���� ó�� ##############

    // 1. accept iocp �� ������ ����
    NOH::CIOCPHandler<CLIENT::CAcceptServer>::GetInstance()->Close();

    // 2. ���� ���� ����
    NOH::CSocketManager<CLIENT::CUser>::GetInstance()->Close();

    // 3. ���� ����
    NOH::CSessionManager<CLIENT::CUser>::GetInstance()->Close();

    // 4. worker iocp �� ������ ����
    NOH::CIOCPHandler<CLIENT::CLanServer>::GetInstance()->Close();

    // 5. ��� thread ����
    NOH::CThreadManager::GetInstance()->Close();

    // 6. Ÿ�̸� 
    NOH::CTimerManager::GetInstance()->Close();

    // 7. Packet ����
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

#include "Session.h"
#include "Packet.h"
#include "IOCPHandler.h"
#include "StreamQueue.h"
#include "Queue_LF.h"
#include "SocketManager.h"

NOH::CSession::CSession()
    : m_SessionCode(0), m_pCompletionKey(nullptr), m_Socket(INVALID_SOCKET), m_spIP(std::make_unique<WCHAR[]>(static_cast<int>(SESSION::BUFF_IP_32))), /*m_pIP(new WCHAR[static_cast<int>(SESSION::BUFF_IP_32)]),*/ m_wPort(0), m_spRecvQ(std::make_unique<CStreamQueue>()),m_spSendQ(std::make_unique<CQueue_LF<CPacket *>>()), /*m_pRecvQ(new CStreamQueue), m_pSendQ(new CQueue_LF<CPacket *>),*/ m_spIOInfo(static_cast<IO_INFO *>( _aligned_malloc( sizeof(IO_INFO), 16 )), &_aligned_free), /*m_pIOInfo(reinterpret_cast<IO_INFO *>(_aligned_malloc(sizeof(IO_INFO), 16))),*/ m_lSendIOCnt(0), m_lSendPQCSFlag(0), m_lPacketCnt(0), m_dwCurTlsIndex(0)
{
}

NOH::CSession::~CSession()
{
}

void NOH::CSession::Initialize(void)
{
    m_spIOInfo.get()->Initialize();
    wmemset(m_spIP.get(), 0, static_cast<size_t>(SESSION::BUFF_IP_32));
}

void NOH::CSession::Close(void)
{
    m_spIP.release();
    //delete m_pIP;
    //m_pIP = nullptr;

    m_spRecvQ.release();
    m_spSendQ.release();

    //delete m_pRecvQ;
    //m_pRecvQ = nullptr;
    //
    //delete m_pSendQ;
    //m_pSendQ = nullptr;

    //_aligned_free(m_pIOInfo);
    //m_pIOInfo = nullptr;
}

void NOH::CSession::SetAddress(const SOCKADDR_IN * lpRemoteSockAddr)
{
    InetNtopW(AF_INET, (PVOID)&lpRemoteSockAddr->sin_addr, m_spIP.get(), INET_ADDRSTRLEN);
    m_wPort = ntohs(lpRemoteSockAddr->sin_port);
}

void NOH::CSession::SetSessionInfo(SOCKET_INFO & CompletionKey, const SESSION_CODE SessionCode)
{
    if (m_spIOInfo.get()->llIOCnt < 0)
        CRASH();

    m_SessionCode = SessionCode;
    m_pCompletionKey = &CompletionKey;
    m_Socket = CompletionKey.GetSocket();
    m_lSendIOCnt = 0;
    m_lSendPQCSFlag = 0;

    memset(&m_SendOverlapped, 0, sizeof(OVERLAPPED));
    memset(&m_RecvOverlapped, 0, sizeof(OVERLAPPED));

    InterlockedIncrement64(&m_spIOInfo.get()->llIOCnt);
    InterlockedExchange64(&m_spIOInfo.get()->llReleaseFlag, static_cast<LONG64>(0));
}

void NOH::CSession::ClearCompletionKey(void)
{
    m_pCompletionKey = nullptr;
}

bool NOH::CSession::OnRecv(CPacket & RecvPacket)
{
    PACKET_PROTOCOL _packetprotocol = static_cast<PACKET_PROTOCOL>(PROTOCOL::DEFAULT);

    // 에코 테스트에는 필요 없음
    //*pRecvPacket >> &_packetprotocol;

    return PacketParsing(RecvPacket, static_cast<PROTOCOL>(_packetprotocol));
}

void NOH::CSession::OnSend(DWORD dwBytesTransfered)
{
}

void NOH::CSession::OnRelease(void)
{
    Release();
}

void NOH::CSession::OnClientJoin(PQCS_TYPE PQCSType)
{
    ClientJoin(PQCSType);
}
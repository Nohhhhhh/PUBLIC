#include "stdafx.h"
#include "User.h"

CLIENT::CUser::CUser()
{
}

CLIENT::CUser::~CUser()
{
}

bool CLIENT::CUser::SendPacket(NOH::CPacket & SendPacket, bool *bpSendPacket, const NOH::PQCS_TYPE PQCSType)
{
    if (1 == InterlockedIncrement64(&m_spIOInfo.get()->llIOCnt))
    {
        if (0 == InterlockedDecrement64(&m_spIOInfo.get()->llIOCnt))
            Release();

        bpSendPacket = false;
        return false;
    }

    SendPacket.SetHeader_SHORT((unsigned short)SendPacket.GetPayloadSize());

    m_spSendQ.get()->Enqueue(&SendPacket);

    bool bPQCSFlag = false;

    if (InterlockedCompareExchange(&m_lSendIOCnt, 1, 1) != 1)
    {
        if (InterlockedCompareExchange(&m_lSendPQCSFlag, 1, 0) == 0)
        {
            PostQueuedCompletionStatus(NOH::CIOCPHandler<CLIENT::CLanServer>::GetInstance()->GetIOCPHandle(), static_cast<DWORD>(PQCSType), reinterpret_cast<ULONG_PTR>(GetCompletionKey()), NULL);
            bPQCSFlag = true;
        }
    }

    if (!bPQCSFlag)
    {
        if (0 == InterlockedDecrement64(&m_spIOInfo.get()->llIOCnt))
        {
            Release();
            bpSendPacket = false;
        }
    }

    return true;
}

void CLIENT::CUser::Release(void)
{
    NOH::IO_INFO _tempioinfo;
    _tempioinfo.llIOCnt = 0;
    _tempioinfo.llReleaseFlag = 0;

    if (0 == InterlockedCompareExchange128(reinterpret_cast<volatile LONG64 *>(m_spIOInfo.get()), static_cast<LONG64>(1), static_cast<LONG64>(0), reinterpret_cast<LONG64 *>(&_tempioinfo)))
        return;

    NOH::CSocketManager<CUser>::GetInstance()->DisconnectSocket(*GetCompletionKey(), NOH::DISCONNECT_TYPE::REUSESOCKET);
    m_Socket = INVALID_SOCKET;

    NOH::CPacket *_psendpacket = nullptr;
    NOH::CQueue_LF<NOH::CPacket *> *_pstoredsendqueue = m_spSendQ.get();
    while (1)
    {
        _psendpacket = nullptr;

        if (!_pstoredsendqueue->Dequeue(&_psendpacket))
            break;

        NOH::CPacket::Free(*_psendpacket);
    }

    m_spRecvQ.get()->ClearBuffer();

    if (0 != _pstoredsendqueue->m_lNodeCnt)
        CRASH();

    _pstoredsendqueue->ClearQueue();


    NOH::CSessionManager<CUser> *_psessionmanager = NOH::CSessionManager<CUser>::GetInstance();
    NOH::SESSION_CODE tempSessionCode = m_SessionCode;
    m_SessionCode = 0;
    _psessionmanager->Put(GET_ARRAYIDX(tempSessionCode));

    InterlockedDecrement(&g_lCurrentUserCount);
}

bool CLIENT::CUser::PacketParsing(NOH::CPacket & RecvPacket, const NOH::PROTOCOL Protocol)
{
    switch (Protocol)
    {
    case NOH::PROTOCOL::DEFAULT:
        return Echo(RecvPacket);
    }

    return false;
}

bool CLIENT::CUser::ClientJoin(const NOH::PQCS_TYPE PQCSType)
{
    bool _bsendpacket = true;

    NOH::CPacket & _sendpacket = *NOH::CPacket::Alloc();

    _sendpacket << (__int64)0x7fffffffffffffff;

    _sendpacket.AddRef();

    if (!SendPacket(_sendpacket, &_bsendpacket, PQCSType))
        NOH::CPacket::Free(_sendpacket);

    NOH::CPacket::Free(_sendpacket);

    return _bsendpacket;
}

bool CLIENT::CUser::Echo(NOH::CPacket & RecvPacket)
{
    bool _bsendpacket = true;

    __int64 _ivalue = 0;
    RecvPacket >> &_ivalue;

    NOH::CPacket & _sendpacket = *NOH::CPacket::Alloc();

    _sendpacket << _ivalue;

    _sendpacket.AddRef();

    if (!SendPacket(_sendpacket, &_bsendpacket, NOH::PQCS_TYPE::SENDPOST))
        NOH::CPacket::Free(_sendpacket);

    NOH::CPacket::Free(_sendpacket);

    return _bsendpacket;
}

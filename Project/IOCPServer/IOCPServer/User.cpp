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
    // lIOCnt++이 1이라는 의미는 의미는 Release를 해야 하는 상황이라는 의미이다.
    // 따라서 lIOCnt--를 해서 0인지 확인 후, 0이라면 Release를 하면 된다.
    // lIoCnt--가 0이 아니라면, 어디선가 Release로 들어 갔다는 의미이므로
    // 어쨌든 해당 세션에 대한 로직 처리는 중단 시켜야 한다.
    if (1 == InterlockedIncrement64(&m_spIOInfo.get()->llIOCnt))
    {
        if (0 == InterlockedDecrement64(&m_spIOInfo.get()->llIOCnt))
            Release();

        // 여기서는 아직 SendQ에 패킷이 들어 가지 않았으므로, return false로 반화시켜서
        // 추가적으로 Free()를 하도록 해야 함.
        bpSendPacket = false;
        return false;
    }

    SendPacket.SetHeader_SHORT((unsigned short)SendPacket.GetPayloadSize());

    // 체크 1
    // sendq가 충분히 크지 못한 경우 recv 들어온 패킷이 send 하는 패킷보다 많아 지면 문제가 발생
    // 여기서는 데이터가 무조건 들어가 야 함.
    // 이때 8byte가 아닌 값이 들어가면 usingsize는 0 초과 8미만이 나오게 되는대, 이러면 sendpost에서(체크 4) 무한 루프로 빠진다.
    // 이 부분에서 enqueue 반환 값이 8byte 아니면, while문 돌면서 sendq가 8byte 이상 비워 질 때 까지 대기를 타야 할 것 같다.
    // 그게 아니라면, 무조건 세션 1명이 보낼 수 있는 최대 패킷 크기보다 크게 주어야 한다.
    //
    // A. => 버퍼 크기가 충분하지 않아서 iRetval이 포인터 주소값인 8바이트 미만으로 나오는 경우가 발생한다면, 게임 컨텐츠에 따라서 적당한 수준의 버퍼 크기를 확보해야 한다.
    //		 만약에 그래도 sendq가 비워지지 않으면 해당 세션가 안받고 있다고 봐야하기 때문에 접속을 끊어야 한다.

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
        // 여기서는 이미 SendQ에 패킷이 들어 갔으므로, Release를 타더라도 return true로 반환시켜서
        // 추가적으로 Free()를 하지 않도록 해야 함.
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

    // Release 대상인지 확인
    if (0 == InterlockedCompareExchange128(reinterpret_cast<volatile LONG64 *>(m_spIOInfo.get()), static_cast<LONG64>(1), static_cast<LONG64>(0), reinterpret_cast<LONG64 *>(&_tempioinfo)))
        return;

    NOH::CSocketManager<CUser>::GetInstance()->DisconnectSocket(*GetCompletionKey(), NOH::DISCONNECT_TYPE::REUSESOCKET);
    m_Socket = INVALID_SOCKET;

    NOH::CPacket *_psendpacket = nullptr;
    NOH::CQueue_LF<NOH::CPacket *> *_pstoredsendqueue = m_spSendQ.get();
    while (1)
    {
        _psendpacket = nullptr;

        // lockfree
        if (!_pstoredsendqueue->Dequeue(&_psendpacket))
            break;

        NOH::CPacket::Free(*_psendpacket);
        // 또는 pSEndPacket->Free();
    }

    m_spRecvQ.get()->ClearBuffer();
    // lockfree
    if (0 != _pstoredsendqueue->m_lNodeCnt)
        CRASH();
    _pstoredsendqueue->ClearQueue();
    // none lockfree
    //pSessionInfo->SendQ.ClearBuffer();

    NOH::CSessionManager<CUser> *_psessionmanager = NOH::CSessionManager<CUser>::GetInstance();
    NOH::SESSION_CODE tempSessionCode = m_SessionCode;
    m_SessionCode = 0;
    _psessionmanager->Put(GET_ARRAYIDX(tempSessionCode));

    // 현재 접속 인원 1 감소
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

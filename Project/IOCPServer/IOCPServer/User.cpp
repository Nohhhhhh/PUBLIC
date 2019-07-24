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
    // lIOCnt++�� 1�̶�� �ǹ̴� �ǹ̴� Release�� �ؾ� �ϴ� ��Ȳ�̶�� �ǹ��̴�.
    // ���� lIOCnt--�� �ؼ� 0���� Ȯ�� ��, 0�̶�� Release�� �ϸ� �ȴ�.
    // lIoCnt--�� 0�� �ƴ϶��, ��𼱰� Release�� ��� ���ٴ� �ǹ��̹Ƿ�
    // ��·�� �ش� ���ǿ� ���� ���� ó���� �ߴ� ���Ѿ� �Ѵ�.
    if (1 == InterlockedIncrement64(&m_spIOInfo.get()->llIOCnt))
    {
        if (0 == InterlockedDecrement64(&m_spIOInfo.get()->llIOCnt))
            Release();

        // ���⼭�� ���� SendQ�� ��Ŷ�� ��� ���� �ʾ����Ƿ�, return false�� ��ȭ���Ѽ�
        // �߰������� Free()�� �ϵ��� �ؾ� ��.
        bpSendPacket = false;
        return false;
    }

    SendPacket.SetHeader_SHORT((unsigned short)SendPacket.GetPayloadSize());

    // üũ 1
    // sendq�� ����� ũ�� ���� ��� recv ���� ��Ŷ�� send �ϴ� ��Ŷ���� ���� ���� ������ �߻�
    // ���⼭�� �����Ͱ� ������ �� �� ��.
    // �̶� 8byte�� �ƴ� ���� ���� usingsize�� 0 �ʰ� 8�̸��� ������ �Ǵ´�, �̷��� sendpost����(üũ 4) ���� ������ ������.
    // �� �κп��� enqueue ��ȯ ���� 8byte �ƴϸ�, while�� ���鼭 sendq�� 8byte �̻� ��� �� �� ���� ��⸦ Ÿ�� �� �� ����.
    // �װ� �ƴ϶��, ������ ���� 1���� ���� �� �ִ� �ִ� ��Ŷ ũ�⺸�� ũ�� �־�� �Ѵ�.
    //
    // A. => ���� ũ�Ⱑ ������� �ʾƼ� iRetval�� ������ �ּҰ��� 8����Ʈ �̸����� ������ ��찡 �߻��Ѵٸ�, ���� �������� ���� ������ ������ ���� ũ�⸦ Ȯ���ؾ� �Ѵ�.
    //		 ���࿡ �׷��� sendq�� ������� ������ �ش� ���ǰ� �ȹް� �ִٰ� �����ϱ� ������ ������ ����� �Ѵ�.

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
        // ���⼭�� �̹� SendQ�� ��Ŷ�� ��� �����Ƿ�, Release�� Ÿ���� return true�� ��ȯ���Ѽ�
        // �߰������� Free()�� ���� �ʵ��� �ؾ� ��.
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

    // Release ������� Ȯ��
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
        // �Ǵ� pSEndPacket->Free();
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

    // ���� ���� �ο� 1 ����
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

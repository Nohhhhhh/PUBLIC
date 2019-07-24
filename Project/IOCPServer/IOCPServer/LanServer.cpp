#include "stdafx.h"
#include "LanServer.h"
#include "Session.h"
#include "Queue_LF.h"
#include "Packet.h"
#include "StreamQueue.h"
#include "SessionManager.h"
#include "IOCPHandler.h"
#include "SocketManager.h"
#include "Profiler.h"
#include "User.h"


bool CLIENT::CLanServer::CompleteRecv(NOH::CSession & Session, DWORD & dwBytesTransfered)
{
    // 완료통지 왔으니까 rear pos 이동
    if (-1 == Session.GetRecvQueue()->MoveRear(dwBytesTransfered))
    {
        // MoveRear에 최대 패킷 사이즈보다 더 큰 사이즈가 들어 갔을 때.
        DisconnectSocket(Session, NOH::DISCONNECT_TYPE::GRACEFUL);
        return false;
    }

    int _iret = 0;
    int _irecvqueueusingsize = 0;
    LONG _lrecvcnt = 0;
    LAN_HEADER _header;

    while (1)
    {
        NOH::CStreamQueue & _recvqueue = *Session.GetRecvQueue();
        _irecvqueueusingsize = _recvqueue.GetUsingSize();

        if (dwBytesTransfered <= 0 || _irecvqueueusingsize <= 0)
            break;

        memset(&_header, 0, sizeof(LAN_HEADER));

        // header 사이즈 보다 작은지 확인
        if (_irecvqueueusingsize < sizeof(LAN_HEADER))
            break;

        // header 뽑기
        _iret = _recvqueue.Peek(reinterpret_cast<char *>(&_header), sizeof(LAN_HEADER));

        dwBytesTransfered -= _iret;

        // payload + header 사이즈 보다 작은지 확인
        if (_irecvqueueusingsize < _header.wPacketSize + sizeof(LAN_HEADER))
            break;

        // header 뽑은 만큼 이동 후, payload 뽑기
        if (-1 == _recvqueue.RemoveData(_iret))
        {
            // 최대 패킷 사이즈보다 더 큰 사이즈가 들어 갔을 때.
            DisconnectSocket(Session, NOH::DISCONNECT_TYPE::GRACEFUL);
            return false;
        }

        NOH::CPacket & _recvpacket = *NOH::CPacket::Alloc();

        _iret = _recvqueue.Dequeue(reinterpret_cast<char *>(_recvpacket.GetPayloadPtr()), _header.wPacketSize);

        // 최대 패킷 사이즈보다 더 큰 사이즈가 들어 갔을 때.
        if (-1 == _iret)
        {
            DisconnectSocket(Session, NOH::DISCONNECT_TYPE::GRACEFUL);
            NOH::CPacket::Free(_recvpacket);
            return false;
        }

        dwBytesTransfered -= _iret;

        // 패킷 위치 이동
        _recvpacket.MoveRear(_iret);

        try
        {
            // 사용자 함수
            if (!Session.OnRecv(_recvpacket))
            {
                NOH::CPacket::Free(_recvpacket);
                return false;
            }
        }
        catch (NOH::exception_PacketOut &e)
        {
            WCHAR szError[200] = { 0 };
            swprintf_s(szError, L"CompleteRecv[Dequeue Error] IP: %s / Port: %d / Socket: %I64u / ErrorSize: %d", Session.GetSessionIP(), Session.GetSessionPort(), static_cast<unsigned __int64>(Session.GetSocket()), e._RequestOutSize);
            Error(szError);
            DisconnectSocket(Session, NOH::DISCONNECT_TYPE::GRACEFUL);
            NOH::CPacket::Free(_recvpacket);
            return false;
        }
        catch (NOH::exception_PacketIn &e)
        {
            WCHAR szError[200] = { 0 };
            swprintf_s(szError, L"CompleteRecv[Enqueue Error]  IP: %s / Port: %d / Socket: %I64u / ErrorSize: %d", Session.GetSessionIP(), Session.GetSessionPort(), static_cast<unsigned __int64>(Session.GetSocket()), e._RequestInSize);
            Error(szError);
            DisconnectSocket(Session, NOH::DISCONNECT_TYPE::GRACEFUL);
            NOH::CPacket::Free(_recvpacket);
            return false;
        }

        // pRecvPacket 메모리 풀 사용 끝
        NOH::CPacket::Free(_recvpacket);
        ++_lrecvcnt;
    }

    InterlockedAdd(&g_lRecvPacketTPS, _lrecvcnt);

    // WSARecv
    RecvPost(Session, false);

    return true;
}

bool CLIENT::CLanServer::CompleteSend(NOH::CSession & Session, DWORD & dwBytesTransfered)
{
    NOH::CPacket *_ppacket = nullptr;
    bool _bdequeue = false;
    int _ifreetotalsize = 0;
    long _lpacketcnt = 0;

    NOH::CQueue_LF<NOH::CPacket *> *_psendqueue = Session.GetSendQueue();

    // 할당된 메모리 풀 반환
    while(_ifreetotalsize != dwBytesTransfered )
    {
        _ppacket = nullptr;

        // lock free
        if (!_psendqueue->Dequeue(&_ppacket))
            break;

        _ifreetotalsize += _ppacket->GetPacketSize();

        // 메모리 풀 사용 끝
        NOH::CPacket::Free(*_ppacket);

        ++_lpacketcnt;
    }

    // none lockfree
    //pSessionInfo->SendQ.Lock();
    //// 완료통지 왔으니까, 보낸 데이터 지우기
    //pSessionInfo->SendQ.RemoveData(iTotalSize);
    //pSessionInfo->SendQ.UnLock();

    InterlockedAdd(&g_lSendPacketTPS, _lpacketcnt);

    // WSASend 할 때 lSendCount를 1 증가 시켰으니까 1 감소 시킴
    InterlockedExchange(Session.GetSendPQCSFlag(), 0);
    InterlockedExchange(Session.GetSendIOCnt(), 0);

    // lockfree
    if (0 < _psendqueue->GetNodeCount())
        // none lockfree
        //if (0 < pSessionInfo->SendQ.GetUsingSize())
    {
        SendPost(Session);
    }

    // 사용자 함수
    Session.OnSend(dwBytesTransfered);

    return true;
}

bool CLIENT::CLanServer::RecvPost(NOH::CSession & Session, const bool & bAccept)
{
    DWORD _flags = 0;

    WSABUF _recvwsabuf[2];
    memset(_recvwsabuf, 0, sizeof(_recvwsabuf));

    int _irecvbufferfreesize = Session.GetRecvQueue()->GetFreeSize();
    int _irecvbuffernotcirculatedfreesize = Session.GetRecvQueue()->GetNotCirculatedFreeSize();

    // 쓸 위치, 순회하지 않고 쓸수 있는 사이즈
    _recvwsabuf[0].buf = Session.GetRecvQueue()->GetRearBufferPtr();
    _recvwsabuf[0].len = _irecvbuffernotcirculatedfreesize;

    // 시작 위치, 시작위치부터 쓸 수 있는 사이즈
    _recvwsabuf[1].buf = Session.GetRecvQueue()->GetBufferPtr();
    _recvwsabuf[1].len = _irecvbufferfreesize - _irecvbuffernotcirculatedfreesize;

    memset(Session.GetRecvOverlapped(), 0, sizeof(OVERLAPPED));

    // lIOCnt 1 증가
    if (!bAccept)
        InterlockedIncrement64(&Session.GetIOInfo()->llIOCnt);

    if (SOCKET_ERROR == WSARecv(Session.GetSocket(), _recvwsabuf, 2, nullptr, &_flags, Session.GetRecvOverlapped(), nullptr))
    {
        int _ierrorcode = WSAGetLastError();

        // WSA_IO_PENDING는 Overlapped 연산이 즉시 완료되지 않았을 경우발생. 나중에 완료될 것임. 
        if (_ierrorcode != WSA_IO_PENDING)
        {
            //if (iErrorCode != 10054 && iErrorCode != 10053 && iErrorCode != 10058)
            if (WSAENOTCONN != _ierrorcode && WSAECONNRESET != _ierrorcode && WSAECONNABORTED != _ierrorcode)
            {
                WCHAR _errormessage[200] = { 0 };
                swprintf_s(_errormessage, L"WSARecv[WSARecv ERROR] - Error Code: %d, ThreadIdx: %lu, Socket: %I64u, IP: %s, Port: %d", _ierrorcode, GetThreadIdx(), static_cast<unsigned __int64>(Session.GetSocket()), Session.GetSessionIP(), Session.GetSessionPort());
                Error(_errormessage);
            }

            // lIOCnt 1 감소 한 것이 0이면, 완전히 release
            // 0이 아니면, sutdown sd_send
            if (0 == InterlockedDecrement64(&Session.GetIOInfo()->llIOCnt))
                Session.OnRelease();
            else 
                DisconnectSocket(Session, NOH::DISCONNECT_TYPE::GRACEFUL);
        }
    }

    return true;
}

bool CLIENT::CLanServer::SendPost(NOH::CSession & Session)
{
RETRY:
    if (0 != InterlockedCompareExchange(Session.GetSendIOCnt(), 1, 0))
        return false;

    WSABUF _sendwasbuf[dfLAN_WSABUF_MAX_SIZE];
    memset(_sendwasbuf, 0, sizeof(_sendwasbuf));

    int _inodecnt = 0;

    // none lf
    //int iTotalSize = 0;

    NOH::CPacket *_ppacket = nullptr;
    long _lpacketsize = 0;

    while (1)
    {
        // 체크 4 (체크 1 참조)
        // front 와 rear 의 위치는 동일하거나 항상 8차이가 나야 하는대,
        // 0 초과 8미만인 경우가 발생. 이렇게 되면 여기서는 항상 반환 값이 0이 되지만, 
        // 실제 usingsize는 0보다 크기 때문에 goto문을 거쳐서 해당 로직에서 무한 루프에 빠진다.

        // lock free
        // false면 유효한 데이터가 없다는 의미
        if (!Session.GetSendQueue()->Peek(&_ppacket, _inodecnt))
            break;

        // none lockfree
        //int iRet = pSessionInfo->SendQ.Peek((char *)&pPacket, sizeof(CPacket *), iTotalSize);
        //
        //if (0 == iRet)
        //	break;

        _sendwasbuf[_inodecnt].buf = reinterpret_cast<char *>(_ppacket->GetHeaderBufferPtr());
        _sendwasbuf[_inodecnt].len = _ppacket->GetPacketSize();

        _lpacketsize += _sendwasbuf[_inodecnt].len;
        ++_inodecnt;

        // none lockfree
        //iTotalSize += sizeof(CPacket *);

        // WSABUF 크기보다 많아지면 안되니까 break
        if (dfLAN_WSABUF_MAX_SIZE == _inodecnt)
            break;
    }

    if (0 == _inodecnt)
    {
        InterlockedExchange(Session.GetSendPQCSFlag(), 0);
        InterlockedExchange(Session.GetSendIOCnt(), 0);

        // lockfree
        if (0 < Session.GetSendQueue()->GetNodeCount())
            goto RETRY;

        // none lockfree
        //if (0 < pSessionInfo->SendQ.GetUsingSize())
        //	goto RETRY;

        return 	false;
    }

    // RecvOverlapped 초기화
    memset(Session.GetSendOverlapped(), 0, sizeof(OVERLAPPED));

    // lIOCnt 1 증가
    InterlockedIncrement64(&Session.GetIOInfo()->llIOCnt);

    if (SOCKET_ERROR == WSASend(Session.GetCompletionKey()->GetSocket(), _sendwasbuf, _inodecnt, nullptr, 0, Session.GetSendOverlapped(), nullptr))
    {
        int _ierrorcode = WSAGetLastError();
        // WSA_IO_PENDING는 Overlapped 연산이 즉시 완료되지 않았을 경우발생. 나중에 완료될 것임. 
        if (_ierrorcode != WSA_IO_PENDING)
        {
            if (WSAENOTCONN != _ierrorcode && WSAECONNRESET != _ierrorcode && WSAECONNABORTED != _ierrorcode)
            {
                WCHAR _errormessage[200] = { 0 };
                swprintf_s(_errormessage, L"WSASend[WSASend ERROR] - Error Code: %d, ThreadIdx: %lu, Socket: %I64u, IP: %s, Port: %d", _ierrorcode, GetThreadIdx(), static_cast<unsigned __int64>(Session.GetSocket()), Session.GetSessionIP(), Session.GetSessionPort());
                
                Error(_errormessage);
            }

            // 위에서 lSendCount를 1로 변경 시켰으나, WSASend 오류가 발생했으므로 다시 0
            InterlockedExchange(Session.GetSendPQCSFlag(), 0);
            InterlockedExchange(Session.GetSendIOCnt(), 0);

            // lIOCnt 1 감소 한 것이 0이면, 완전히 release
            // 0이 아니면, sutdown sd_send
            if (0 == InterlockedDecrement64(&Session.GetIOInfo()->llIOCnt))
                Session.OnRelease();
            else
                DisconnectSocket(Session, NOH::DISCONNECT_TYPE::GRACEFUL);

            return false;
        }
    }

    return true;
}

void CLIENT::CLanServer::Error(WCHAR * szError)
{
    SYSLOG(g_LogGameServer, NOH::LOG_LEVEL::LEVEL_SYSTEM, L"%s", szError);
}

bool CLIENT::CLanServer::PQCSTypeParsing(NOH::SOCKET_INFO & SocketInfo, const NOH::PQCS_TYPE PQCSType)
{
    switch (PQCSType)
    {
    case NOH::PQCS_TYPE::CLOSE_THREAD:
    {
        if (nullptr == &SocketInfo)
            return false;
    }
    case NOH::PQCS_TYPE::RECVPOST:
    {
        RecvPost(SocketInfo.GetSession(), false);

        break;
    }
    case NOH::PQCS_TYPE::SENDPOST:
    {
        SendPost(SocketInfo.GetSession());
        
        if (0 == InterlockedDecrement64(&SocketInfo.GetSession().GetIOInfo()->llIOCnt))
            SocketInfo.GetSession().OnRelease();

        break;
    }
    case NOH::PQCS_TYPE::JOIN:
    {
        RecvPost(SocketInfo.GetSession(), true);
        SendPost(SocketInfo.GetSession());
        
        if (0 == InterlockedDecrement64(&SocketInfo.GetSession().GetIOInfo()->llIOCnt))
            SocketInfo.GetSession().OnRelease();

        break;
    }
    default:
        CRASH();
    }

    return true;
}

void CLIENT::CLanServer::DisconnectSocket(NOH::CSession & Session, const NOH::DISCONNECT_TYPE Type)
{
    NOH::CSocketManager<CUser>::GetInstance()->DisconnectSocket(*Session.GetCompletionKey(), Type);
}

void CLIENT::CLanServer::ReUseSocket(NOH::SOCKET_INFO & SocketInfo)
{
    NOH::CSocketManager<CUser>::GetInstance()->AcceptEx(SocketInfo);
}

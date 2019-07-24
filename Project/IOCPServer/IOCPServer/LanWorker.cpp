#include "Dump.h"
#include "LanWorker.h"
#include "Session.h"
#include "Queue_LF.h"
#include "Packet.h"
#include "StreamQueue.h"
#include "SessionManager.h"
#include "IOCPHandler.h"
#include "SocketManager.h"
#include "Profiler.h"
#include "Global.h"

NOH::CLanWorker::CLanWorker()
    : m_hIOCP(INVALID_HANDLE_VALUE)
{
}

NOH::CLanWorker::~CLanWorker()
{
}

void NOH::CLanWorker::Run()
{
    m_dwProfileTlsIndex = TlsAlloc();
    DWORD _dwthreadidx = NOH::CProfiler::GetInstance()->GetThreadIdx(GetThreadType()) - 1;
    TlsSetValue(m_dwProfileTlsIndex, &_dwthreadidx);

    DWORD			_dwbytestransfered = 0;
    ULONG_PTR		_ulpcompletionkey = 0;
    LPOVERLAPPED    _lpoverlapped = nullptr;
 
    // 대기
    while (!m_bStart)
        Sleep(100);

    while (m_bStart)
    {
        _dwbytestransfered = 0;
        _ulpcompletionkey = 0;
        _lpoverlapped = nullptr;

        BOOL _bret = GetQueuedCompletionStatus(m_hIOCP, &_dwbytestransfered, &_ulpcompletionkey, &_lpoverlapped, INFINITE);

        SOCKET_INFO & _socketinfo = *reinterpret_cast<SOCKET_INFO *>(_ulpcompletionkey);

        if (TRUE == _bret && nullptr == _lpoverlapped)
        {
            if (PQCSTypeParsing(_socketinfo, static_cast<const PQCS_TYPE>(_dwbytestransfered)))
                continue;
            else
                break;
        }

        if (TRUE == _bret && 0 == _dwbytestransfered && 0 != _ulpcompletionkey)
        {
            ReUseSocket(_socketinfo);
            continue;
        }

        CSession & _session = *_socketinfo.pSession;

        // 종료 처리
        if (FALSE == _bret && 0 == _dwbytestransfered)
        {
            if (&_session.m_SendOverlapped == _lpoverlapped)
                InterlockedExchange(&_session.m_lSendIOCnt, 0);

            DisconnectSocket(_session, DISCONNECT_TYPE::GRACEFUL);
        }
        else
        {
            if (&_session.m_RecvOverlapped == _lpoverlapped)
            {
                //PROFILE_BEGIN(*L"CompleteRecv", GetThreadType(), GetThreadIdx());
                CompleteRecv(_session, _dwbytestransfered);
                //PROFILE_END(*L"CompleteRecv", GetThreadType(), GetThreadIdx());
            }
            else
            {
                //PROFILE_BEGIN(*L"CompleteSend", GetThreadType(), GetThreadIdx());
                CompleteSend(_session, _dwbytestransfered);
                //PROFILE_END(*L"CompleteSend", GetThreadType(), GetThreadIdx());
            }
        }

        if (0 == InterlockedDecrement64(&_session.m_spIOInfo.get()->llIOCnt))
            _session.OnRelease();
    }
}

void NOH::CLanWorker::Close()
{
    TlsFree(m_dwProfileTlsIndex);
    End();
}

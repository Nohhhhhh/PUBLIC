#include "Dump.h"
#include "AcceptWorker.h"
#include "SocketManager.h"
#include "IOCPHandler.h"
#include "SessionManager.h"
#include "Profiler.h"
#include "Global.h"
#include "User.h"
#include "Packet.h"

NOH::CAcceptWorker::CAcceptWorker()
    : m_hIOCP(INVALID_HANDLE_VALUE), m_dwThreadIdx(0)
{
}

NOH::CAcceptWorker::~CAcceptWorker()
{
}

void NOH::CAcceptWorker::Run(void)
{
    m_dwProfileTlsIndex = TlsAlloc();
    DWORD _dwthreadidx = NOH::CProfiler::GetInstance()->GetThreadIdx(GetThreadType()) - 1;
    TlsSetValue(m_dwProfileTlsIndex, &_dwthreadidx);

    BOOL			    _bret = 0;
    DWORD			    _dwbytestransfered = 0;
    ULONG_PTR		    _ulpcompletionkey = 0;
    LPOVERLAPPED        _lpoverlapped = nullptr;
    int                 _isessionidx = 0;

    // ´ë±â
    while (!m_bStart)
        Sleep(100);

    if (!CreateAcceptExSockAddr())
    {
        WCHAR _errormessage[200] = { 0 };
        swprintf_s(_errormessage, L"CAcceptWorker[CreateAcceptExSockAddr Error]  ErrorCode: %d", WSAGetLastError());
        Error(*_errormessage);
    }
    bool btest = false;
    int icount = 0;
    while (m_bStart)
    {
        _dwbytestransfered = 0;
        _ulpcompletionkey = 0;
        _lpoverlapped = nullptr;

        if (!GetQueuedCompletionStatus(m_hIOCP, &_dwbytestransfered, &_ulpcompletionkey, &_lpoverlapped, INFINITE))
        {
            WCHAR _errormessage[200] = { 0 };
            int _ierrorcode = WSAGetLastError();

            if (ERROR_NETNAME_DELETED != _ierrorcode)
                swprintf_s(_errormessage, L"CAcceptWorker[GQCS Error] - ErrorCode: %d, Socket: %lld", _ierrorcode, reinterpret_cast<SOCKET_INFO *>(_lpoverlapped)->Socket);

            Error(*_errormessage, reinterpret_cast<SOCKET_INFO &>(*_lpoverlapped));

            InterlockedIncrement(&g_lAcceptTPS);
            InterlockedDecrement(&g_lRemainingSocketCount);
            continue;
        }
        else

        if (static_cast<DWORD>(PQCS_TYPE::CLOSE_THREAD) == _dwbytestransfered && nullptr == _lpoverlapped && 0 == _ulpcompletionkey)
            break;

        if (nullptr == reinterpret_cast<SOCKET_INFO *>(_lpoverlapped))
        {
            WCHAR _errormessage[200] = { 0 };
            swprintf_s(_errormessage, L"CAcceptWorker[SocketInfo is nullptr]");
            Error(*_errormessage);
            continue;
        }

        SOCKET_INFO & _socketinfo = reinterpret_cast<SOCKET_INFO &>(*_lpoverlapped);

        _isessionidx = -1;
        CSession & _session = *GetSession(_socketinfo.SocketType, &_isessionidx);

        if (nullptr == &_session)
        {
            WCHAR _errormessage[200] = { 0 };
            swprintf_s(_errormessage, L"CAcceptWorker[_psession is nullptr] - SessionIdx: %d", _isessionidx);
            Error(*_errormessage, _socketinfo);
            continue;
        }

        if (!AcceptExSockAddr(_socketinfo, _session))
        {
            WCHAR _errormessage[200] = { 0 };
            swprintf_s(_errormessage, L"CAcceptWorker[setsockopt Error]  ErrorCode: %d Socket: %lld", WSAGetLastError(), _socketinfo.Socket);
            Error(*_errormessage, _socketinfo, _isessionidx);
            continue;
        }

        CheckSessionBan(_session);

        if (_socketinfo.bFirst)
        {       
            if (!AssociateAcceptSocket(_socketinfo))
            {
                WCHAR _errormessage[200] = { 0 };
                swprintf_s(_errormessage, L"CAcceptWorker[IOCP Error]  ErrorCode: %d Socket: %lld", GetLastError(), _socketinfo.Socket);
                Error(*_errormessage, _socketinfo, _isessionidx);
                continue;
            }

            _socketinfo.bFirst = false;
        }
        
        if (!InitSessionInfo(_socketinfo, _session, _isessionidx))
        {
            WCHAR _errormessage[200] = { 0 };
            swprintf_s(_errormessage, L"CAcceptWorker[Session Init Error]  ErrorCode: %d Socket: %lld", GetLastError(), _socketinfo.Socket);
            Error(*_errormessage, _socketinfo, _isessionidx);
            continue;
        }
    }
}

void NOH::CAcceptWorker::Close(void)
{
    End();
}
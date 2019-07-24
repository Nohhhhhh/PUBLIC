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

    // 대기
    while (!m_bStart)
        Sleep(100);

    // AcceptExSockAddr 할당하기
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

        //PROFILE_END(*L"AcceptEx", GetThreadType(), GetThreadIdx());
        // 1. accept()
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
        {
            //InterlockedIncrement(&g_lAcceptTPS); 
            //InterlockedDecrement(&g_lRemainingSocketCount);
        }
        //PROFILE_BEGIN(*L"AcceptEx", GetThreadType(), GetThreadIdx());

        // 서버 종료 처리
        if (static_cast<DWORD>(PQCS_TYPE::CLOSE_THREAD) == _dwbytestransfered && nullptr == _lpoverlapped && 0 == _ulpcompletionkey)
            break;

        // socketinfo 확인
        if (nullptr == reinterpret_cast<SOCKET_INFO *>(_lpoverlapped))
        {
            WCHAR _errormessage[200] = { 0 };
            swprintf_s(_errormessage, L"CAcceptWorker[SocketInfo is nullptr]");
            Error(*_errormessage);
            continue;
        }

        SOCKET_INFO & _socketinfo = reinterpret_cast<SOCKET_INFO &>(*_lpoverlapped);

        // 2. Session 뽑기
        _isessionidx = -1;
        CSession & _session = *GetSession(_socketinfo.SocketType, &_isessionidx);

        if (nullptr == &_session)
        {
            WCHAR _errormessage[200] = { 0 };
            swprintf_s(_errormessage, L"CAcceptWorker[_psession is nullptr] - SessionIdx: %d", _isessionidx);
            Error(*_errormessage, _socketinfo);
            continue;
        }

        // 3. AcceptEx의 SockAddr 얻기 
        if (!AcceptExSockAddr(_socketinfo, _session))
        {
            WCHAR _errormessage[200] = { 0 };
            swprintf_s(_errormessage, L"CAcceptWorker[setsockopt Error]  ErrorCode: %d Socket: %lld", WSAGetLastError(), _socketinfo.Socket);
            Error(*_errormessage, _socketinfo, _isessionidx);
            continue;
        }

        // 5. IP Ban 확인 
        CheckSessionBan(_session);

        // 6. IOCP 등록
        // AcceptEx의 경우 IOCP 등록은 최초에 한 번만 하면 된다. closesocket을 통해 소켓이 완전히 닫히지 않는 이상
        // IOCP를 세션 등록 보다 먼저 하는게 좋은 이유
        // 크게 상관은 없지만 이전에 세션을 초기화하고 IOCP를 등록하다 보니, 컨텐츠 단에 접속도 하기 전에
        // 세션 릴리즈가 발생하는 상황이 벌어 졌다. 물론 예외처리로 처리 할 수 있지만, 
        // 기본적으로 IOCP가 NULL이 됬다는 것은 소켓이 깨졌다는 의미이므로, 세션을 등록할 필요성 조차 없다.
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
        

        // 8. 세션 ID 할당 및 초기화 후, PQCS 호출
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
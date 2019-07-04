#include "Profiler.h"
#include "Dump.h"
#include "FileStream.h"
#include "TimerManager.h"

#include <Strsafe.h>
#include <ShlObj.h>
#include <chrono>

NOH::CProfiler::CProfiler()
    : m_bEmptyData(true), m_spFileStream(std::make_unique<CFileStream>()), /*m_spProfile(std::make_unique<PROFILE_THREAD_TYPE[]>(static_cast<int>(PROFILE::THREAD_TYPE_COUNT))),*/ m_spDirectory(std::make_unique<WCHAR[]>(static_cast<int>(COMMON::BUF_128))), m_lFrequency({ 0 }), m_dMicroFrequency(0), m_FileIdx(0), m_dwThreadIdx{ 0 }
{
}

NOH::CProfiler::~CProfiler()
{
}

void NOH::CProfiler::Initialize(const WCHAR &wDirectory)
{
    wmemset(m_spDirectory.get(), 0, static_cast<size_t>(COMMON::BUF_128));
    swprintf_s(m_spDirectory.get(), static_cast<size_t>(COMMON::BUF_128), L"%s", &wDirectory);
    SHCreateDirectoryExW(nullptr, reinterpret_cast<LPCWSTR>(m_spDirectory.get()), nullptr);

    memset(&m_lFrequency, 0, sizeof(LARGE_INTEGER));
    QueryPerformanceFrequency(&m_lFrequency);
    // 나노 세컨드: 10억분의 1초. 마이크로 세컨드: 100만분의 1초. 밀리 세컨드. 1천분의 1초
    m_dMicroFrequency = static_cast<double>(m_lFrequency.QuadPart / 1000000.0f);
    //std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

    for (int iThreadIdx = 0; iThreadIdx < static_cast<int>(THREAD_TYPE::THREAD_TYPE_TOTAL); ++iThreadIdx)
        m_mProfile.insert(std::make_pair(static_cast<THREAD_TYPE>(iThreadIdx), std::move(std::make_unique<PROFILE_THREAD>())));
}

void NOH::CProfiler::Begin(const WCHAR &wName, const NOH::THREAD_TYPE &ThreadType, const int &iThreadIdx)
{
    PROFILE_DATA *_pdata = CheckData(wName, ThreadType, iThreadIdx);

    if (nullptr == _pdata)
        return;

    // Begin을 이미 한 상태인지 확인
    if (0 != _pdata->lStartTime.QuadPart)
        return;

    QueryPerformanceCounter(&_pdata->lStartTime);
}

void NOH::CProfiler::End(const WCHAR &wName, const NOH::THREAD_TYPE &ThreadType, const int &iThreadIdx)
{
    PROFILE_DATA *_pdata = CheckData(wName, ThreadType, iThreadIdx);

    // Begin을 이미 한 상태인지 확인
    if (0 == _pdata->lStartTime.QuadPart)
        return;

    LARGE_INTEGER _lendtime;
    QueryPerformanceCounter(&_lendtime);

    unsigned __int64 _ui64time = _lendtime.QuadPart - _pdata->lStartTime.QuadPart;

    _pdata->ui64TotalTime += _ui64time;

    int _ilastidx = 0;
    // max
    for (int _iidx = 0; _iidx < static_cast<int>(PROFILE::MINMAX_SIZE); ++_iidx)
    {
        // 저장된 것보다 작으면 해당 인덱스 저장 후, continue
        if (_pdata->ui64Max[_iidx] > _ui64time)
        {
            _ilastidx = _iidx;
            continue;
        }

        // 여기왔다는 것은 i64Time이 더 크다는 것임.
        // 따라서 저장된 데이터 한 칸씩 이동.
        // 단, 마지막 인덱스에 도달했다면 마지막으로 저장된 j 인덱스에 i64Time 저장.
        if (_iidx + 1 < static_cast<int>(PROFILE::MINMAX_SIZE))
            _pdata->ui64Max[_iidx + 1] = _pdata->ui64Max[_iidx];
        else if (_iidx + 1 == static_cast<int>(PROFILE::MINMAX_SIZE))
            _pdata->ui64Max[_ilastidx] = _ui64time;
    }

    _ilastidx = 0;

    // min
    for (int _iidx = 0; _iidx < static_cast<int>(PROFILE::MINMAX_SIZE); ++_iidx)
    {
        // 저장된 것보다 크면 해당 인덱스 저장 후, continue
        if (_pdata->ui64Min[_iidx] < _ui64time)
        {
            _ilastidx = _iidx;
            continue;
        }

        // 여기왔다는 것은 i64Time이 더 작다는 것임.
        // 따라서 저장된 데이터 한 칸씩 이동.
        // 단, 마지막 인덱스에 도달했다면 마지막으로 저장된 j 인덱스에 i64Time 저장.
        if (_iidx + 1 < static_cast<int>(PROFILE::MINMAX_SIZE))
            _pdata->ui64Min[_iidx + 1] = _pdata->ui64Min[_iidx];
        else if (_iidx + 1 == static_cast<int>(PROFILE::MINMAX_SIZE))
            _pdata->ui64Min[_ilastidx] = _ui64time;
    }

    (_pdata->ui64Call)++;
    _pdata->lStartTime.QuadPart = 0;

    m_bEmptyData = false;

    OUTPUT_INFO * _poutputinfo = nullptr;

    if (m_mProfileOutput.end() == m_mProfileOutput.find(ThreadType))
    {
        m_mProfileOutput.insert(std::make_pair(ThreadType, std::move(std::make_unique<OUTPUT_INFO>(_pdata->wThreadName))));
    }

    _poutputinfo = m_mProfileOutput.find(ThreadType)->second.get();

    if (_poutputinfo->OutputData.end() == _poutputinfo->OutputData.find(&wName))
        _poutputinfo->OutputData.insert(std::make_pair(std::move(&wName), std::move(std::make_unique<OUTPUT_DATA>())));

    OUTPUT_DATA * _poutputdata = _poutputinfo->OutputData.find(&wName)->second.get();
    _poutputdata->ui64Min = (_poutputdata->ui64Min <= _pdata->ui64Min[1] ? _poutputdata->ui64Min : _pdata->ui64Min[1]);
    _poutputdata->ui64Max = (_poutputdata->ui64Max >= _pdata->ui64Max[1] ? _poutputdata->ui64Max : _pdata->ui64Max[1]);
    _poutputdata->ui64Call += _pdata->ui64Call;
    _poutputdata->ui64TotalTime += _pdata->ui64TotalTime;
}

bool NOH::CProfiler::SaveData(void)
{
    if (m_bEmptyData)
        return false;

    WCHAR _wfullpath[static_cast<int>(COMMON::BUF_256)] = { 0 };

    // 파일 이름 만들기
    HRESULT hResult = StringCbPrintf(_wfullpath, sizeof(_wfullpath), L"%s\\Profiler_%04d%02d%02d_%03d.txt", m_spDirectory.get(), CTimerManager::GetInstance()->GetYear(), CTimerManager::GetInstance()->GetMonth(), CTimerManager::GetInstance()->GetDay(), ++m_FileIdx);

    // 매크로
    if (FAILED(hResult))
        return false;

    if (!m_spFileStream.get()->Open(reinterpret_cast<LPCWSTR>(_wfullpath), GENERIC_WRITE, OPEN_ALWAYS))
        return false;

    if (!m_spFileStream.get()->Seek(0, FILE_END))
        return false;

    WCHAR _wbuffer[static_cast<int>(COMMON::BUF_16224)] = { 0 };
    int _ibuffercount = static_cast<int>(COMMON::BUF_16224);
    int _ibufferdatasize = 0;

    //_ibufferdatasize += swprintf_s( _wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize, L"   TheadType   |   ThreadID   |           Name   |         Average   |             Min   |             Max   |         Call  |\r\n" );
    _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize, L"   TheadType   |                Name   |         Average   |             Min   |             Max   |              Call  |\r\n");
    _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize, L"-------------------------------------------------------------------------------------------------------------------------\r\n");

    for (auto & _output : m_mProfileOutput)
    {
        for (auto & _data : _output.second->OutputData)
        {
            _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize,
                L"%12s   |%20s   |%14.4lf㎲   |%14.4lf㎲   |%14.4lf㎲   |%17lld\r\n",
                _output.second->wThreadName.c_str(),
                _data.first.c_str(),
                static_cast<double>(_data.second->ui64TotalTime) / m_dMicroFrequency / _data.second->ui64Call,
                static_cast<double>(_data.second->ui64Min) / m_dMicroFrequency,
                static_cast<double>(_data.second->ui64Max) / m_dMicroFrequency,
                _data.second->ui64Call);
        }

        _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize, L"-------------------------------------------------------------------------------------------------------------------------\r\n");
    }
   
    _ibufferdatasize *= sizeof(WCHAR);

    DWORD _dwbyteswritten = 0;
    if (!m_spFileStream.get()->Write(reinterpret_cast<LPVOID>(_wbuffer), reinterpret_cast<DWORD &>(_ibufferdatasize), _dwbyteswritten))
        return false;

    return true;
}

//bool NOH::CProfiler::SaveData_Detail(void)
//{
//    if (m_bEmptyData)
//        return false;
//
//    WCHAR _wfullpath[static_cast<int>(COMMON::BUF_256)] = { 0 };
//
//    // 파일 이름 만들기
//    HRESULT hResult = StringCbPrintf(_wfullpath, sizeof(_wfullpath), L"%s\\Profiler_%04d%02d%02d_%03d.txt", m_spDirectory.get(), CTimerManager::GetInstance()->GetYear(), CTimerManager::GetInstance()->GetMonth(), CTimerManager::GetInstance()->GetDay(), ++m_FileIdx);
//
//    // 매크로
//    if (FAILED(hResult))
//        return false;
//
//    if (!m_spFileStream.get()->Open(reinterpret_cast<LPCWSTR>(_wfullpath), GENERIC_WRITE, OPEN_ALWAYS))
//        return false;
//
//    if (!m_spFileStream.get()->Seek(0, FILE_END))
//        return false;
//
//    WCHAR _wbuffer[static_cast<int>(COMMON::BUF_16224)] = { 0 };
//    int _ibuffercount = static_cast<int>(COMMON::BUF_16224);
//    int _ibufferdatasize = 0;
//
//    int _ithreadtype = 0;
//    int _ithreadidx = 0;
//    int idataidx = 0;
//
//    PROFILE_THREAD_TYPE *_pprofile = nullptr;
//    PROFILE_THREAD *_pprofilethread = nullptr;
//
//    _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize, L"   TheadType   |   ThreadID   |           Name   |         Average   |             Min   |             Max   |         Call  |\r\n");
//    _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize, L"------------------------------------------------------------------------------------------------------------------------------\r\n");
//
//    while (static_cast<int>(PROFILE::THREAD_TYPE_COUNT) != _ithreadtype)
//    {
//        _pprofile = &m_spProfile.get()[_ithreadtype++];
//
//        for (_ithreadidx = 0; _ithreadidx < static_cast<int>(PROFILE::THREAD_COUNT); ++_ithreadidx)
//        {
//            if (false == _pprofile->ProfileThread[_ithreadidx].bUse)
//                continue;
//
//            for (idataidx = 0; idataidx < static_cast<int>(PROFILE::DATA_SIZE); ++idataidx)
//            {
//                if (true == _pprofile->ProfileThread[_ithreadidx].ProfileData[idataidx].bUse)
//                {
//                    _pprofilethread = &_pprofile->ProfileThread[_ithreadidx];
//
//                    _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize,
//                        L"%12s   |%11d   |%15s   |%14.4lf㎲   |%14.4lf㎲   |%14.4lf㎲   |%12lld\r\n",
//                        _pprofilethread->wThreadName,
//                        _pprofilethread->dwThreadID,
//                        _pprofilethread->ProfileData[idataidx].wName,
//                        static_cast<double>(_pprofilethread->ProfileData[idataidx].ui64TotalTime) / m_dMicroFrequency / _pprofilethread->ProfileData[idataidx].ui64Call,
//                        static_cast<double>(_pprofilethread->ProfileData[idataidx].ui64Min[0]) / m_dMicroFrequency,
//                        static_cast<double>(_pprofilethread->ProfileData[idataidx].ui64Max[0]) / m_dMicroFrequency,
//                        _pprofilethread->ProfileData[idataidx].ui64Call);
//                }
//            }
//
//            _ibufferdatasize += swprintf_s(_wbuffer + _ibufferdatasize, _ibuffercount - _ibufferdatasize, L"------------------------------------------------------------------------------------------------------------------------------\r\n");
//        }
//    };
//
//    _ibufferdatasize *= sizeof(WCHAR);
//
//    DWORD _dwbyteswritten = 0;
//    if (!m_spFileStream.get()->Write(reinterpret_cast<LPVOID>(_wbuffer), reinterpret_cast<DWORD &>(_ibufferdatasize), _dwbyteswritten))
//        return false;
//
//    return true;
//}

NOH::PROFILE_DATA * NOH::CProfiler::CheckData(const WCHAR &wName, const NOH::THREAD_TYPE &ThreadType, const int &iThreadIdx)
{
    PROFILE_THREAD_INFO *_pprofilethreadinfo = nullptr;
    PROFILE_THREAD * _pprofilethread = m_mProfile.find(ThreadType)->second.get();

    // 미 사용이면 추가
    if (!_pprofilethread->bUse)
        _pprofilethread->bUse = true;

    // 없다면 만들기
    if (_pprofilethread->ProfileThreadInfo.end() == _pprofilethread->ProfileThreadInfo.find(iThreadIdx))
    {
        _pprofilethread->ProfileThreadInfo.insert(std::make_pair(iThreadIdx, std::move(std::make_unique<PROFILE_THREAD_INFO>())));
        _pprofilethreadinfo = _pprofilethread->ProfileThreadInfo.find(iThreadIdx)->second.get();

        switch (ThreadType)
        {
        case THREAD_TYPE::THREAD_MAIN:
            _pprofilethreadinfo->wThreadName = L"MAIN";
            break;
        case THREAD_TYPE::THREAD_LAN:
            _pprofilethreadinfo->wThreadName = L"LAN";
            break;
        case THREAD_TYPE::THREAD_NET:
            _pprofilethreadinfo->wThreadName = L"NET";
            break;
        case THREAD_TYPE::THREAD_ACCEPT:
            _pprofilethreadinfo->wThreadName = L"ACCEPT";
            break;
        case THREAD_TYPE::THREAD_TIMER:
            _pprofilethreadinfo->wThreadName = L"TIMER";
            break;
        }

        _pprofilethreadinfo->dwThreadID = GetCurrentThreadId();
        _pprofilethreadinfo->ProfileData.insert(std::make_pair(std::move(&wName), std::move(std::make_unique<PROFILE_DATA>(_pprofilethreadinfo->wThreadName, &wName))));
    }
    else
    {
        _pprofilethreadinfo = _pprofilethread->ProfileThreadInfo.find(iThreadIdx)->second.get();

        // 없다면 만들기
        if (_pprofilethreadinfo->ProfileData.end() == _pprofilethreadinfo->ProfileData.find(&wName))
            _pprofilethreadinfo->ProfileData.insert(std::make_pair(std::move(&wName), std::move(std::make_unique<PROFILE_DATA>(_pprofilethreadinfo->wThreadName, &wName))));
    }

    return _pprofilethread->ProfileThreadInfo.find(iThreadIdx)->second->ProfileData.find(&wName)->second.get();
}

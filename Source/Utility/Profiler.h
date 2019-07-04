/*
CThread 클래스를 상속 받는 클래스의 경우, GetThreadType() 와 GetThreadIdx() 를 반드시 만들어야 함.

-- 실제 사용 방법
PROFILE_BEGIN(*L"apple", GetThreadType(), GetThreadIdx());
PROFILE_END(*L"apple", GetThreadType(), GetThreadIdx());

*/

#pragma once

#include <WinSock2.h>
#include <memory>
#include <map>

#include "__NOH.h"
#include "Singleton.h"


//#ifdef PROFILIE_CHECK
	#define PROFILE_BEGIN(wName, ThreadType, ThreadIdx)     NOH::CProfiler::GetInstance()->Begin(wName, ThreadType, ThreadIdx);
	#define PROFILE_END(wName, ThreadType, ThreadIdx)       NOH::CProfiler::GetInstance()->End(wName, ThreadType, ThreadIdx);
//#else
//	#define //PROFILE_BEGIN(x)   
//	#define //PROFILE_END(x)
//#endif

namespace NOH
{
    // __NOH.h 로 옮김 
    //enum class PROFILE
    //{
    //    THREAD_TYPE_COUNT   = 10,       // 스레드 종류 개수 예) Accept, iocp 만 있으면 종류는 2개
    //    THREAD_COUNT        = 10,       // 스레드 종류 별 스레드 개수
    //    DATA_SIZE           = 100,
    //    MINMAX_SIZE         = 4,
    //};

    typedef struct st_OUTPUT_DATA
    {
	    unsigned __int64	ui64TotalTime;
	    unsigned __int64	ui64Min;
	    unsigned __int64	ui64Max;
	    unsigned __int64	ui64Call;

        st_OUTPUT_DATA() : ui64TotalTime(0), ui64Min(-1), ui64Max(0), ui64Call(0)
        {
        }
    } OUTPUT_DATA;

    typedef struct st_OUTPUT_INFO
    {
        std::wstring                                                wThreadName;
        std::map<const std::wstring, std::unique_ptr<OUTPUT_DATA>>  OutputData;

        st_OUTPUT_INFO(const std::wstring & _wThreadName) : wThreadName(std::move(_wThreadName))
        {
        }
    } OUTPUT_INFO;

    typedef struct st_PROFILE_DATA
    {
        const std::wstring  wThreadName;
        const std::wstring	wName;
	    LARGE_INTEGER		lStartTime;
	    unsigned __int64	ui64TotalTime;
	    unsigned __int64	ui64Min[static_cast<int>( PROFILE::MINMAX_SIZE )];
	    unsigned __int64	ui64Max[static_cast<int>( PROFILE::MINMAX_SIZE )];
	    unsigned __int64	ui64Call;

        st_PROFILE_DATA(const std::wstring & _wThreadName, const std::wstring & _wName) : wThreadName(std::move(_wThreadName)), wName(std::move(_wName)), ui64TotalTime(0), ui64Call(0)
        {
            memset(&lStartTime, 0, sizeof(LARGE_INTEGER));

            for ( int _iidx = 0; _iidx < static_cast<int>( PROFILE::MINMAX_SIZE ); ++_iidx )
            {
                ui64Min[_iidx] = -1;
                ui64Max[_iidx] = 0;
            }
        }
    } PROFILE_DATA;

    typedef struct st_PROFILE_THREAD_INFO
    {
        DWORD                     dwThreadID;
        std::wstring              wThreadName;
        std::map<const std::wstring, std::unique_ptr<PROFILE_DATA>> ProfileData;

        st_PROFILE_THREAD_INFO() : dwThreadID(0) 
        {
        }
    } PROFILE_THREAD_INFO;

    typedef struct st_PROFILE_THREAD
    {
        bool                            bUse;
	    std::map<const int, std::unique_ptr<PROFILE_THREAD_INFO>>   ProfileThreadInfo;

        st_PROFILE_THREAD() : bUse(false) {}
    } PROFILE_THREAD;

    class CFileStream;
    class CProfiler : public CSingleton<CProfiler>
    {
    public:
        CProfiler();
        ~CProfiler();

        void Initialize( const WCHAR &wDirectory );

        void Begin( const WCHAR &wName, const NOH::THREAD_TYPE &ThreadType, const int &iThreadIdx );
        void End( const WCHAR &wName, const NOH::THREAD_TYPE &ThreadType, const int &iThreadIdx );
        bool SaveData(void);
        bool SaveData_Detail(void);

        DWORD GetThreadType(void) { return InterlockedIncrement(&m_dwThreadType); }
        DWORD GetThreadIdx(NOH::THREAD_TYPE ThreadType) { return InterlockedIncrement(&m_dwThreadIdx[static_cast<int>(ThreadType)]); }

    private:
        PROFILE_DATA *  CheckData( const WCHAR &wName, const NOH::THREAD_TYPE &ThreadType, const int &iThreadIdx );

    private:
        bool                                                            m_bEmptyData;
        std::unique_ptr<CFileStream>                                    m_spFileStream;
        std::unique_ptr<WCHAR[]>                                        m_spDirectory;

        std::map<const THREAD_TYPE, std::unique_ptr<PROFILE_THREAD>>    m_mProfile;
        std::map<const THREAD_TYPE, std::unique_ptr<OUTPUT_INFO>>       m_mProfileOutput;


        LARGE_INTEGER		                                            m_lFrequency;
        double				                                            m_dMicroFrequency;
        int					                                            m_FileIdx;

        DWORD                                                           m_dwThreadType;
        DWORD                                                           m_dwThreadIdx[static_cast<int>(THREAD_TYPE::THREAD_TYPE_TOTAL)];
    };
    
}
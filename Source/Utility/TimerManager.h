/*
    // 초 단위
    // 등록 즉시 호출 후 등록 시간 마다 호출
    CTimerManager::GetInstance()->RegisterTimer( func, 60, TIMER_TYPE::INSTANT_TIMER);
    // 매 시간 마다 호출
    CTimerManager::GetInstance()->RegisterTimer( func, FLAT_TIME::EVERY_1_MINUTE, TIMER_TYPE::FLAT_TIMER);

    CTimerManager::GetInstance()->RemoveTimer( func );
*/
#pragma once

#include <list>
#include <functional>
#include <memory>

#include "__NOH.h"
#include "Thread.h"
#include "Singleton.h"


// 동기화 신경 안씀. 단일 스레드로 실행.
namespace NOH
{
    class CTimerManager : public CSingleton<CTimerManager>
    {
        class CTimer : public CThread
        {
            using TIMERFUNC = void (*) (void);

            typedef struct st_TIMER_INFO
	        {
		        TIMERFUNC		Func;
                LONGLONG        llInstantTime;
		        FLAT_TIME		FlatTime;
		        TIMER_TYPE		TimerType;
                LARGE_INTEGER   lInitTime;

		        st_TIMER_INFO( void ) 
                    : Func(nullptr), llInstantTime(-1), FlatTime(FLAT_TIME::DEFAULT), TimerType(TIMER_TYPE::DEFAULT), lInitTime({0}) {}
		        st_TIMER_INFO( const TIMERFUNC& func, const LONGLONG llinstanttime, const FLAT_TIME flattime, const TIMER_TYPE timertype, LARGE_INTEGER inittime )
			        : Func(func), FlatTime(flattime), TimerType(timertype), lInitTime(inittime) {}
		       
	        } TIMER_INFO;

        public:
            CTimer() 
                : m_hIOCP(INVALID_HANDLE_VALUE), m_ThreadType(NOH::THREAD_TYPE::THREAD_TIMER), m_dMilliFrequency(0) {};
            ~CTimer() {};

            void         Initialize( void );
                         
            virtual void Run( void ) override;
            virtual void Close( void ) override;
            virtual void SetIOCPHandle( HANDLE hIOCP ) override { m_hIOCP = hIOCP; }

            const DWORD &       GetThreadIdx( void ) { return this->m_dwProfileTlsIndex; }
            const THREAD_TYPE & GetThreadType( void ) { return m_ThreadType; }

        public:
	        void RegisterTimer( const TIMERFUNC& Func, const LONGLONG llInstantTime, const TIMER_TYPE TimerType );
            void RegisterTimer( const TIMERFUNC& Func, const FLAT_TIME FlatTime, const TIMER_TYPE TimerType );
            // 타이머 함수 제거
	        void RemoveTimer( const TIMERFUNC& Func );
            // 타이머 전부 제거
            void RemoveAllTimer( void );
            
            WORD GetDay() const { return m_LocalTime.wDay; }
            WORD GetHour() const { return m_LocalTime.wHour; }
            WORD GetMinute() const { return m_LocalTime.wMinute; }
            WORD GetMonth() const { return m_LocalTime.wMonth; }
            WORD GetSecond() const { return m_LocalTime.wSecond; }
            WORD GetYear() const { return m_LocalTime.wYear; }
            WORD GetDayOfWeek() const { return m_LocalTime.wDayOfWeek; }


        private:
            using TIMERLIST = std::list<TIMER_INFO>;

            HANDLE			m_hIOCP;
            TIMERLIST       m_TimerList;
            SYSTEMTIME      m_LocalTime;

            double          m_dMilliFrequency;
            double          m_dSecondFrequency;
            SRWLOCK         m_SRWLock;

            THREAD_TYPE     m_ThreadType;
        };

    public:
        CTimerManager()
            : m_spTimer(std::make_unique<CTimer>()) {};
        ~CTimerManager() {};

        void Initialize( void );
        void Close( void );

        CTimer* GetTimer( void ) { return m_spTimer.get(); }

        WORD GetDay() const { return m_spTimer.get()->GetDay(); }
        WORD GetHour() const { return m_spTimer.get()->GetHour(); }
        WORD GetMinute() const { return m_spTimer.get()->GetMinute(); }
        WORD GetMonth() const { return m_spTimer.get()->GetMonth(); }
        WORD GetSecond() const { return m_spTimer.get()->GetSecond(); }
        WORD GetYear() const { return m_spTimer.get()->GetYear(); }
        WORD GetDayOfWeek() const { return m_spTimer.get()->GetDayOfWeek(); }

    private:
        std::unique_ptr<CTimer>      m_spTimer;
    };
   
}
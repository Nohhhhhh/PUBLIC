#pragma once

#include <WinSock2.h>
#include <Mstcpip.h>

#include "__NOH.h"
#include "Singleton.h"
#include "Stack_LF.h"
#include "Session.h"
#include "SocketManager.h"

#define INPUT_ARRAYIDX(CODE, ARRAYIDX)		((CODE >> 48 | ARRAYIDX) << 48) 
#define INPUT_SESSIONID(CODE, SESSIONID)	(CODE | SESSIONID)
#define GET_ARRAYIDX(CODE)					(CODE >> 48)

namespace NOH
{
    class CObject;
    template <class T>
    class CSessionManager : public CSingleton<CSessionManager<T>>
    {
    public:
        CSessionManager();
        ~CSessionManager();

    public:
        void Initiailize(int iMaxUserCount);
        void Close(void);

        CSession* Search(SESSION_CODE SessionCode);

        UINT64*     GetSessionID(void) { return &m_ui64SessionID; }
        CSession*   GetSession(const int iSessionIdx) { return reinterpret_cast<CSession *>(m_vecUserInfo[iSessionIdx].get()); }
        int         GetMaxUserCount(void) { return m_iMaxSessionCount; }

        bool        Get(int * ipSessionIdx) { return m_spSessionStack.get()->Get(ipSessionIdx); }
        void        Put(int iSessionIdx) { m_spSessionStack.get()->Put(iSessionIdx); }

        // TODO
        bool        CheckSessionBan(const CSession & Session) { return true; }

    private:
        std::unique_ptr<CStack_LF<int>>	        m_spSessionStack;
        //CStack_LF<int>                          *m_pSessionStack;
        int                                     m_iMaxSessionCount;

        std::vector<std::unique_ptr<T>>     	m_vecUserInfo;
        //CObject                                 **m_ppUserInfo;
        UINT64				                      m_ui64SessionID;
    };

    template<class T>
    inline CSessionManager<T>::CSessionManager()
        : m_spSessionStack(nullptr),/*m_pSessionStack(nullptr),*/ m_iMaxSessionCount(0), m_vecUserInfo(0),/*m_ppUserInfo(nullptr),*/ m_ui64SessionID(0)
    {
    }

    template<class T>
    inline CSessionManager<T>::~CSessionManager()
    {
    }

    template<class T>
    inline void CSessionManager<T>::Initiailize(int iMaxSessionCount)
    {
        m_iMaxSessionCount = iMaxSessionCount;

        m_spSessionStack.reset(std::move(new CStack_LF<int>(iMaxSessionCount)));
        //m_pSessionStack = new CStack_LF<int>(iMaxSessionCount);

        for (int iUserIdx = iMaxSessionCount; iUserIdx > 0; --iUserIdx)
            m_spSessionStack.get()->Put(iUserIdx - 1);

        //m_ppUserInfo = new CObject*[iMaxSessionCount];

        for (int iUserIdx = 0; iUserIdx < iMaxSessionCount; ++iUserIdx)
        {
            m_vecUserInfo.emplace_back(std::make_unique<T>());
            m_vecUserInfo[iUserIdx].get()->Initialize();
            //m_ppUserInfo[iUserIdx] = new T;
            //static_cast<T *>(m_ppUserInfo[iUserIdx])->Initialize();
        }
    }

    template<class T>
    inline void CSessionManager<T>::Close(void)
    {
        if (0 != m_vecUserInfo.size())
        {
            int iCnt = 0;
            int iWaitCnt = 0;
            int iSessionCnt = 0;
            T * pObject = nullptr;

            while (1)
            {
                for (iCnt = 0; iCnt < m_iMaxSessionCount; ++iCnt)
                {
                    pObject = m_vecUserInfo[iCnt].get();

                    if (1 != pObject->GetIOInfo()->llReleaseFlag)
                    {
                        CSocketManager<T>::GetInstance()->DisconnectSocket(*pObject->GetCompletionKey(), DISCONNECT_TYPE::REUSESOCKET);
                        ++iSessionCnt;
                    }
                }

                if (0 == iSessionCnt)
                    break;

                Sleep(100);

                // 강제 종료
                if (10 <= iWaitCnt)
                {
                    for (iCnt = 0; iCnt < m_iMaxSessionCount; ++iCnt)
                    {
                        pObject = m_vecUserInfo[iCnt].get();

                        if (1 != pObject->GetIOInfo()->llReleaseFlag)
                            CSocketManager<T>::GetInstance()->DisconnectSocket(*pObject->GetCompletionKey(), DISCONNECT_TYPE::FORCED);
                    }
                }

                ++iWaitCnt;
                iSessionCnt = 0;
            }

            for (auto & iter : m_vecUserInfo)
            static_cast<CSession *>(iter.get())->Close();

            m_vecUserInfo.clear();
        }

        if (nullptr != m_spSessionStack)
        {
            m_spSessionStack.release();
            //delete m_pSessionStack;
            //m_pSessionStack = nullptr;
        }
    }

    template<class T>
    inline CSession * CSessionManager<T>::Search(SESSION_CODE SessionCode)
    {
        CSession * pSession = static_cast<CSession *>(m_ppUserInfo[GET_ARRAYIDX(SessionCode)]);

        if (SessionCode == pSession->GetCompletionKey())
            return pSession;

        return nullptr;
    }

}

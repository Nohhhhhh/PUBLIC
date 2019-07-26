#pragma once

#include "__NOH.h"
#include "Singleton.h"
#include "Thread.h"

#include <vector>
#include <memory>

namespace NOH
{
    template <class T>
    class CIOCPHandler : public CSingleton<CIOCPHandler<T>>
    {
    public:
        CIOCPHandler();
        ~CIOCPHandler();

        bool Initialize(const DWORD dwMaxThreadCount = 0);
        bool AssociateListenSocket(const HANDLE hFile, const SOCKET_TYPE SocketType);
        void Close(void);
        void RunThread(void);

        HANDLE GetIOCPHandle(void) { return m_hIOCP; }

    private:
        bool CreateThreadPool(void);
        void CloseIOCP(void);

        HANDLE                              m_hIOCP;							// IOCP 핸들
        DWORD                               m_dwWorkerThread;					// Worker Thread의 개수	
        std::vector<std::unique_ptr<T>>     m_vecWorkerThread;         // Worker Thread 포인터 - smartpointer version
    };

    template<class T>
    inline CIOCPHandler<T>::CIOCPHandler()
        : m_hIOCP(INVALID_HANDLE_VALUE), m_dwWorkerThread(0)
    {
    }

    template<class T>
    inline CIOCPHandler<T>::~CIOCPHandler()
    {
    }

    template<class T>
    inline bool CIOCPHandler<T>::Initialize(const DWORD dwMaxThreadCount)
    {
        m_dwWorkerThread = dwMaxThreadCount;

        m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

        if (!m_hIOCP)
            return false;

        if (!CreateThreadPool())
            return false;

        return true;
    }

    template<class T>
    inline bool CIOCPHandler<T>::AssociateListenSocket(const HANDLE hFile, const SOCKET_TYPE SocketType)
    {
        if (m_hIOCP != CreateIoCompletionPort(hFile, m_hIOCP, 0, 0))
            return false;

        for (DWORD dwCPU = 0; dwCPU != m_dwWorkerThread; ++dwCPU)
            m_vecWorkerThread[dwCPU].get()->SetAcceptExSockAddrInfo(reinterpret_cast<SOCKET>(hFile), SocketType);

        return true;
    }

    template<class T>
    inline void CIOCPHandler<T>::Close(void)
    {
        CloseIOCP();

        if (0 != m_vecWorkerThread.size())
        {
            for (DWORD dwIndex = 0; dwIndex != m_dwWorkerThread; ++dwIndex)
                m_vecWorkerThread[dwIndex].get()->Close();

            m_vecWorkerThread.clear();
        }

        if (m_hIOCP)
        {
            CloseHandle(m_hIOCP);
            m_hIOCP = INVALID_HANDLE_VALUE;
        }
    }

    template<class T>
    inline void CIOCPHandler<T>::RunThread(void)
    {
        for (DWORD dwCPU = 0; dwCPU != m_dwWorkerThread; ++dwCPU)
            m_vecWorkerThread[dwCPU].get()->SetStartFlag();
    }

    template<class T>
    inline bool CIOCPHandler<T>::CreateThreadPool(void)
    {
        SYSTEM_INFO systemInfo;

        if (0 == m_dwWorkerThread)
        {
            GetSystemInfo(&systemInfo);
            m_dwWorkerThread = systemInfo.dwNumberOfProcessors * 2 + 2;
        }

        for (DWORD dwCPU = 0; dwCPU != m_dwWorkerThread; ++dwCPU)
        {
            m_vecWorkerThread.emplace_back(std::move(std::make_unique<T>()));
            m_vecWorkerThread[dwCPU].get()->Begin();
            m_vecWorkerThread[dwCPU].get()->SetIOCPHandle(m_hIOCP);
        }

        return true;
    }

    template<class T>
    inline void CIOCPHandler<T>::CloseIOCP(void)
    {
        if (INVALID_HANDLE_VALUE == m_hIOCP)
            return;

        for (DWORD i = 0; i != m_dwWorkerThread; ++i)
            PostQueuedCompletionStatus(m_hIOCP, static_cast<DWORD>(PQCS_TYPE::CLOSE_THREAD), reinterpret_cast<ULONG_PTR>(nullptr), reinterpret_cast<ULONG_PTR>(nullptr));
    }
}
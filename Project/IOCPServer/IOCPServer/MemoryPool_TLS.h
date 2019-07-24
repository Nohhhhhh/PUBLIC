/*
MemoryPool_LF에서 CHUNK_DATA 타입으로 메모리를 Alloc 1번당 1개의 CHUNK가 할당되는 것이고,
MemoryPool_TLS에서는 MemoryPool_LF에서 할당 받은 CHUNK에 원하는 개수 만큼의 메모리 BLOCK이 연결 되는 구조.
*/
#pragma once

#include "MemoryPool_LF.h"

#define df_MEMORY_POOL_CHUNK_BLOCK_CHECK	0x92d1c3acbe8fba77

namespace NOH
{
    template <class TLS_DATA>
    class CMemoryPool_TLS
    {
        template <class CHUNK_DATA>
        class CChunkBlock
        {
        public:
            //------------------------------------------------------------------------
            // DATA_BLCOK 구조체가 여러 개 연결 되어 있는 구조
            // DATA_BLOCK 1개 당 여러 개의 CHUNK_DATA가 연결되어 있는 구조
            // CChunkBlock<CHUNK_DATA>은 MemoryPool_LF 에서 할당 됨
            // CMemoryPool_TLS에서 할당된 CChunkBlock<CHUNK_DATA>를 반환
            // 이때, CChunkBlock<CHUNK_DATA>의 주소를 바로 반환하는게 아니라 CHUNK_DATA의 주소를 반환
            //------------------------------------------------------------------------
            typedef struct st_DATA_BLOCK
            {
                CChunkBlock<CHUNK_DATA>		*pChunkBlock;
                UINT64						ui64BlockCheck;
                CHUNK_DATA					Data;
                st_DATA_BLOCK				*pNextBlock;

                st_DATA_BLOCK(CChunkBlock<CHUNK_DATA> *pChunk, UINT64 ui64Check, st_DATA_BLOCK *pData)
                    : pChunkBlock(pChunk), ui64BlockCheck(ui64Check), pNextBlock(pData) {}
            } DATA_BLOCK;

            friend class CMemoryPool_TLS<TLS_DATA>;

        public:
            CChunkBlock();
            virtual ~CChunkBlock();

            //-------------------------------------------------------------------------
            // param : void
            // return: CHUNK_DATA *
            //
            // Data 블럭 할당
            //
            // - 
            // Chunk의 Data 블럭 중 1개를 할당
            //-------------------------------------------------------------------------
            CHUNK_DATA * Alloc(void);

            //-------------------------------------------------------------------------
            // param : void
            // return: void
            //
            // Data 블럭 또는 Chunk 반환
            //
            // -
            // 해당 함수 호출 시, m_lAllocRef 감소.
            // m_lAllocRef == 0 이면, Chunk 반환.
            //-------------------------------------------------------------------------
            bool Free(bool bReplacementNew);

            //-------------------------------------------------------------------------
            // param : CMemoryPool_TLS<TLS_DATA> * (Constructor 함수를 호출한 객체)
            // return: void
            //
            // DATA_BLOCK 할당하여 연결
            //
            // -
            // m_pDataBlock에 여러 개의 DATA_BLOCK을 할당하여 연결하기.
            //-------------------------------------------------------------------------
            void Constructor(CMemoryPool_TLS<TLS_DATA> & MemoryPool_TLS, const long lChunkSize);

            //-------------------------------------------------------------------------
           // param : long (CHUNK 개수)
           // return: void
           //
           // 멤버를 초기화
           //
           // -
           // CChunkBlock<CHUNK_DATA>의 멤버를 초기화
           //-------------------------------------------------------------------------
            void Initialize(long lChunkSize);

        private:
            //------------------------------------------------------------------------
            // Chunk 할당 및 반환 시, 사용될 alloc 된 MemoryPool_TLS 포인터
            //------------------------------------------------------------------------
            CMemoryPool_TLS<TLS_DATA> *m_pMemoryPool_TLS;

            size_t m_PointerSize;
            size_t m_BlockCheckDataSize;

            //------------------------------------------------------------------------
            // Data 블럭 포인터
            //------------------------------------------------------------------------
            DATA_BLOCK	*m_pDataBlock;
            //------------------------------------------------------------------------
            // Top Data 블럭 (Alloc 될 때마다 이동)
            //------------------------------------------------------------------------
            DATA_BLOCK	*m_pAllocTop;

            //------------------------------------------------------------------------
            // Chunk의 Data 블럭 개수
            //------------------------------------------------------------------------
            long	m_lAllocMax;
            //------------------------------------------------------------------------
            // Free 시, 감소 될 Data 블럭 개수
            // 0이 되면, Chunk 반환
            //------------------------------------------------------------------------
            long	m_lAllocRef;
        };

    public:
        //-------------------------------------------------------------------------
        // param : long (생성할 Chunk Data 블럭 개수), bool (replacement 여부)
        // return: 
        //
        // 생성자, 파괴자
        //
        // iBlockNum = 0 이면, Alloc을 통해 동적 메모리 할당
        // iBlockNum != 0 이면, 미리 메모리 할당
        // -
        // 모든 생성된 노드 delete
        //-------------------------------------------------------------------------
        explicit CMemoryPool_TLS(const long lChunkSize, const bool bReplacementNew);
        virtual ~CMemoryPool_TLS();

        //-------------------------------------------------------------------------
        // param : void
        // return: TLS_DATA *
        //
        // TLS_DATA * 할당
        // 
        // -
        // TLS에 등록된 CChunkBlock<CHUNK_DATA> 에서 CHUNK_DATA를 반환
        //-------------------------------------------------------------------------
        TLS_DATA * Alloc(void);

        //-------------------------------------------------------------------------
       // param : TLS_DATA * (반환 될 포인터)
       // return: void
       //
       // TLS_DATA * 반환
       // 
       // -
       // TLS_DATA *로 CChunkBlock<TLS_DATA> * 형태로 만든 후, CChunkBlock<TLS_DATA>의 Free() 함수 호출
       // 실제 반환 여부는 CChunkBlock<TLS_DATA>의 Free()에서 판단
       //-------------------------------------------------------------------------
        void Free(TLS_DATA *pData);

        //-------------------------------------------------------------------------
        // param : void
        // return: CChunkBlock<TLS_DATA> * (반환 될 포인터)
        //
        // CChunkBlock<TLS_DATA> * 할당
        // 
        // -
        // m_pChunkBlockMemoryPool을 통해서 CChunkBlock<TLS_DATA>을 할당
        //-------------------------------------------------------------------------
        CChunkBlock<TLS_DATA> * ChunkAlloc(void);

        long		GetChunkCount(void) { return m_lChunkCnt; }
        long		GetUsingBlockCount(void) { return m_lUsingBlockCnt; }
        long		GetUsingChunkCount(void) { return m_lUsingChunkCnt; }

    private:
        //------------------------------------------------------------------------
        // CChunkBlock<TLS_DATA>를 할당해줄 MemoryPool_LF 클래스 포인터
        //------------------------------------------------------------------------
        //CMemoryPool_LF<CChunkBlock<TLS_DATA>>		*m_pChunkBlockMemoryPool;
        std::unique_ptr<CMemoryPool_LF<CChunkBlock<TLS_DATA>>> m_spChunkBlockMemoryPool;

        DWORD								m_dwTlsIndex;
        long								m_lChunkCnt;	    // 총 만들어진 Chunk 개수
        long								m_lUsingBlockCnt;	// 총 사용준인 DATA 개수
        long								m_lUsingChunkCnt;	// 총 사용중인 Chunk 개수
        long								m_lChunkSize;
        bool								m_bReplacementNew;
    };

    template<class TLS_DATA>
    template<class CHUNK_DATA>
    inline CMemoryPool_TLS<TLS_DATA>::CChunkBlock<CHUNK_DATA>::CChunkBlock(void)
        : m_pMemoryPool_TLS(nullptr), m_PointerSize(sizeof(void*)), m_BlockCheckDataSize(sizeof(UINT64)), m_pDataBlock(nullptr), m_pAllocTop(nullptr), m_lAllocMax(0), m_lAllocRef(0)
    {

    }

    template<class TLS_DATA>
    template<class CHUNK_DATA>
    inline CMemoryPool_TLS<TLS_DATA>::CChunkBlock<CHUNK_DATA>::~CChunkBlock(void)
    {
        DATA_BLOCK *pRealTopBlock = m_pDataBlock;
        DATA_BLOCK *pDeleteBlock = nullptr;

        for (int iBlockCnt = 0; iBlockCnt < m_lAllocMax; ++iBlockCnt)
        {
            pDeleteBlock = pRealTopBlock;
            pRealTopBlock = pRealTopBlock->pNextBlock;

            delete[] pDeleteBlock;
            pDeleteBlock = nullptr;
        }
    }

    template<class TLS_DATA>
    template<class CHUNK_DATA>
    inline CHUNK_DATA * CMemoryPool_TLS<TLS_DATA>::CChunkBlock<CHUNK_DATA>::Alloc(void)
    {
        CHUNK_DATA * pReturnData = reinterpret_cast<CHUNK_DATA *>((reinterpret_cast<char *>(m_pAllocTop) + (sizeof(void *) + sizeof(UINT64))));

        m_pAllocTop = m_pAllocTop->pNextBlock;

        // 더 이상 DATA_BLOCK이 없을 때, 미리 할당
        if (m_pAllocTop == nullptr)
            m_pMemoryPool_TLS->ChunkAlloc();

        return pReturnData;
    }

    template<class TLS_DATA>
    template<class CHUNK_DATA>
    inline bool CMemoryPool_TLS<TLS_DATA>::CChunkBlock<CHUNK_DATA>::Free(bool bReplacementNew)
    {
        if (0 != InterlockedDecrement(&m_lAllocRef))
            return false;

        if (m_pDataBlock->ui64BlockCheck != df_MEMORY_POOL_CHUNK_BLOCK_CHECK)
            CDump::GetInstance()->Crash();

        m_pMemoryPool_TLS->m_spChunkBlockMemoryPool.get()->Free(m_pDataBlock->pChunkBlock, bReplacementNew);

        return true;
    }

    template<class TLS_DATA>
    template<class CHUNK_DATA>
    inline void CMemoryPool_TLS<TLS_DATA>::CChunkBlock<CHUNK_DATA>::Constructor(CMemoryPool_TLS<TLS_DATA> & MemoryPool_TLS, const long lChunkSize)
    {
        m_pMemoryPool_TLS = &MemoryPool_TLS;
        m_lAllocMax = lChunkSize;
        m_lAllocRef = lChunkSize;

        DATA_BLOCK *pTempBlock = nullptr;

        for (int iBlockCnt = 0; iBlockCnt < lChunkSize; ++iBlockCnt)
        {
            DATA_BLOCK *pNewBlock = new DATA_BLOCK(this, df_MEMORY_POOL_CHUNK_BLOCK_CHECK, nullptr);

            if (0 == iBlockCnt)
            {
                m_pDataBlock = pNewBlock;
                m_pAllocTop = pNewBlock;
                pTempBlock = pNewBlock;
            }
            else
            {
                pTempBlock->pNextBlock = pNewBlock;
                pTempBlock = pTempBlock->pNextBlock;
            }
        }
    }

    template<class TLS_DATA>
    template<class CHUNK_DATA>
    inline void CMemoryPool_TLS<TLS_DATA>::CChunkBlock<CHUNK_DATA>::Initialize(long lChunkSize)
    {
        m_pAllocTop = m_pDataBlock;
        m_lAllocRef = lChunkSize;
    }

    template<class TLS_DATA>
    inline CMemoryPool_TLS<TLS_DATA>::CMemoryPool_TLS(const long lChunkSize, const bool bReplacementNew)
        : m_dwTlsIndex(TlsAlloc()), m_lChunkSize(lChunkSize), m_bReplacementNew(bReplacementNew), m_lUsingBlockCnt(0), m_lUsingChunkCnt(0), m_lChunkCnt(0), m_spChunkBlockMemoryPool(std::make_unique<CMemoryPool_LF<CChunkBlock<TLS_DATA>>>())//, m_pChunkBlockMemoryPool(nullptr)
    {
        //if (TLS_OUT_OF_INDEXES == m_dwTlsIndex)
        //    CRASH();

        //m_pChunkBlockMemoryPool = new NOH::CMemoryPool_LF<CChunkBlock<TLS_DATA>>();
    }

    template<class TLS_DATA>
    inline CMemoryPool_TLS<TLS_DATA>::~CMemoryPool_TLS()
    {
        CChunkBlock<TLS_DATA> *pChunkBlock = (CChunkBlock<TLS_DATA> *)TlsGetValue(m_dwTlsIndex);

        // CChunkBlock<CHUNK_DATA>::Alloc() 에서 미리 할당 후, 전혀 사용되지 않은 Block 메모리 해제
        while (nullptr != pChunkBlock)
        {
            if (true == pChunkBlock->Free(m_bReplacementNew))
                break;
        }

        //delete[] m_pChunkBlockMemoryPool;
        //m_pChunkBlockMemoryPool = nullptr;
        TlsFree(m_dwTlsIndex);
    }

    template<class TLS_DATA>
    inline TLS_DATA * CMemoryPool_TLS<TLS_DATA>::Alloc(void)
    {
        CChunkBlock<TLS_DATA> *pChunkBlock = reinterpret_cast<CChunkBlock<TLS_DATA> *>(TlsGetValue(m_dwTlsIndex));

        // 최초로 Alloc() 호출 될 때
        if (nullptr == pChunkBlock)
            pChunkBlock = reinterpret_cast<CChunkBlock<TLS_DATA> *>(ChunkAlloc());

        TLS_DATA *pReturn = pChunkBlock->Alloc();

        if (m_bReplacementNew)
            new (pReturn) TLS_DATA();

        InterlockedIncrement(&m_lUsingBlockCnt);

        return pReturn;
    }

    template<class TLS_DATA>
    inline void CMemoryPool_TLS<TLS_DATA>::Free(TLS_DATA * pData)
    {
        CChunkBlock<TLS_DATA>::DATA_BLOCK *pDataBlock = reinterpret_cast<CChunkBlock<TLS_DATA>::DATA_BLOCK *>((reinterpret_cast<char *>(pData) - (sizeof(void *) + sizeof(UINT64))));

        if (df_MEMORY_POOL_CHUNK_BLOCK_CHECK != pDataBlock->ui64BlockCheck)
            CRASH();

        InterlockedDecrement(&m_lUsingBlockCnt);

        if (pDataBlock->pChunkBlock->Free(m_bReplacementNew))
            InterlockedDecrement(&m_lUsingChunkCnt);
    }

    template<class TLS_DATA>
    inline typename CMemoryPool_TLS<TLS_DATA>::CChunkBlock<TLS_DATA> * CMemoryPool_TLS<TLS_DATA>::ChunkAlloc(void)
    {
        CChunkBlock<TLS_DATA> *pChunkBlock = reinterpret_cast<CChunkBlock<TLS_DATA> *>(m_spChunkBlockMemoryPool.get()->Alloc());

        InterlockedIncrement(&m_lUsingChunkCnt);

        if (nullptr == pChunkBlock->m_pDataBlock)
        {
            InterlockedIncrement(&m_lChunkCnt);
            pChunkBlock->Constructor(*this, m_lChunkSize);
        }
        else
        {
            pChunkBlock->Initialize(m_lChunkSize);
        }

        TlsSetValue(m_dwTlsIndex, pChunkBlock);

        return pChunkBlock;
    }
}

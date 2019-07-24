/*
MemoryPool_LF���� CHUNK_DATA Ÿ������ �޸𸮸� Alloc 1���� 1���� CHUNK�� �Ҵ�Ǵ� ���̰�,
MemoryPool_TLS������ MemoryPool_LF���� �Ҵ� ���� CHUNK�� ���ϴ� ���� ��ŭ�� �޸� BLOCK�� ���� �Ǵ� ����.
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
            // DATA_BLCOK ����ü�� ���� �� ���� �Ǿ� �ִ� ����
            // DATA_BLOCK 1�� �� ���� ���� CHUNK_DATA�� ����Ǿ� �ִ� ����
            // CChunkBlock<CHUNK_DATA>�� MemoryPool_LF ���� �Ҵ� ��
            // CMemoryPool_TLS���� �Ҵ�� CChunkBlock<CHUNK_DATA>�� ��ȯ
            // �̶�, CChunkBlock<CHUNK_DATA>�� �ּҸ� �ٷ� ��ȯ�ϴ°� �ƴ϶� CHUNK_DATA�� �ּҸ� ��ȯ
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
            // Data �� �Ҵ�
            //
            // - 
            // Chunk�� Data �� �� 1���� �Ҵ�
            //-------------------------------------------------------------------------
            CHUNK_DATA * Alloc(void);

            //-------------------------------------------------------------------------
            // param : void
            // return: void
            //
            // Data �� �Ǵ� Chunk ��ȯ
            //
            // -
            // �ش� �Լ� ȣ�� ��, m_lAllocRef ����.
            // m_lAllocRef == 0 �̸�, Chunk ��ȯ.
            //-------------------------------------------------------------------------
            bool Free(bool bReplacementNew);

            //-------------------------------------------------------------------------
            // param : CMemoryPool_TLS<TLS_DATA> * (Constructor �Լ��� ȣ���� ��ü)
            // return: void
            //
            // DATA_BLOCK �Ҵ��Ͽ� ����
            //
            // -
            // m_pDataBlock�� ���� ���� DATA_BLOCK�� �Ҵ��Ͽ� �����ϱ�.
            //-------------------------------------------------------------------------
            void Constructor(CMemoryPool_TLS<TLS_DATA> & MemoryPool_TLS, const long lChunkSize);

            //-------------------------------------------------------------------------
           // param : long (CHUNK ����)
           // return: void
           //
           // ����� �ʱ�ȭ
           //
           // -
           // CChunkBlock<CHUNK_DATA>�� ����� �ʱ�ȭ
           //-------------------------------------------------------------------------
            void Initialize(long lChunkSize);

        private:
            //------------------------------------------------------------------------
            // Chunk �Ҵ� �� ��ȯ ��, ���� alloc �� MemoryPool_TLS ������
            //------------------------------------------------------------------------
            CMemoryPool_TLS<TLS_DATA> *m_pMemoryPool_TLS;

            size_t m_PointerSize;
            size_t m_BlockCheckDataSize;

            //------------------------------------------------------------------------
            // Data �� ������
            //------------------------------------------------------------------------
            DATA_BLOCK	*m_pDataBlock;
            //------------------------------------------------------------------------
            // Top Data �� (Alloc �� ������ �̵�)
            //------------------------------------------------------------------------
            DATA_BLOCK	*m_pAllocTop;

            //------------------------------------------------------------------------
            // Chunk�� Data �� ����
            //------------------------------------------------------------------------
            long	m_lAllocMax;
            //------------------------------------------------------------------------
            // Free ��, ���� �� Data �� ����
            // 0�� �Ǹ�, Chunk ��ȯ
            //------------------------------------------------------------------------
            long	m_lAllocRef;
        };

    public:
        //-------------------------------------------------------------------------
        // param : long (������ Chunk Data �� ����), bool (replacement ����)
        // return: 
        //
        // ������, �ı���
        //
        // iBlockNum = 0 �̸�, Alloc�� ���� ���� �޸� �Ҵ�
        // iBlockNum != 0 �̸�, �̸� �޸� �Ҵ�
        // -
        // ��� ������ ��� delete
        //-------------------------------------------------------------------------
        explicit CMemoryPool_TLS(const long lChunkSize, const bool bReplacementNew);
        virtual ~CMemoryPool_TLS();

        //-------------------------------------------------------------------------
        // param : void
        // return: TLS_DATA *
        //
        // TLS_DATA * �Ҵ�
        // 
        // -
        // TLS�� ��ϵ� CChunkBlock<CHUNK_DATA> ���� CHUNK_DATA�� ��ȯ
        //-------------------------------------------------------------------------
        TLS_DATA * Alloc(void);

        //-------------------------------------------------------------------------
       // param : TLS_DATA * (��ȯ �� ������)
       // return: void
       //
       // TLS_DATA * ��ȯ
       // 
       // -
       // TLS_DATA *�� CChunkBlock<TLS_DATA> * ���·� ���� ��, CChunkBlock<TLS_DATA>�� Free() �Լ� ȣ��
       // ���� ��ȯ ���δ� CChunkBlock<TLS_DATA>�� Free()���� �Ǵ�
       //-------------------------------------------------------------------------
        void Free(TLS_DATA *pData);

        //-------------------------------------------------------------------------
        // param : void
        // return: CChunkBlock<TLS_DATA> * (��ȯ �� ������)
        //
        // CChunkBlock<TLS_DATA> * �Ҵ�
        // 
        // -
        // m_pChunkBlockMemoryPool�� ���ؼ� CChunkBlock<TLS_DATA>�� �Ҵ�
        //-------------------------------------------------------------------------
        CChunkBlock<TLS_DATA> * ChunkAlloc(void);

        long		GetChunkCount(void) { return m_lChunkCnt; }
        long		GetUsingBlockCount(void) { return m_lUsingBlockCnt; }
        long		GetUsingChunkCount(void) { return m_lUsingChunkCnt; }

    private:
        //------------------------------------------------------------------------
        // CChunkBlock<TLS_DATA>�� �Ҵ����� MemoryPool_LF Ŭ���� ������
        //------------------------------------------------------------------------
        //CMemoryPool_LF<CChunkBlock<TLS_DATA>>		*m_pChunkBlockMemoryPool;
        std::unique_ptr<CMemoryPool_LF<CChunkBlock<TLS_DATA>>> m_spChunkBlockMemoryPool;

        DWORD								m_dwTlsIndex;
        long								m_lChunkCnt;	    // �� ������� Chunk ����
        long								m_lUsingBlockCnt;	// �� ������� DATA ����
        long								m_lUsingChunkCnt;	// �� ������� Chunk ����
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

        // �� �̻� DATA_BLOCK�� ���� ��, �̸� �Ҵ�
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

        // CChunkBlock<CHUNK_DATA>::Alloc() ���� �̸� �Ҵ� ��, ���� ������ ���� Block �޸� ����
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

        // ���ʷ� Alloc() ȣ�� �� ��
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

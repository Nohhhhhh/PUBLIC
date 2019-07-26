#pragma once

#include <new.h>
#include <WinSock2.h>

#include "Dump.h"

namespace NOH
{
    template<class T> using unique_ptr_aligned = std::unique_ptr<T, decltype(&_aligned_free)>;

    template <class DATA>
    class CMemoryPool_LF
    {
    private:

        //------------------------------------------------------------------------
        // 각 블럭 앞에 사용될 노드 구조체.
        //------------------------------------------------------------------------
        typedef struct st_BLOCK_NODE
        {
            st_BLOCK_NODE *pNextNode;

            // 구조체 생성자
            // 구조체 선언과 동시에 생성자가 호출
            st_BLOCK_NODE() : pNextNode(nullptr) {}
        } BLOCK_NODE;

        //------------------------------------------------------------------------
        // 락 프리에 사용될 16byte 노드 구조체.
        //------------------------------------------------------------------------
        typedef struct st_FREE_NODE
        {
            st_BLOCK_NODE	*pFreeNode;
            LONGLONG		llUniqueIdx;

            st_FREE_NODE() : pFreeNode(nullptr), llUniqueIdx(0) {}
            void Initialize(void) { pFreeNode = nullptr, llUniqueIdx = 0; }
        } FREE_NODE;

    public:

        //-------------------------------------------------------------------------
        // param : int (최대 블럭 개수), bool (replacement 여부)
        // return: 
        //
        // 생성자, 파괴자
        //
        // iBlockNum = 0 이면, Alloc을 통해 동적 메모리 할당
        // iBlockNum != 0 이면, 미리 메모리 할당
        // -
        // 모든 생성된 노드 delete
        //-------------------------------------------------------------------------
        explicit CMemoryPool_LF(const int iBlockNum = 0);
        virtual	~CMemoryPool_LF();

        //-------------------------------------------------------------------------
        // param : int (생성할 블럭 개수)
        // return: void
        //
        // 미리 블럭 생성
        //
        // 미리 메모리를 할당해야 하는 경우에 생성자에서 호출
        //-------------------------------------------------------------------------
        void	CreateBlock(const int iBlockNum);

        //-------------------------------------------------------------------------
        // param : void
        // return: DATA * (데이터 블럭 포인터)
        //
        // 메모리 할당
        //
        // Free된 노드가 있으면 반환
        // Free된 노드가 없다면 동적 생성 후 반환
        //-------------------------------------------------------------------------
        DATA	*Alloc(void);

        //-------------------------------------------------------------------------
        // param : DATA * (데이터 블럭 포인터)
        // return: true (항상)
        //
        // 블럭 해제
        //
        // Free된 노드가 있으면 반환
        // Free된 노드가 없다면 동적 생성 후 반환
        //-------------------------------------------------------------------------
        bool	Free(DATA *pData, bool bReplacementNew);

        //-------------------------------------------------------------------------
        // param : void
        // return: long (사용중인 블럭 개수)
        //
        // 사용중인 블럭 개수
        //
        // 생성된 블럭 중 현재 사용중인 블럭의 개수
        //-------------------------------------------------------------------------
        long	GetAllocCount(void) { return m_lAllocCnt; }

        //-------------------------------------------------------------------------
        // param : void
        // return: long (생성된 블럭 개수)
        //
        // 생성된 블럭 개수
        //
        // 생성된 블럭 개수
        //-------------------------------------------------------------------------
        long	GetBlockCount(void) { return m_lBlockCnt; }

    private:
        //------------------------------------------------------------------------
        // 블럭 한 개의 사이즈
        //------------------------------------------------------------------------
        int					m_iOneBlockSize;

        //------------------------------------------------------------------------
        // 현재 가리키는 블럭 노드
        //------------------------------------------------------------------------
        //FREE_NODE	    	*m_pCurFreeNode;
        unique_ptr_aligned<FREE_NODE>        m_spCurFreeNode;
        //------------------------------------------------------------------------
        // 생성된 블럭 개수
        //------------------------------------------------------------------------
        long				m_lBlockCnt;

        //------------------------------------------------------------------------
        // 할당된 블럭 개수
        //------------------------------------------------------------------------
        long				m_lAllocCnt;

        //------------------------------------------------------------------------
        // 유니크 인덱스
        //------------------------------------------------------------------------
        LONGLONG			m_llUniqueIdx;
    };


    template <class DATA>
    inline CMemoryPool_LF<DATA>::CMemoryPool_LF(int iBlockNum)
        : m_lBlockCnt(0), m_lAllocCnt(0), m_llUniqueIdx(0), m_iOneBlockSize(sizeof(BLOCK_NODE) + sizeof(DATA)), m_spCurFreeNode(static_cast<FREE_NODE *>( _aligned_malloc( sizeof(FREE_NODE), 16 )), &_aligned_free)/*, m_pCurFreeNode(reinterpret_cast<FREE_NODE *>(_aligned_malloc(sizeof(FREE_NODE), 16)))*/
    {
        m_spCurFreeNode.get()->Initialize();

        if (0 >= iBlockNum)
            return;

        CreateBlock(iBlockNum);
    }

    template <class DATA>
    inline CMemoryPool_LF<DATA>::~CMemoryPool_LF()
    {
        BLOCK_NODE *_pcurnode = m_spCurFreeNode.get()->pFreeNode;
        BLOCK_NODE *_pdeletenode = nullptr;

        if (nullptr == _pcurnode)
            return;

        while (1)
        {
            _pdeletenode = _pcurnode;
            _pcurnode = _pcurnode->pNextNode;

            free(_pdeletenode);
            _pdeletenode = nullptr;

            if ((0 == InterlockedDecrement(&m_lBlockCnt)) && (nullptr == _pcurnode))
                break;
        }
    }

    template<class DATA>
    inline void CMemoryPool_LF<DATA>::CreateBlock(const int iBlockNum)
    {
        char *_cpnewblock = nullptr;
        FREE_NODE * _pstoredfreenode = nullptr;

        for (int iCnt = 0; iCnt < iBlockNum; ++iCnt)
        {
            _cpnewblock = nullptr;
            _pstoredfreenode = m_spCurFreeNode.get();
            // 첫 할당
            if (nullptr == _pstoredfreenode->pFreeNode)
            {
                _cpnewblock = reinterpret_cast<char *>(malloc(m_iOneBlockSize));
                memset(_cpnewblock, 0, m_iOneBlockSize);

                _pstoredfreenode->pFreeNode = reinterpret_cast<BLOCK_NODE *>(_cpnewblock);
                _pstoredfreenode->pFreeNode->pNextNode = nullptr;
            }
            else
            {
                _cpnewblock = (char *)malloc(m_iOneBlockSize);
                memset(_cpnewblock, 0, m_iOneBlockSize);

                (reinterpret_cast<BLOCK_NODE *>(_cpnewblock))->pNextNode = _pstoredfreenode->pFreeNode;
                _pstoredfreenode->pFreeNode = reinterpret_cast<BLOCK_NODE *>(_cpnewblock);
            }

            InterlockedIncrement(&m_lBlockCnt);
        }
    }

    template <class DATA>
    inline DATA* CMemoryPool_LF<DATA>::Alloc()
    {
        FREE_NODE	    _curfreenode;
        BLOCK_NODE	    *_preturnnode = nullptr;
        bool			_breplacement = false;
        long			_ltempblockcnt = m_lBlockCnt;

        if (InterlockedIncrement(&m_lAllocCnt) > m_lBlockCnt)
        {
            // 동적할당 모드
            char *cpNewBlock = reinterpret_cast<char *>(malloc(m_iOneBlockSize));
            memset(cpNewBlock, 0, m_iOneBlockSize);
            _preturnnode = reinterpret_cast<BLOCK_NODE *>(cpNewBlock);
            _breplacement = true;
            InterlockedIncrement(&m_lBlockCnt);

        }
        else
        {
            LONGLONG _lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);

            while (1)
            {
                _curfreenode.pFreeNode = m_spCurFreeNode.get()->pFreeNode;
                _curfreenode.llUniqueIdx = m_spCurFreeNode.get()->llUniqueIdx;
                _preturnnode = _curfreenode.pFreeNode;

                if ((nullptr != _curfreenode.pFreeNode) && (1 == InterlockedCompareExchange128(reinterpret_cast<volatile LONG64 *>(m_spCurFreeNode.get()), _lluniqueidx, reinterpret_cast<LONG64>(_curfreenode.pFreeNode->pNextNode), reinterpret_cast<LONG64 *>(&_curfreenode))))
                    break;
            }
        }

        DATA *pData = reinterpret_cast<DATA *>(_preturnnode + 1);

        // replacement new
        if (_breplacement)
            new ((DATA *)pData) DATA();

        return pData;
    }

    template <class DATA>
    inline bool CMemoryPool_LF<DATA>::Free(DATA *pData, bool bReplacementNew)
    {
        if (m_lAllocCnt <= 0 || nullptr == pData)
            CRASH();

        if (bReplacementNew)
            pData->~DATA();

        FREE_NODE	_curfreenode;
        FREE_NODE	_newfreenode;
        LONGLONG	_lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);

        BLOCK_NODE *_pnode = reinterpret_cast<BLOCK_NODE *>(reinterpret_cast<char *>(pData) - sizeof(BLOCK_NODE));

        while (1)
        {
            _curfreenode.pFreeNode = m_spCurFreeNode.get()->pFreeNode;
            _curfreenode.llUniqueIdx = m_spCurFreeNode.get()->llUniqueIdx;

            _newfreenode.pFreeNode = _pnode;
            _newfreenode.llUniqueIdx = _lluniqueidx;
            _pnode->pNextNode = _curfreenode.pFreeNode;

            if (1 == InterlockedCompareExchange128(reinterpret_cast<volatile LONG64 *>(m_spCurFreeNode.get()), _newfreenode.llUniqueIdx, reinterpret_cast<LONG64>(_newfreenode.pFreeNode), reinterpret_cast<LONG64 *>(&_curfreenode)))
                break;
        }

        InterlockedDecrement(&m_lAllocCnt);

        return true;
    }
}
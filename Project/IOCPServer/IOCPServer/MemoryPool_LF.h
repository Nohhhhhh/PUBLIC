/*---------------------------------------------------------------

MemoryPool.

�޸� Ǯ Ŭ����.
Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.

- ����.

CMemoryPool_LF<DATA> MemPool(300, FALSE);
DATA *pData = MemPool.Alloc();

pData ���

MemPool.Free(pData);


!.	���� ���� ���Ǿ� �ӵ��� ������ �� �޸𸮶�� �����ڿ���
Lock �÷��׸� �־� ����¡ ���Ϸ� ���縦 ���� �� �ִ�.
���� �߿��� ��찡 �ƴ��̻� ��� ����.



���ǻ��� :	�ܼ��� �޸� ������� ����Ͽ� �޸𸮸� �Ҵ��� �޸� ����� �����Ͽ� �ش�.
Ŭ������ ����ϴ� ��� Ŭ������ ������ ȣ�� �� Ŭ�������� �Ҵ��� ���� ���Ѵ�.
Ŭ������ �����Լ�, ��Ӱ��谡 ���� �̷����� �ʴ´�.
----------------------------------------------------------------*/
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
        // �� �� �տ� ���� ��� ����ü.
        //------------------------------------------------------------------------
        typedef struct st_BLOCK_NODE
        {
            st_BLOCK_NODE *pNextNode;

            // ����ü ������
            // ����ü ����� ���ÿ� �����ڰ� ȣ��
            st_BLOCK_NODE() : pNextNode(nullptr) {}
        } BLOCK_NODE;

        //------------------------------------------------------------------------
        // �� ������ ���� 16byte ��� ����ü.
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
        // param : int (�ִ� �� ����), bool (replacement ����)
        // return: 
        //
        // ������, �ı���
        //
        // iBlockNum = 0 �̸�, Alloc�� ���� ���� �޸� �Ҵ�
        // iBlockNum != 0 �̸�, �̸� �޸� �Ҵ�
        // -
        // ��� ������ ��� delete
        //-------------------------------------------------------------------------
        explicit CMemoryPool_LF(const int iBlockNum = 0);
        virtual	~CMemoryPool_LF();

        //-------------------------------------------------------------------------
        // param : int (������ �� ����)
        // return: void
        //
        // �̸� �� ����
        //
        // �̸� �޸𸮸� �Ҵ��ؾ� �ϴ� ��쿡 �����ڿ��� ȣ��
        //-------------------------------------------------------------------------
        void	CreateBlock(const int iBlockNum);

        //-------------------------------------------------------------------------
        // param : void
        // return: DATA * (������ �� ������)
        //
        // �޸� �Ҵ�
        //
        // Free�� ��尡 ������ ��ȯ
        // Free�� ��尡 ���ٸ� ���� ���� �� ��ȯ
        //-------------------------------------------------------------------------
        DATA	*Alloc(void);

        //-------------------------------------------------------------------------
        // param : DATA * (������ �� ������)
        // return: true (�׻�)
        //
        // �� ����
        //
        // Free�� ��尡 ������ ��ȯ
        // Free�� ��尡 ���ٸ� ���� ���� �� ��ȯ
        //-------------------------------------------------------------------------
        bool	Free(DATA *pData, bool bReplacementNew);

        //-------------------------------------------------------------------------
        // param : void
        // return: long (������� �� ����)
        //
        // ������� �� ����
        //
        // ������ �� �� ���� ������� ���� ����
        //-------------------------------------------------------------------------
        long	GetAllocCount(void) { return m_lAllocCnt; }

        //-------------------------------------------------------------------------
        // param : void
        // return: long (������ �� ����)
        //
        // ������ �� ����
        //
        // ������ �� ����
        //-------------------------------------------------------------------------
        long	GetBlockCount(void) { return m_lBlockCnt; }

    private:
        //------------------------------------------------------------------------
        // �� �� ���� ������
        //------------------------------------------------------------------------
        int					m_iOneBlockSize;

        //------------------------------------------------------------------------
        // ���� ����Ű�� �� ���
        //------------------------------------------------------------------------
        //FREE_NODE	    	*m_pCurFreeNode;
        unique_ptr_aligned<FREE_NODE>        m_spCurFreeNode;
        //------------------------------------------------------------------------
        // ������ �� ����
        //------------------------------------------------------------------------
        long				m_lBlockCnt;

        //------------------------------------------------------------------------
        // �Ҵ�� �� ����
        //------------------------------------------------------------------------
        long				m_lAllocCnt;

        //------------------------------------------------------------------------
        // ����ũ �ε���
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
            // ù �Ҵ�
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

                // ���Ӱ� �Ҵ�� �޸��� next�� m_pCurrentNode
                // m_pCurrentNode�� ����� �޸𸮸� m_pCurrentNode�� ����Ű�� ��
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

        // ������ �� ������ �Ҵ�� �� ������ �����ϴٸ�, �����Ҵ�
        // ������ �� ������ �Ҵ�� �� ������ �����ϴٸ�, �����Ҵ�
        // m_lBlockCnt�� �������� ���� ��, ���ϴ� ���� :
        // Alloc 1 : 5 / 5 �� ���� -> m_lAllocCnt++ �Ǽ� 6���, m_lBlockCnt�� ���ϱ� ����
        // Alloc 2 : 6 / 5 �� ���� -> m_lAllocCnt++ �Ǽ� 7���, �Ʒ����� m_lBlockCnt++ �Ǽ� 6�� ��.
        // ���� Alloc 1���� m_lBlockCnt�� 6���� �Ǵ��ϱ� ������ �Ʒ� �������� ����.
        // �׷��Ƿ� goto ������ ������ ������ �߻�. ������ �����Ͱ� ���� ������ �����Ҵ��ؾ� �ϴ´�,
        // ����ȭ ������ �����Ͱ� �ִٰ� �Ǵ��ع���.
        // �ذ� ������δ� blockcnt�� �������� ���� ��, ���� �ϰų� goto ���� ����ϸ� ��
        if (InterlockedIncrement(&m_lAllocCnt) > m_lBlockCnt)
        {
            // �����Ҵ� ���
            char *cpNewBlock = reinterpret_cast<char *>(malloc(m_iOneBlockSize));
            memset(cpNewBlock, 0, m_iOneBlockSize);
            _preturnnode = reinterpret_cast<BLOCK_NODE *>(cpNewBlock);
            _breplacement = true;
            InterlockedIncrement(&m_lBlockCnt);

        }
        // ������ �� ������ �Ҵ�� �� �������� ũ�� freenode ��ȯ
        else
        {
            LONGLONG _lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);

            while (1)
            {
                _curfreenode.pFreeNode = m_spCurFreeNode.get()->pFreeNode;
                _curfreenode.llUniqueIdx = m_spCurFreeNode.get()->llUniqueIdx;
                _preturnnode = _curfreenode.pFreeNode;

                // �̸� �Ҵ�� ���� ��ȯ����� �ϴ´� pFreeNode�� nullptr�̶�� ����
                // ������ �����ٴ� ���̴ϱ�
                // �̷��� ��Ȳ�� ���ͼ� �ȵȴ�.
                if (nullptr == _curfreenode.pFreeNode)
                    CRASH();

                if ((nullptr != _curfreenode.pFreeNode) && (1 == InterlockedCompareExchange128(reinterpret_cast<volatile LONG64 *>(m_spCurFreeNode.get()), _lluniqueidx, reinterpret_cast<LONG64>(_curfreenode.pFreeNode->pNextNode), reinterpret_cast<LONG64 *>(&_curfreenode))))
                    break;
            }
        }

        DATA *pData = reinterpret_cast<DATA *>(_preturnnode + 1);

        // replacement new
        // new (�޸� Ǯ�κ��� ���� �޸� ������) Ŭ����������;
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

            // CurFreeNode�� NewFreeNode�� next�� ����
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
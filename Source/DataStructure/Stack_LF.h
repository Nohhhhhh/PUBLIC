#pragma once

#include "MemoryPool_TLS.h"

namespace NOH
{
	template <class DATA>
	class CStack_LF
	{
	private:
		//------------------------------------------------------------------------
		// �� �� �տ� ���� ��� ����ü.
		//------------------------------------------------------------------------
		typedef struct st_STACK_NODE
		{
	    	// DATA�� ����ü �������̹Ƿ� �����δ� �����͸� �����ϰ� �ȴ�.
			DATA			Data;
			st_STACK_NODE	*pNextNode;

            st_STACK_NODE() : Data(0), pNextNode(nullptr) {}
		} STACK_NODE;

		//------------------------------------------------------------------------
		// �� ������ ���� 16byte ��� ����ü.
		//------------------------------------------------------------------------
		typedef struct st_TOP_NODE
		{
			STACK_NODE	*pTopNode;
			LONGLONG	llUniqueIdx;

            st_TOP_NODE() : pTopNode(nullptr), llUniqueIdx(0) {}
            void Initialize(void) { pTopNode = nullptr; llUniqueIdx = 0; }
		} TOP_NODE;

	public:

		//-------------------------------------------------------------------------
		// param : long (DATA BLOCK ����)
		// return: 
		//
		// ������, �ı���
		//
		// iBlockNum != 0 �̸�, ��� MemroyPool�� ������ŭ �̸� �Ҵ��� ����
		// -
		// ��� ������ ��� delete
		//-------------------------------------------------------------------------
		explicit CStack_LF(const long lChunkSize = 0);
		virtual	~CStack_LF();

		//-------------------------------------------------------------------------
		// param : void
		// return: DATA * (������ �� ������)
		//
		// ���ÿ��� �̱�
		//
		// ���ÿ� ���� ����Ʈ �� Top�� ����
		//-------------------------------------------------------------------------
		bool	Get(DATA * pData);

		//-------------------------------------------------------------------------
		// param : DATA * (������ �� ������)
		// return: true (�׻�)
		//
		// ���ÿ� �ױ�
		//
		// ���ÿ� ������ �ױ�
		//-------------------------------------------------------------------------
		bool	Put(const DATA Data);

		//-------------------------------------------------------------------------
		// param : void
		// return: long (�׿� �ִ� ���� ����)
		//
		// �׿� �ִ� ���� ����
		//
		// ���� ���ÿ� �׿� �ִ� ���� ����
		//-------------------------------------------------------------------------
		long	GetStackCount(void) { return m_lStackCnt; }

	private:
		//------------------------------------------------------------------------
		// ���� ����Ű�� �� ���
		//------------------------------------------------------------------------
        unique_ptr_aligned<TOP_NODE>        m_spCurTopNode;

		//------------------------------------------------------------------------
		// ���� �� ����
		//------------------------------------------------------------------------
		long								m_lStackCnt;

		//------------------------------------------------------------------------
		// ����ũ �ε���
		//------------------------------------------------------------------------
		LONGLONG							m_llUniqueIdx;

        std::unique_ptr<CMemoryPool_TLS<STACK_NODE>>    m_spMemoryPool;
	};
}

template <class DATA>
inline NOH::CStack_LF<DATA>::CStack_LF(long lChunkSize) : m_lStackCnt(0), m_llUniqueIdx(0), m_spMemoryPool(std::make_unique<CMemoryPool_TLS<STACK_NODE>>(lChunkSize, false)), /*m_pMemoryPool(new CMemoryPool_TLS<STACK_NODE>(lChunkSize, false)),*/ m_spCurTopNode(static_cast<TOP_NODE *>( _aligned_malloc( sizeof(TOP_NODE), 16 )), &_aligned_free)
{
    m_spCurTopNode.get()->Initialize();
}

template <class DATA>
inline NOH::CStack_LF<DATA>::~CStack_LF()
{
    STACK_NODE *_pcurnode = m_spCurTopNode.get()->pTopNode;
	STACK_NODE *_pdeletenode = nullptr;

	while (1)
	{
		if (nullptr == _pcurnode)
			break;

		_pdeletenode = _pcurnode;
		_pcurnode = _pcurnode->pNextNode;

        m_spMemoryPool.get()->Free(_pdeletenode);
		if (0 == InterlockedDecrement(&m_lStackCnt))
			break;
	}
}

template <class DATA>
inline bool NOH::CStack_LF<DATA>::Get(DATA * pData)
{
	if (m_lStackCnt <= 0)
		return false;

	TOP_NODE	_curtopnode;
	STACK_NODE	*_preturnnode = nullptr;
	LONGLONG	_lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);

	while (1)
	{
		_curtopnode.pTopNode = m_spCurTopNode.get()->pTopNode;
		_curtopnode.llUniqueIdx = m_spCurTopNode.get()->llUniqueIdx;
		_preturnnode = _curtopnode.pTopNode;

		if (nullptr == _curtopnode.pTopNode)
			continue;

		if (1 == InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spCurTopNode.get() ), _lluniqueidx, reinterpret_cast<LONG64>( _curtopnode.pTopNode->pNextNode ), reinterpret_cast<LONG64 *>( &_curtopnode ) ) )
			break;
	}

	*pData = _preturnnode->Data;

    m_spMemoryPool.get()->Free(_preturnnode);
	InterlockedDecrement(&m_lStackCnt);

	return true;
}

template <class DATA>
inline bool NOH::CStack_LF<DATA>::Put(const DATA Data)
{
	LONGLONG		_lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);
	TOP_NODE		_curtopnode;
	TOP_NODE		_newtopnode;

	InterlockedIncrement(&m_lStackCnt);

    _newtopnode.pTopNode = m_spMemoryPool.get()->Alloc();

	if (nullptr == _newtopnode.pTopNode)
	{
		InterlockedDecrement(&m_lStackCnt);
		return false;
	}

	while (1)
	{
		_curtopnode.pTopNode = m_spCurTopNode.get()->pTopNode;
		_curtopnode.llUniqueIdx = m_spCurTopNode.get()->llUniqueIdx;

		_newtopnode.llUniqueIdx = _lluniqueidx;
		_newtopnode.pTopNode->Data = Data;
		_newtopnode.pTopNode->pNextNode = _curtopnode.pTopNode;

		if (1 == InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spCurTopNode.get() ), _newtopnode.llUniqueIdx, reinterpret_cast<LONG64>( _newtopnode.pTopNode ), reinterpret_cast<LONG64 *>( &_curtopnode) ) )
			break;
	}

	return true;
}
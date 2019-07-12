/*---------------------------------------------------------------

Stack.

구조 : 멤버의 메모리 풀에서 할당 받은 것을 사용자에게 사용하도록 한 후,
다시 반환될 때 Stack에 쌓이는 구조이다.

주의사항 : 해당 Stack은 메모리 풀에서 할당받은 포인터를 관리하는 것이다.
따라서, stack 선언은 CStack_LF<구조체 포인터> Stack(300); 과 같이 사용해야 한다.
즉, 템플릿의 DATA 자체가 포인터 형이 되는 것이다.

----------------------------------------------------------------*/
#pragma once

#include "MemoryPool_TLS.h"

namespace NOH
{
	template <class DATA>
	class CStack_LF
	{
	private:
		//------------------------------------------------------------------------
		// 각 블럭 앞에 사용될 노드 구조체.
		//------------------------------------------------------------------------
		typedef struct st_STACK_NODE
		{
	    	// DATA는 구조체 포인터이므로 실제로는 포인터를 관리하게 된다.
			DATA			Data;
			st_STACK_NODE	*pNextNode;

            st_STACK_NODE() : Data(0), pNextNode(nullptr) {}
		} STACK_NODE;

		//------------------------------------------------------------------------
		// 락 프리에 사용될 16byte 노드 구조체.
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
		// param : long (DATA BLOCK 개수)
		// return: 
		//
		// 생성자, 파괴자
		//
		// iBlockNum != 0 이면, 멤버 MemroyPool을 개수만큼 미리 할당해 놓음
		// -
		// 모든 생성된 노드 delete
		//-------------------------------------------------------------------------
		explicit CStack_LF(const long lChunkSize = 0);
		virtual	~CStack_LF();

		//-------------------------------------------------------------------------
		// param : void
		// return: DATA * (데이터 블럭 포인터)
		//
		// 스택에서 뽑기
		//
		// 스택에 쌓인 리스트 중 Top을 전달
		//-------------------------------------------------------------------------
		bool	Get(DATA * pData);

		//-------------------------------------------------------------------------
		// param : DATA * (데이터 블럭 포인터)
		// return: true (항상)
		//
		// 스택에 쌓기
		//
		// 스택에 데이터 쌓기
		//-------------------------------------------------------------------------
		bool	Put(const DATA Data);

		//-------------------------------------------------------------------------
		// param : void
		// return: long (쌓여 있는 블럭의 개수)
		//
		// 쌓여 있는 블럭의 개수
		//
		// 현재 스택에 쌓여 있는 블럭의 개수
		//-------------------------------------------------------------------------
		long	GetStackCount(void) { return m_lStackCnt; }

	private:
		//------------------------------------------------------------------------
		// 현재 가리키는 블럭 노드
		//------------------------------------------------------------------------
        unique_ptr_aligned<TOP_NODE>        m_spCurTopNode;

		//------------------------------------------------------------------------
		// 스택 블럭 개수
		//------------------------------------------------------------------------
		long								m_lStackCnt;

		//------------------------------------------------------------------------
		// 유니크 인덱스
		//------------------------------------------------------------------------
		LONGLONG							m_llUniqueIdx;

		//CMemoryPool_TLS<STACK_NODE>	        *m_pMemoryPool;
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

		// tls
		//m_pMemoryPool->Free(_pdeletenode);
        m_spMemoryPool.get()->Free(_pdeletenode);
		if (0 == InterlockedDecrement(&m_lStackCnt))
			break;
	}

	//delete m_pMemoryPool;
    //m_pMemoryPool = nullptr;
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

		// 미리 할당된 블럭을 반환해줘야 하는대 pTopNode가 nullptr이라는 것은
		// 누군가 빼갓다는 것이니까
		if (nullptr == _curtopnode.pTopNode)
			continue;

		if (1 == InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spCurTopNode.get() ), _lluniqueidx, reinterpret_cast<LONG64>( _curtopnode.pTopNode->pNextNode ), reinterpret_cast<LONG64 *>( &_curtopnode ) ) )
			break;
	}

	*pData = _preturnnode->Data;

	// 리스트에서 빠져야 하므로 Free
	// tls
	//m_pMemoryPool->Free(_preturnnode);
    m_spMemoryPool.get()->Free(_preturnnode);
	InterlockedDecrement(&m_lStackCnt);

	return true;
}

template <class DATA>
inline bool NOH::CStack_LF<DATA>::Put(const DATA Data)
{
	// lock free할 때는 메모리 풀을 memset하면 안된다.
	LONGLONG		_lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);
	TOP_NODE		_curtopnode;
	TOP_NODE		_newtopnode;

	InterlockedIncrement(&m_lStackCnt);

	//_newtopnode.pTopNode = m_pMemoryPool->Alloc();
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

		// CurFreeNode를 NewFreeNode의 next로 연결
		_newtopnode.llUniqueIdx = _lluniqueidx;
		// 템플릿 타입인 DATA는 구조체 *Data 이므로, 포인터가 저장이 되는 것임.
		_newtopnode.pTopNode->Data = Data;
		_newtopnode.pTopNode->pNextNode = _curtopnode.pTopNode;

		if (1 == InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spCurTopNode.get() ), _newtopnode.llUniqueIdx, reinterpret_cast<LONG64>( _newtopnode.pTopNode ), reinterpret_cast<LONG64 *>( &_curtopnode) ) )
			break;
	}

	return true;
}
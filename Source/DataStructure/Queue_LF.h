#pragma once

#include "MemoryPool_TLS.h"

#define dfNODESIZE					16
#define dfQUEUE_LF_CHUNK_MAX_SIZE	300

namespace NOH
{
	template <class DATA>
	class CQueue_LF
	{
	public:
		typedef struct st_NODE
		{
			DATA			Data;
			st_NODE			*pNextNode;

            st_NODE(void) : Data(nullptr), pNextNode(nullptr) {}
            void Initialize(void) { Data = reinterpret_cast<DATA>( 0x7fffffffffffffff ); pNextNode = nullptr; }
		} NODE;

		typedef struct st_POSITION_NODE
		{
			NODE		*pCurNode;          // low
			LONGLONG	llUniqueIdx;        // high

            st_POSITION_NODE(void) : pCurNode(nullptr), llUniqueIdx(0) {}
            void Initialize(NODE & _CurNode, const LONGLONG _llUniqueIdx) { pCurNode = &_CurNode; llUniqueIdx = _llUniqueIdx; }
		} POSITION_NODE;

	public:

		//------------------------------------------------------
		// param  : void
		// return : none
		// param  : void
		// return : none
		//
		// 생성자 & 소멸자
		//------------------------------------------------------
		CQueue_LF(void);
		virtual	~CQueue_LF(void);

		//------------------------------------------------------
		// param  : void
		// return : void
		//
		// 버퍼 및 버퍼 사이즈 초기화
		//------------------------------------------------------
		void		Initialize(void);

		//------------------------------------------------------
		// param  : void
		// return : long
		//
		// 노드 개수 구하기
		//------------------------------------------------------
		long		GetNodeCount(void);

		//------------------------------------------------------
		// param  : DATA (넣을 데이터 포인터)
		// return : true (성공)
		//
		// 데이터 넣기
		//------------------------------------------------------
		bool		Enqueue(const DATA Data);

		//------------------------------------------------------
		// param  : DATA * (데이터 담을 포인터)
		// return : int
		//
		// 데이터 뽑기
		//------------------------------------------------------
		bool		Dequeue(DATA *pData);

		//------------------------------------------------------------
		// param  : DATA *(데이터 담을 포인터), int (읽을 노드 개수)
		// return : void
		//
		// 데이터 뽑기
		//
		// 단, 포지션 이동 없음
		// -
		// 11/09
		// peek을 할 때 Enqueue을 막지 않는 이상 안전하지 않음.
		// 즉 락을 걸지 않으면 동기화가 불가능.
		// 하지만 WSASend를 완료통지 한 번에 한번 씩만 걸기 때문에 
		// 현재는 안전하게 사용가능하다고 믿고 가야 함.
		//------------------------------------------------------------
		bool		Peek(DATA *pData, int iNodeNum = 0);

		//------------------------------------------------------------
		// param  : void
		// return : void
		//
		// 모든 노드 삭제.
		//------------------------------------------------------------
		void	ClearQueue(void);

	public:
		//------------------------------------------------------------
		// 버퍼의 헤더, 테일
		//------------------------------------------------------------
		//POSITION_NODE		*m_pHead;
		//POSITION_NODE		*m_pTail;
        unique_ptr_aligned<POSITION_NODE> m_spHead;
        unique_ptr_aligned<POSITION_NODE> m_spTail;

		//------------------------------------------------------------
		// 비교 인덱스
		//------------------------------------------------------------
		INT64			m_llUniqueIdx;

		//------------------------------------------------------------
		// 노드 개수
		//------------------------------------------------------------
		long							m_lNodeCnt;
		
		static CMemoryPool_LF<NODE>			*m_pMemoryPool;
	};
		
	template<class DATA>
	inline CQueue_LF<DATA>::CQueue_LF(void) 
        : m_llUniqueIdx(0), m_lNodeCnt(0), m_spHead(static_cast<POSITION_NODE *>( _aligned_malloc( sizeof(POSITION_NODE), 16 )), &_aligned_free), m_spTail(static_cast<POSITION_NODE *>( _aligned_malloc( sizeof(POSITION_NODE), 16 )), &_aligned_free)/*, m_pHead(nullptr), m_pTail(nullptr)*/
	{
		Initialize();
	}

	template<class DATA>
	inline CQueue_LF<DATA>::~CQueue_LF(void)
	{
		
	}

	template<class DATA>
	inline void CQueue_LF<DATA>::Initialize(void)
	{
		// lf
		if (nullptr == m_pMemoryPool)
			m_pMemoryPool = new CMemoryPool_LF<NODE>();
			
		NODE *_pdummy = m_pMemoryPool->Alloc();
        _pdummy->Initialize();

        m_spHead->Initialize(*_pdummy, 0);
        m_spTail.get()->Initialize(*_pdummy, 0);
	}

	template<class DATA>
	inline long CQueue_LF<DATA>::GetNodeCount(void)
	{
		return m_lNodeCnt;
	}

	template<class DATA>
	inline bool CQueue_LF<DATA>::Enqueue(const DATA Data)
	{
		POSITION_NODE _checktail;
		NODE *_pnextnode = nullptr;
		LONGLONG _lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);

		NODE *_pnewnode = m_pMemoryPool->Alloc();
		_pnewnode->Data = Data;

		while (1)
		{
			_checktail.pCurNode = m_spTail.get()->pCurNode;
			_checktail.llUniqueIdx = m_spTail.get()->llUniqueIdx;
			_pnextnode = _checktail.pCurNode->pNextNode;

			if (_checktail.pCurNode == m_spTail.get()->pCurNode)
			{ 
				if (_pnextnode == nullptr)
				{
					_pnewnode->pNextNode = nullptr;

					if (nullptr == InterlockedCompareExchangePointer( reinterpret_cast<volatile PVOID *>( &_checktail.pCurNode->pNextNode ), reinterpret_cast<PVOID>( _pnewnode ), reinterpret_cast<PVOID>( _pnextnode) ) )
						break;
				}
				else
				{
					InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spTail.get() ), _lluniqueidx, reinterpret_cast<LONG64>( _pnextnode ), reinterpret_cast<LONG64 *>( &_checktail) );
				}
			}
		}

		InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spTail.get() ), _lluniqueidx, reinterpret_cast<LONG64>( _pnewnode ), reinterpret_cast<LONG64 *>( &_checktail) );

		InterlockedIncrement(&m_lNodeCnt);

		return true;
	}

	template<class DATA>
	inline bool CQueue_LF<DATA>::Dequeue(DATA *pData)
	{
		if (InterlockedDecrement(&m_lNodeCnt) < 0)
		{
			InterlockedIncrement(&m_lNodeCnt);
			return false;
		}

		POSITION_NODE _checkhead;
		POSITION_NODE _checktail;
		NODE *_pnextnode = nullptr;
		LONGLONG _lluniqueidx = InterlockedIncrement64(&m_llUniqueIdx);

		while (1)
		{
			_checkhead.pCurNode = m_spHead.get()->pCurNode;
			_checkhead.llUniqueIdx = m_spHead.get()->llUniqueIdx;
			_pnextnode = _checkhead.pCurNode->pNextNode;

			if (nullptr == _pnextnode)
				continue;

			if (_checkhead.pCurNode == m_spTail.get()->pCurNode)
			{
				_checktail.pCurNode = m_spTail.get()->pCurNode;
				_checktail.llUniqueIdx = m_spTail.get()->llUniqueIdx;
				_pnextnode = _checktail.pCurNode->pNextNode;

				if (nullptr != _pnextnode)
					InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spTail.get() ), _lluniqueidx, reinterpret_cast<LONG64>( _pnextnode ), reinterpret_cast<LONG64 *>( &_checktail ) );
			}
			else
			{
				*pData = _pnextnode->Data;

				if (1 == InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spHead.get() ), _lluniqueidx, reinterpret_cast<LONG64>( _pnextnode ), reinterpret_cast<LONG64 *>( &_checkhead) ) )
					break;
			}
		}

		// lf
		m_pMemoryPool->Free(_checkhead.pCurNode, true);

		return true;
	}

	template<class DATA>
	inline bool CQueue_LF<DATA>::Peek(DATA *pData, int iNodeNum)
	{
		if (iNodeNum >= m_lNodeCnt)
			return false;

		NODE *_ptemphead;
		NODE *_pcheckhead;
		NODE *_pnextnode = nullptr;

		_ptemphead = m_spHead.get()->pCurNode;
		_pcheckhead = _ptemphead;
	
		while (1)
		{
			_pnextnode = _ptemphead->pNextNode;

			if (nullptr == _pnextnode)
				return false;

			--iNodeNum;

			if (-1 == iNodeNum)
			{
				if (_pcheckhead != m_spHead.get()->pCurNode)
					return false;

				*pData = _pnextnode->Data;

				break;
			}
				
			_ptemphead = _ptemphead->pNextNode;

			if(nullptr == _ptemphead)
				return false;
		}
	
		return true;
	}

	template<class DATA>
	inline void CQueue_LF<DATA>::ClearQueue()
	{
		NODE *_ptemphead = m_spHead.get()->pCurNode;
		NODE *_ptempdelete = nullptr;

		while (1)
		{
			_ptempdelete = _ptemphead->pNextNode;
			_ptemphead = _ptemphead->pNextNode;

			if (nullptr == _ptempdelete)
				break;

			// lf
			m_pMemoryPool->Free(_ptempdelete, true);

			_ptempdelete = nullptr;
		}

		m_spHead.get()->llUniqueIdx = 0;
		m_spHead.get()->pCurNode->Data = reinterpret_cast<DATA>( 0xffffffffffffffff );
		m_spHead.get()->pCurNode->pNextNode = nullptr;

		m_spTail.get()->llUniqueIdx = 0;
		m_spTail.get()->pCurNode = m_spHead.get()->pCurNode;
		m_spTail.get()->pCurNode->pNextNode = nullptr;

		InterlockedExchange(&m_lNodeCnt, 0);
	}

	// lf
	template<class DATA> CMemoryPool_LF<typename CQueue_LF<DATA>::NODE> * CQueue_LF<DATA>::m_pMemoryPool;
}
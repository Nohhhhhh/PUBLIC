/*
락프리큐는 메모리풀이 동적할당하는 것이면 락프리큐의 사이즈도 무한으로 늘어남.
따라서 락프리큐도 동적으로 가야 한다.

Peek은 단독으로 사용되도록 막아줘야 한다. 외부에서 사용하는 구조자체에서


릴리즈가 오면, 큐의 노드 전부 free해주기만 하면됨.
세션배열을 재사용 하는 것이니까.
서버가 진짜 꺼질 때만 메모리풀의 노드는 파괴자에서 파괴해야 한다.

peek을 할 때 get을 막지 않는 이상 안전하지 않는다.
애초에 락을 걸지 않으면 동기화가 불가능 하다.
하지만 send를 한번 씩만 걸기 때문에 안전하게 사용가능하다고 믿고 가야 한다.
*/

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
		
		//static CMemoryPool_TLS<NODE>	*m_pMemoryPool;
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
		// tls
		/*if (nullptr == m_pMemoryPool)
			m_pMemoryPool = new CMemoryPool_TLS<NODE>(dfQUEUE_LF_CHUNK_MAX_SIZE, false);*/
		
		// lf
		if (nullptr == m_pMemoryPool)
			m_pMemoryPool = new CMemoryPool_LF<NODE>();
			
		NODE *_pdummy = m_pMemoryPool->Alloc();
		//NODE *pDummy = new NODE;
        _pdummy->Initialize();
		//pDummy->Data = reinterpret_cast<DATA>( 0x7fffffffffffffff );
		//pDummy->pNextNode = nullptr;

        m_spHead->Initialize(*_pdummy, 0);
        m_spTail.get()->Initialize(*_pdummy, 0);

		//m_pHead = reinterpret_cast<POSITION_NODE *>( _aligned_malloc( dfNODESIZE, 16 ) );
		//m_pHead->pCurNode = pDummy;
		//m_pHead->llUniqueIdx = 0;
        //
		//m_pTail = reinterpret_cast<POSITION_NODE *>( _aligned_malloc( dfNODESIZE, 16 ) );
		//m_pTail->pCurNode = pDummy;
		//m_pTail->llUniqueIdx = 0;
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
			// 지역변수에 저장
			// m_pTail의 pCurNode가 가리키는 주소 값을 넘긴 것이니까 m_pTail이 바뀌어도 
			// 바뀌지 전의 m_pTail의 pCurNode가 가리키는 node의 주소 값이 바뀔 일은 없다.
			// 따라서 지역 pCheckTail의 pCurNode가 가리키는 node는 바뀌지 않는다.
			_checktail.pCurNode = m_spTail.get()->pCurNode;
			_checktail.llUniqueIdx = m_spTail.get()->llUniqueIdx;
			_pnextnode = _checktail.pCurNode->pNextNode;

			// m_pTail이 위에서 저장한 지역변수 pTail와 동일하다는 것은 pTail에 저장 후에 아직까지는 다른 스레드에서 Enqueue 하지 않은 것을 의미.
			if (_checktail.pCurNode == m_spTail.get()->pCurNode)
			{ 
				// pNext가 nullptr이 라는 것도 역시 다른 곳에서 아직까지 Enqueue 하지 않은 것을 의미. m_pTail->pNext는 nullptr 이어야 하기 때문.
				if (_pnextnode == nullptr)
				{
					// pCheckTail의 pNext는 nullptr이어야 한다.
					// 하지만 이 때, 다른 스레드에서 넣고 빼고를 반복하다보면
					// 다른 스레드에서 새롭게 할당한 pNewNode에 pCheckTail이 가리키는 메모리가 할당 될 수 있다.
					// 이러면 다른 스레드에서 할당한 pNewNode 뒤에 현재 스레드에서 할당한 pNewNode가 들어가게 된다.
					// 사실 큰 문제는 없지만, 조금이라도 더 정확하게 순서를 확보하기 위해서 pNewNode->pNext를 바꾸기 직전에
					// nullptr로 바꾸어 사전에 걸러지게끔 하도록 했다.

					// 상황 :
					// [스레드 1] pCheckTail이 [메모리 1]을 가리키고 [메모리 1의 next]는 nullptr;
					// ... 이순간 여러 스레드에서 넣고 빼고
					// [스레드 4] pNewNode가 [메모리 1]이고 [메모리 1의 next]는 nullptr;
					//
					// 이러면 스레드1이 먼저 발생 했음에도 스레드 4의 pNewNode 뒤에 스레드 1의 pNewNode가 들어감.
					// 이를 최대한 방지하기 위해서 직전에 pNewNode->pNext를 nullptr로 바꿈
					_pnewnode->pNextNode = nullptr;

					if (nullptr == InterlockedCompareExchangePointer( reinterpret_cast<volatile PVOID *>( &_checktail.pCurNode->pNextNode ), reinterpret_cast<PVOID>( _pnewnode ), reinterpret_cast<PVOID>( _pnextnode) ) )
						break;
				}
				// pNext가 nullptr이 아니라는 것은 어디선가 Enqueue가 됬다는 것을 의미
				else
				{
					// 어디선가 Enqueue가 됬으니, m_pTail과 지역변수 pTail이 같은지 확인.
					// 같다면 아직 m_Tail의 위치 이동은 되지 않은 것이니 이동.
					InterlockedCompareExchange128( reinterpret_cast<volatile LONG64 *>( m_spTail.get() ), _lluniqueidx, reinterpret_cast<LONG64>( _pnextnode ), reinterpret_cast<LONG64 *>( &_checktail) );
				}
			}
		}

		// Enqueue가 됬으니, m_pTail의 위치를 pNewNode로 이동
		// 단, m_pTail과 pTail이 동일하다면 위치를 이동
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
			// 지역변수에 저장
			_checkhead.pCurNode = m_spHead.get()->pCurNode;
			_checkhead.llUniqueIdx = m_spHead.get()->llUniqueIdx;
			_pnextnode = _checkhead.pCurNode->pNextNode;

			// nullptr이라는 것은 Head와 Tail이 동일하다는 의미
			// 더이상 뺄 데이터가 없는 상태
			if (nullptr == _pnextnode)
				continue;

			// 다른 스레드에서 노드가 추가됬고, Tail의 위치만 아직 바뀌지 않은 상태.
			// 따라서 Tail을 이동
			if (_checkhead.pCurNode == m_spTail.get()->pCurNode)
			{
				_checktail.pCurNode = m_spTail.get()->pCurNode;
				_checktail.llUniqueIdx = m_spTail.get()->llUniqueIdx;
				_pnextnode = _checktail.pCurNode->pNextNode;

				// pNextNode가 nullptr이 아니라는 것은 어디선가 m_pTail 뒤에 node가 추가 됬다는 것을 의미.
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

		// tls
		//m_pMemoryPool->Free(CheckHead.pCurNode);
		// lf
		m_pMemoryPool->Free(_checkhead.pCurNode, true);
		//delete CheckHead.pCurNode;

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

			// nullptr이라는 것은 Head와 Tail이 동일하다는 의미
			// 더이상 뺄 데이터가 없는 상태
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
			// 더미 노드는 삭제 하지 않는다. 그냥 재사용.
			// 더미 노드는 dequeue 할 때 free가 될 것이기 때문에, 여기서 또 free를 하면 안된다.
			_ptempdelete = _ptemphead->pNextNode;
			_ptemphead = _ptemphead->pNextNode;

			if (nullptr == _ptempdelete)
				break;

			// tls
			//m_pMemoryPool->Free(pTempDelete);
			// lf, non_lf
			m_pMemoryPool->Free(_ptempdelete, true);
			//delete pTempDelete;

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

	// Queue의 MemoryPool 에서는 tls 버전을 사용하면 안되는 이유:
	// Queue에서 메모리풀을 사용하기 위한 전제조건은 해당 인스턴스 전용으로 사용되야 한다는 것이다.
	// Queue의 Enqueue 로직 상 tail의 next가 null 이면 tail의 next에 노드를 연결시키는대, tls를 사용하게 되면 문제가 발생된다.
	// 만약 지역 변수로 tail을 저장 후, 해당 tail의 node가 반환됬고, 다른 스레드에서 할당이 되버리면 문제가 된다.
	// 해당 tail은 다른 세션의 노드가 됬지만 여전히 tail의 next는 nullptr이고, 지역 변수에 저장된 tail의 next도 nullptr이므로
	// 전혀 상관없는 세션의 노드의 next에 추가하려는 노드가 붙어버리게 된다.


	// 1. 스레드1 에서 A라는 세션의 Queue에 새 노드(11)를 넣으려고 한다. tail(8)을 지역변수에 저장한다. 지역변수 tail(8)의 next도, 실제 tail(8)의 next도 nullptr 이다.
	// 2. 스레드2 에서 A라는 세션의 Queue에서 스레드 1에서 지역변수에 저장된 tail(8)의 노드를 반환했다.
	// 3. 스레드3 에서 B라는 세션의 Queue에 스레드2에서 반환한 tail(8)의 노드를 추가한다.
	// 4. 스레드1 의 지역에 저장된 tail(8)의 next도, 실제 tail(8이 아닌 다른 주소)의 next도 여전히 nullptr이다. 따라서, 새 노드(11)은 B라는 세션의 Queue에 있는 tail(8)에 연결된다.

	// tls
	//template<class DATA> CMemoryPool_TLS<typename CQueue_LF<DATA>::NODE> * CQueue_LF<DATA>::m_pMemoryPool;
	// lf
	template<class DATA> CMemoryPool_LF<typename CQueue_LF<DATA>::NODE> * CQueue_LF<DATA>::m_pMemoryPool;
}
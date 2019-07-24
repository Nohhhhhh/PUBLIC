#include "stdafx.h"

/*
락프리큐는 메모리풀이 동적할당하는 것이면 락프리큐의 사이즈도 무한으로 늘어남.
따라서 락프리큐도 동적으로 가야 한다.

Peek은 단독으로 사용되도록 막아줘야 한다. 외부에서 사용하는 구조자체에서

생성자()
{
	alinged malloc

	initial();
}

initiail()
{
	더미노드가 없다면 전부 비워졌을 때 tail을 예외처리 해주어야 한다. 하지만 더미노드가 있다면 head만 건드리면 되니까. 문제가 없다.

	_pHeadst->pNode = _pMemoryPool->Alloc();
}

릴리즈가 오면, 큐의 노드 전부 free해주기만 하면됨.
세션배열을 재사용 하는 것이니까. 
서버가 진짜 꺼질 때만 메모리풀의 노드는 파괴자에서 파괴해야 한다.

put()
{
	new = alloc();
	new == nullptr인지 확인

	지역변수 Checktail = 멤버 tail
	checktao.node 는 주소값이 저장된다.
	pNowNext = CheckTail.pNode->pNext;
	
	if(pNowNext == nullptr)
	{
		pNewNode->pNext = nullptr;
			
		if(exchangepointer == nullptr)
		{
			break;
		}
	}
	else
	{
		그냥 tail의 위치만 next로 바꿔준다.
	}

	// tail을 이동시키는 것은 각자 한다. 

	usesize++;
}

get()
{
	일단 차감 후 뽑을 데이터가 있다면 뽑고 없다면 종료.
	if(InterlockedDefrement(&_useisze) < 0)
	{
		InterlockedIncrement(&usesize);
		return false;
	}

	유니크 = ++유니크;

	while(1)
	{
		checkhead = head;
		next = check->next;

		if(pNext == nullptr)
			return false;

		if(check.pnode == tail->pnode)
		{
			// 해당 로직이 없다면 else 부분만 실행된다는 것을의미
			// 결국 head가 tail을 앞찌르는 상황이 된다.
			// 만약 앞찌를 때 다른 ㅗㅅ에서 enqueue dequeue를 하면 꼬임.
			태일이 아직 이동이 되지 않은 상태니까
			태일을 이동을 시도
			CheckTail = head;

			pNext = checkTailpNode->pNext;

			pNext != nullptr
			interlocked128();
			
		}
		else
		{
			return이 true 일 때만outData를 사용하도록 해야 한다.
			pOutData = next;
			지역이랑 내헤드랑 같으면 next로 변경
			interlocked128
				break;
		}
	}
	
	free(check);

	return true;
}

peek()
{

	pNode = head->pnode->pnext;
	pHead = pNode;

	사이즈 체크
	usesize <= iPos

	for()
	{
		혹시 모르니까 한한칸 이동시 mull 체크
		pNode를 체크

		pNode = pNode->pNext;
	}

	// 혹시 모르니까 또 체크

	outdat 뽑고.


	마지막으로 헤드가 변경됬다면 포기
	if(pHead != 내 head->pnode->pnext)
		return flase;

	우선 peek을 할 때 get을 막지 않는 이상 안전하지 않는다.
	애초에 락을 걸지 않으면 동기화가 불가능 하다.
	하지만 send를 한번 씩만 걸기 때문에 안전하게 사용가능하다고 믿고 가야 한다.

	return true;
}


lanserever에서 사용자 체크를 stack에 배열의 index를 넣는다.

CStack_LF<int>	_SessionIdexStack;
일단 초기화할 때 전부 index 다 넣고.

성공하면 idex return 실패하면 -1 return
사용자 접속하면 하나 뽑고.
릴리즈 할 때 push





*/
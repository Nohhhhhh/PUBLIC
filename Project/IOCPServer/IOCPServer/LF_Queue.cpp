#include "stdafx.h"

/*
������ť�� �޸�Ǯ�� �����Ҵ��ϴ� ���̸� ������ť�� ����� �������� �þ.
���� ������ť�� �������� ���� �Ѵ�.

Peek�� �ܵ����� ���ǵ��� ������� �Ѵ�. �ܺο��� ����ϴ� ������ü����

������()
{
	alinged malloc

	initial();
}

initiail()
{
	���̳�尡 ���ٸ� ���� ������� �� tail�� ����ó�� ���־�� �Ѵ�. ������ ���̳�尡 �ִٸ� head�� �ǵ帮�� �Ǵϱ�. ������ ����.

	_pHeadst->pNode = _pMemoryPool->Alloc();
}

����� ����, ť�� ��� ���� free���ֱ⸸ �ϸ��.
���ǹ迭�� ���� �ϴ� ���̴ϱ�. 
������ ��¥ ���� ���� �޸�Ǯ�� ���� �ı��ڿ��� �ı��ؾ� �Ѵ�.

put()
{
	new = alloc();
	new == nullptr���� Ȯ��

	�������� Checktail = ��� tail
	checktao.node �� �ּҰ��� ����ȴ�.
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
		�׳� tail�� ��ġ�� next�� �ٲ��ش�.
	}

	// tail�� �̵���Ű�� ���� ���� �Ѵ�. 

	usesize++;
}

get()
{
	�ϴ� ���� �� ���� �����Ͱ� �ִٸ� �̰� ���ٸ� ����.
	if(InterlockedDefrement(&_useisze) < 0)
	{
		InterlockedIncrement(&usesize);
		return false;
	}

	����ũ = ++����ũ;

	while(1)
	{
		checkhead = head;
		next = check->next;

		if(pNext == nullptr)
			return false;

		if(check.pnode == tail->pnode)
		{
			// �ش� ������ ���ٸ� else �κи� ����ȴٴ� �����ǹ�
			// �ᱹ head�� tail�� ����� ��Ȳ�� �ȴ�.
			// ���� ��� �� �ٸ� �Ǥ����� enqueue dequeue�� �ϸ� ����.
			������ ���� �̵��� ���� ���� ���´ϱ�
			������ �̵��� �õ�
			CheckTail = head;

			pNext = checkTailpNode->pNext;

			pNext != nullptr
			interlocked128();
			
		}
		else
		{
			return�� true �� ����outData�� ����ϵ��� �ؾ� �Ѵ�.
			pOutData = next;
			�����̶� ������ ������ next�� ����
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

	������ üũ
	usesize <= iPos

	for()
	{
		Ȥ�� �𸣴ϱ� ����ĭ �̵��� mull üũ
		pNode�� üũ

		pNode = pNode->pNext;
	}

	// Ȥ�� �𸣴ϱ� �� üũ

	outdat �̰�.


	���������� ��尡 �����ٸ� ����
	if(pHead != �� head->pnode->pnext)
		return flase;

	�켱 peek�� �� �� get�� ���� �ʴ� �̻� �������� �ʴ´�.
	���ʿ� ���� ���� ������ ����ȭ�� �Ұ��� �ϴ�.
	������ send�� �ѹ� ���� �ɱ� ������ �����ϰ� ��밡���ϴٰ� �ϰ� ���� �Ѵ�.

	return true;
}


lanserever���� ����� üũ�� stack�� �迭�� index�� �ִ´�.

CStack_LF<int>	_SessionIdexStack;
�ϴ� �ʱ�ȭ�� �� ���� index �� �ְ�.

�����ϸ� idex return �����ϸ� -1 return
����� �����ϸ� �ϳ� �̰�.
������ �� �� push





*/
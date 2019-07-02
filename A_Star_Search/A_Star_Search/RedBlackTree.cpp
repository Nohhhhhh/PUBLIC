#include "stdafx.h"
#include "RedBlackTree.h"


CRedBlackTree::CRedBlackTree()
{
	Init();
}

CRedBlackTree::~CRedBlackTree()
{
	Release(m_stpRoot);
}

void CRedBlackTree::Init()
{
	if (m_stpRoot != NULL)
		Release(m_stpRoot);

	m_stpRoot = NULL;
	m_stpNillNode = new ST_RBNODE;
	m_stpNillNode->stpParent = NULL;
	m_stpNillNode->stpLeft = NULL;
	m_stpNillNode->stpRight = NULL;
	m_stpNillNode->eColor = E_NODECOLOR::eBLACK;
	m_stpNillNode->stpTileAddr = NULL;
}

void CRedBlackTree::Release(ST_RBNODE *stpRoot)
{
	if (stpRoot == NULL)
		return;

	ST_RBNODE *stpTemp = stpRoot;

	if (stpRoot->stpLeft != m_stpNillNode)
	{
		stpRoot = stpRoot->stpLeft;
		Release(stpRoot);
		//printf("delete %d\n", stpRoot->iData);
		delete stpRoot;
		stpRoot = stpTemp;
	}

	if (stpRoot->stpRight != m_stpNillNode)
	{
		stpRoot = stpRoot->stpRight;
		Release(stpRoot);
		//printf("delete %d\n", stpRoot->iData);
		delete stpRoot;
		stpRoot = stpTemp;
	}

	if (stpRoot == m_stpRoot)
	{
		//printf("delete %d\n", m_stpRoot->iData);
		delete m_stpRoot;
		m_stpRoot = NULL;

		delete m_stpNillNode;
		m_stpNillNode = NULL;
	}
}


BOOL CRedBlackTree::InsertNode(CAStar::ST_TILENODE *stpInsertAddr)
{
	ST_RBNODE *stpNew = new ST_RBNODE;

	stpNew->stpParent = NULL;
	stpNew->stpLeft = m_stpNillNode;
	stpNew->stpRight = m_stpNillNode;
	stpNew->eColor = E_NODECOLOR::eRED;
	stpNew->stpTileAddr = stpInsertAddr;

	if (!Link_Node(m_stpRoot, stpNew))
	{
		delete stpNew;
		return FALSE;
	}


	Balance_Insert(stpNew);

	return TRUE;
}

BOOL CRedBlackTree::Link_Node(ST_RBNODE *stpRoot, ST_RBNODE *stpNew)
{
	BOOL bStatus = TRUE;

	//-------------------------------------------------------------------
	// m_stpRoot가 NULL 이면, 데이터 없는 것.
	// 따라서 루트노드에 해당하는 데이터 초기화
	//-------------------------------------------------------------------
	if (m_stpRoot == NULL)
	{
		stpNew->eColor = E_NODECOLOR::eBLACK;
		m_stpRoot = stpNew;
		bStatus = TRUE;
		return bStatus;
	}
	//-------------------------------------------------------------------
	// stpRoot의 data와 stpNew의 data가
	// 동일하는 것은, 중복된 값이라는 의미이기 때문에
	// 아무 처리 하지 않는다.
	//-------------------------------------------------------------------
	else if (stpRoot->stpTileAddr == stpNew->stpTileAddr)
	{
		bStatus = FALSE;
		return bStatus;
	}
	//-------------------------------------------------------------------
	// stpRoot의 data가 stpNew의 data보다 크고/작고,
	// stpRoot의 왼쪽/오른쪽 자식노드가 nill 노드가 아니면
	// 자식노드가 있다는 의미이므로 재귀함수 호출
	//-------------------------------------------------------------------
	else if (stpRoot->stpTileAddr > stpNew->stpTileAddr && stpRoot->stpLeft != m_stpNillNode)
	{
		bStatus = Link_Node(stpRoot->stpLeft, stpNew);
	}
	else if (stpRoot->stpTileAddr < stpNew->stpTileAddr && stpRoot->stpRight != m_stpNillNode)
	{
		bStatus = Link_Node(stpRoot->stpRight, stpNew);
	}
	//-------------------------------------------------------------------
	// stpNew 노드가 삽입될 위치를 찾은 경우
	//-------------------------------------------------------------------
	else
	{
		if (stpRoot->stpTileAddr > stpNew->stpTileAddr)
			stpRoot->stpLeft = stpNew;
		else
			stpRoot->stpRight = stpNew;

		stpNew->stpParent = stpRoot;
	}

	return bStatus;
}

void CRedBlackTree::Balance_Insert(ST_RBNODE *stpNew)
{
	ST_RBNODE *stpParent;
	ST_RBNODE *stpGrandParentNode;
	ST_RBNODE *stpUncleNode;

	if (stpNew == m_stpRoot)
		return;

	if (stpNew->stpParent->eColor == eBLACK)
		return;

	while (stpNew->stpParent != m_stpRoot && stpNew->stpParent->eColor == eRED && stpNew->eColor == eRED)
	{
		stpParent = stpNew->stpParent;
		stpGrandParentNode = stpParent->stpParent;

		//-------------------------------------------------------------------
		// 부모노드가 할아버지노드의 왼쪽
		//-------------------------------------------------------------------
		if (stpParent == stpGrandParentNode->stpLeft)
		{
			stpUncleNode = stpGrandParentNode->stpRight;

			if (stpUncleNode->eColor == eRED)
			{
				stpParent->eColor = eBLACK;
				stpUncleNode->eColor = eBLACK;
				stpGrandParentNode->eColor = eRED;

				//-------------------------------------------------------------------
				// 위의 로직을 통해, 부모를 기준으로 밸런스의 변화는 없음.
				// 하지만 할아버지 노드를 레드로 바꿨기 때문에, 증조할아버지의 색을 
				// 비교해봐야 함. 따라서, newNode에 할아버지 노드를 대입해서 비교해야 함
				//-------------------------------------------------------------------
				if (stpGrandParentNode != m_stpRoot)
					stpNew = stpGrandParentNode;
			}
			else
			{
				//-------------------------------------------------------------------
				// 부모노드가 할아버지노드의 왼쪽,
				// 부모의 오른쪽 자식이 stpNew면 부모를 기준으로 좌회전 후,
				// 할아버지를 기준으로 우회전
				//-------------------------------------------------------------------
				if (stpNew == stpParent->stpRight)
				{
					Rotation_Left(stpParent);
					Rotation_Right(stpGrandParentNode);

					stpNew->eColor = eBLACK;
					stpGrandParentNode->eColor = eRED;

					if (stpNew->stpRight == m_stpRoot)
					{
						m_stpRoot = stpNew;
						break;
					}
					//-------------------------------------------------------------------
					// 자식과 부모 노드가 레드인 경우, stpNew는 자식노드가 되어야 함
					// 부모노드의 왼쪽에 stpNew가 있는 상황을 만들기 위해
					//-------------------------------------------------------------------
					//stpNew = stpNew->stpLeft;
				}
				//-------------------------------------------------------------------
				// 부모노드가 할아버지노드의 왼쪽,
				// 부모의 왼쪽 자식이 stpNew면 할아버지를 기준으로 우회전
				//-------------------------------------------------------------------
				else
				{
					Rotation_Right(stpGrandParentNode);

					stpParent->eColor = eBLACK;
					stpGrandParentNode->eColor = eRED;

					if (stpParent->stpRight == m_stpRoot)
					{
						m_stpRoot = stpParent;
						break;
					}

					//-------------------------------------------------------------------
					// 자식과 부모 노드가 레드인 경우, stpNew는 자식노드가 되어야 함
					// 부모노드의 오른쪽에 stpNew가 있는 상황을 만들기 위해
					//-------------------------------------------------------------------
					//stpNew = stpNew->stpParent;
				}
			}
		}
		//-------------------------------------------------------------------
		// 부모노드가 할아버지노드의 오른쪽
		//-------------------------------------------------------------------
		else
		{
			stpUncleNode = stpGrandParentNode->stpLeft;

			if (stpUncleNode->eColor == eRED)
			{
				stpParent->eColor = eBLACK;
				stpUncleNode->eColor = eBLACK;
				stpGrandParentNode->eColor = eRED;

				if (stpGrandParentNode != m_stpRoot)
					stpNew = stpGrandParentNode;
			}
			else
			{
				//-------------------------------------------------------------------
				// 부모노드가 할아버지노드의 오른쪽,
				// 부모의 왼쪽 자식이 stpNew면 부모를 기준으로 우회전 후,
				// 할아버지를 기준으로 좌회전
				//-------------------------------------------------------------------
				if (stpNew == stpParent->stpLeft)
				{
					Rotation_Right(stpParent);
					Rotation_Left(stpGrandParentNode);

					stpNew->eColor = eBLACK;
					stpGrandParentNode->eColor = eRED;

					if (stpNew->stpLeft == m_stpRoot)
					{
						m_stpRoot = stpNew;
						break;
					}
				}
				//-------------------------------------------------------------------
				// 부모노드가 할아버지노드의 오른쪽,
				// 부모의 오른쪽 자식이 stpNew면 할아버지를 기준으로 좌회전
				//-------------------------------------------------------------------
				else
				{
					Rotation_Left(stpGrandParentNode);

					stpParent->eColor = eBLACK;
					stpGrandParentNode->eColor = eRED;

					if (stpParent->stpLeft == m_stpRoot)
					{
						m_stpRoot = stpParent;
						break;
					}
				}
			}

		}
	}

	m_stpRoot->eColor = eBLACK;
}


void CRedBlackTree::Rotation_Left(ST_RBNODE *stpRoot)
{
	ST_RBNODE *stpChildNode = stpRoot->stpRight;

	stpRoot->stpRight = stpChildNode->stpLeft;
	stpChildNode->stpLeft->stpParent = stpRoot;
	stpChildNode->stpLeft = stpRoot;

	if (stpRoot->stpParent != NULL)
	{
		if (stpRoot->stpParent->stpLeft == stpRoot)
			stpRoot->stpParent->stpLeft = stpChildNode;
		else
			stpRoot->stpParent->stpRight = stpChildNode;
	}

	stpChildNode->stpParent = stpRoot->stpParent;
	stpChildNode->stpLeft->stpParent = stpChildNode;
}

void CRedBlackTree::Rotation_Right(ST_RBNODE *stpRoot)
{
	ST_RBNODE *stpChildNode = stpRoot->stpLeft;

	stpRoot->stpLeft = stpChildNode->stpRight;
	stpChildNode->stpRight->stpParent = stpRoot;
	stpChildNode->stpRight = stpRoot;

	if (stpRoot->stpParent != NULL)
	{
		if (stpRoot->stpParent->stpLeft == stpRoot)
			stpRoot->stpParent->stpLeft = stpChildNode;
		else
			stpRoot->stpParent->stpRight = stpChildNode;
	}

	stpChildNode->stpParent = stpRoot->stpParent;
	stpChildNode->stpRight->stpParent = stpChildNode;
}


CAStar::ST_TILENODE* CRedBlackTree::InorderIter(int iTileX, int iTileY)
{
	return Inorder(m_stpRoot, iTileX, iTileY);
}

CAStar::ST_TILENODE* CRedBlackTree::Inorder(ST_RBNODE *stpRoot, int iTileX, int iTileY)
{
	if (stpRoot == NULL)
		return NULL;

	if (stpRoot->stpLeft != m_stpNillNode)
		Inorder(stpRoot->stpLeft, iTileX, iTileY);

	if (stpRoot->stpTileAddr->iX == iTileX && stpRoot->stpTileAddr->iY == iTileY)
		return stpRoot->stpTileAddr;

	//printf("%*d : %d\n", iDepth, stpRoot->iData, stpRoot->eColor);

	if (stpRoot->stpRight != m_stpNillNode)
		Inorder(stpRoot->stpRight, iTileX, iTileY);

	if (stpRoot == m_stpRoot)
		return NULL;
}

/*
BOOL CRedBlackTree::DeleteNode(CAStar::ST_TILENODE *stpDeletetAddr)
{
	ST_RBNODE *stpCurrent = m_stpRoot;
	ST_RBNODE *stpDelete = Delete(stpDeletetAddr, m_stpRoot, stpCurrent);

	//------------------------------------------------------------------------------
	// 삭제할 노드의 자식이 2개이 이상일 때만 balance로 진입
	//------------------------------------------------------------------------------
	//if (stpDeleteNode->stpLeft != m_stpNillNode && stpDeleteNode->stpRight != m_stpNillNode)
	Balance_Delete(stpDelete);

	delete stpDelete;

	return TRUE;
}


CRedBlackTree::ST_RBNODE* CRedBlackTree::Delete(CAStar::ST_TILENODE *stpDeletetAddr, ST_RBNODE *stpRoot, ST_RBNODE *stpCurrent)
{

	ST_RBNODE *stpDelete = NULL;
	ST_RBNODE *stpReplace = NULL;

	//------------------------------------------------------------------------------
	// 재귀함수를 통해서 삭제할 노드와 삭제할 노드의 부모노드를 찾을 동안 해당 로직 수행
	//------------------------------------------------------------------------------
	if (stpCurrent->stpTileAddr > stpDeletetAddr)
	{
		stpDelete = Delete(stpDeletetAddr, stpCurrent, stpCurrent->stpLeft);
	}
	else if (stpCurrent->stpTileAddr < stpDeletetAddr)
	{
		stpDelete = Delete(stpDeletetAddr, stpCurrent, stpCurrent->stpRight);
	}
	//------------------------------------------------------------------------------
	// 삭제할 노드와 삭제할 노드의 부모노드를 찾은 경우 아래 로직으로 수행
	//------------------------------------------------------------------------------
	else
	{
		//------------------------------------------------------------------------------
		// 자식 노드가 없는 경우
		//------------------------------------------------------------------------------
		if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
		{
			//------------------------------------------------------------------------------
			// 삭제할 노드가 Root 일 경우
			//------------------------------------------------------------------------------
			if (m_stpRoot == stpCurrent)
			{
				m_stpRoot = NULL;
			}
			//------------------------------------------------------------------------------
			// 삭제할 노드가 부모 노드의 왼쪽 자식노드인 경우
			//------------------------------------------------------------------------------
			else if (stpRoot->stpLeft == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// root와 현재노드의 자식의 부모와 연결해준다.
				//------------------------------------------------------------------------------
				stpRoot->stpLeft = stpCurrent->stpLeft;
				stpCurrent->stpLeft->stpParent = stpRoot;
			}
			//------------------------------------------------------------------------------
			// 삭제할 노드가 부모 노드의 오른쪽 자식노드인 경우
			//------------------------------------------------------------------------------
			else if (stpRoot->stpRight == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// root와 현재노드의 자식의 부모와 연결해준다.
				//------------------------------------------------------------------------------
				stpRoot->stpRight = stpCurrent->stpLeft;
				stpCurrent->stpLeft->stpParent = stpRoot;
			}

			return stpCurrent;
		}
		//------------------------------------------------------------------------------
		// 자식 노드가 1개 있는 경우
		//------------------------------------------------------------------------------
		else if ((stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode) || (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode))
		{
			//------------------------------------------------------------------------------
			// 삭제할 노드가 루트 노드인 경우
			//------------------------------------------------------------------------------
			if (m_stpRoot == stpCurrent)
			{
				if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// 삭제할 노드가 루트니까 현재노드의 자식이 root가 되는 경우이다. 
					// 따라서, 자식의 부모를 null
					//------------------------------------------------------------------------------
					m_stpRoot = stpCurrent->stpLeft;
					stpCurrent->stpLeft->stpParent = NULL;
				}
				else if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// 삭제할 노드가 루트니까 현재노드의 자식이 root가 되는 경우이다. 
					// 따라서, 자식의 부모를 null
					//------------------------------------------------------------------------------
					m_stpRoot = stpCurrent->stpRight;
					stpCurrent->stpRight->stpParent = NULL;
				}
			}
			//------------------------------------------------------------------------------
			// 삭제할 노드가 부모 노드의 왼쪽 자식노드인 경우
			//------------------------------------------------------------------------------
			if (stpRoot->stpLeft == stpCurrent)
			{
				if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root와 현재노드의 자식의 부모와 연결해준다.
					//------------------------------------------------------------------------------
					stpRoot->stpLeft = stpCurrent->stpLeft;
					stpCurrent->stpLeft->stpParent = stpRoot;
				}
				else if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root와 현재노드의 자식의 부모와 연결해준다.
					//------------------------------------------------------------------------------
					stpRoot->stpLeft = stpCurrent->stpRight;
					stpCurrent->stpRight->stpParent = stpRoot;
				}
			}
			//------------------------------------------------------------------------------
			// 삭제할 노드가 부모 노드의 오른쪽 자식노드인 경우
			//------------------------------------------------------------------------------
			else if (stpRoot->stpRight == stpCurrent)
			{
				if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root와 현재노드의 자식의 부모와 연결해준다.
					//------------------------------------------------------------------------------
					stpRoot->stpRight = stpCurrent->stpLeft;
					stpCurrent->stpLeft->stpParent = stpRoot;
				}
				else if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root와 현재노드의 자식의 부모와 연결해준다.
					//------------------------------------------------------------------------------
					stpRoot->stpRight = stpCurrent->stpRight;
					stpCurrent->stpRight->stpParent = stpRoot;
				}
			}
			return stpCurrent;
		}
		//------------------------------------------------------------------------------
		// 자식 노드가 2개 있는 경우
		//------------------------------------------------------------------------------
		else if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
		{
			//------------------------------------------------------------------------------
			// 삭제할 노드가 루트 노드인 경우
			//------------------------------------------------------------------------------
			if (m_stpRoot == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// 왼쪽을 기준으로 이동할 노드를 찾는다.
				// 이 경우, findnodefordelete 함수에서 root와 자식의 연결을 해준다.
				//------------------------------------------------------------------------------
				stpReplace = FindNodeForDelete(stpCurrent, stpCurrent->stpLeft);

				stpCurrent->stpTileAddr = stpReplace->stpTileAddr;
			}
			//------------------------------------------------------------------------------
			// 삭제할 노드가 부모 노드의 왼쪽 자식노드인 경우
			//------------------------------------------------------------------------------
			else if (stpRoot->stpLeft == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// 왼쪽을 기준으로 이동할 노드를 찾는다.
				// 이 경우, findnodefordelete 함수에서 root와 자식의 연결을 해준다.
				//------------------------------------------------------------------------------
				stpReplace = FindNodeForDelete(stpCurrent, stpCurrent->stpLeft);

				stpCurrent->stpTileAddr = stpReplace->stpTileAddr;
			}
			//------------------------------------------------------------------------------
			// 삭제할 노드가 부모 노드의 오른쪽 자식노드인 경우
			//------------------------------------------------------------------------------
			else if (stpRoot->stpRight == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// 왼쪽을 기준으로 이동할 노드를 찾는다.
				// 이 경우, findnodefordelete 함수에서 root와 자식의 연결을 해준다.
				//------------------------------------------------------------------------------
				stpReplace = FindNodeForDelete(stpCurrent, stpCurrent->stpLeft);

				stpCurrent->stpTileAddr = stpReplace->stpTileAddr;
			}
			return stpReplace;
		}
	}

	return stpDelete;
}

void CRedBlackTree::Balance_Delete(ST_RBNODE *stpDelete)
{
	ST_RBNODE *stpReplace;
	ST_RBNODE *stpParent;
	ST_RBNODE *stpSibling;

	//------------------------------------------------------------------------------
	// 실제로 삭제될 노드의 색이 레드면, 할일이 없음.
	// 주의 할 점은 대체 될 노드의 색이 레드가 아니라는 것.
	//------------------------------------------------------------------------------
	if (stpDelete->eColor == eRED)
		return;

	//------------------------------------------------------------------------------
	// 실제로 삭제될 노드의 왼쪽자식이 nill이고, 오른쪽 자식이 nill이 아닐 때는 
	// replace 노드는 실제 삭제될 노드의 오른쪽 자식.
	// 실제로 삭제될 노드의 오른쪽자식이 nill이고, 왼쪽 자식이 nill이 아닐 때는
	// replace 노드는 실제 삭제될 노드의 왼쪽 자식.
	//------------------------------------------------------------------------------
	if (stpDelete->stpLeft == m_stpNillNode && stpDelete->stpRight != m_stpNillNode)
		stpReplace = stpDelete->stpRight;
	else
		stpReplace = stpDelete->stpLeft;

	while (stpReplace != m_stpRoot && stpReplace->eColor == eBLACK && m_stpRoot != NULL)
	{
		stpParent = stpReplace->stpParent;

		//------------------------------------------------------------------------------
		// 부모기준으로 replacenode가 왼쪽인 경우
		//------------------------------------------------------------------------------
		if (stpParent->stpLeft == stpReplace)
		{
			stpSibling = stpParent->stpRight;

			//------------------------------------------------------------------------------
			// 형제가 레드인 경우
			//------------------------------------------------------------------------------
			if (stpSibling->eColor == eRED)
			{
				stpSibling->eColor = eBLACK;
				stpParent->eColor = eRED;
				Rotation_Left(stpParent);

				if (stpReplace->stpParent == m_stpRoot)
					m_stpRoot = stpParent->stpParent;
			}
			//------------------------------------------------------------------------------
			// 형제가 블랙이고, 형제의 두 자식이 모두 블랙인 경우
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpLeft->eColor == eBLACK && stpSibling->stpRight->eColor == eBLACK)
			{
				stpSibling->eColor = eRED;
				stpReplace = stpParent;
			}
			//------------------------------------------------------------------------------
			// 형제가 블랙이고, 형제의 왼자식이 레드, 오른자식은 레드가 아닌 경우에만 해당 로직 수행
			// 형제가 블랙이고, 형제의 왼자식이 레드, 오른자식도 레드인 경우는
			// 이미 형제가 블랙이고, 형제의 오른자식이 레드인 경우에 해당하므로,
			// 아래의 로직을 수행할 필요 없음
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpLeft->eColor == eRED && stpSibling->stpRight->eColor != eRED)
			{
				stpSibling->stpLeft->eColor = eBLACK;
				stpSibling->eColor = eRED;
				Rotation_Right(stpSibling);
				stpSibling = stpSibling->stpParent;

				stpSibling->eColor = stpParent->eColor;
				stpSibling->stpParent->eColor = eBLACK;
				stpSibling->stpRight->eColor = eBLACK;
				Rotation_Left(stpParent);

				if (stpReplace->stpParent == m_stpRoot)
					m_stpRoot = stpParent->stpParent;

				stpReplace = m_stpRoot;
			}
			//------------------------------------------------------------------------------
			// 형제가 블랙이고, 형제의 오른자식이 레드인 경우, 형제의 왼자식은 어떤색이든 상관 X
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpRight->eColor == eRED)
			{
				stpSibling->eColor = stpParent->eColor;
				stpSibling->stpParent->eColor = eBLACK;
				stpSibling->stpRight->eColor = eBLACK;
				Rotation_Left(stpParent);

				if (stpReplace->stpParent == m_stpRoot)
					m_stpRoot = stpParent->stpParent;

				stpReplace = m_stpRoot;
			}
		}
		//------------------------------------------------------------------------------
		// 부모기준으로 replacenode가 오른쪽인 경우
		//------------------------------------------------------------------------------
		else
		{
			stpSibling = stpParent->stpLeft;

			//------------------------------------------------------------------------------
			// 형제가 레드인 경우
			//------------------------------------------------------------------------------
			if (stpSibling->eColor == eRED)
			{
				stpSibling->eColor = eBLACK;
				stpParent->eColor = eRED;
				Rotation_Right(stpParent);

				if (stpReplace->stpParent == m_stpRoot)
					m_stpRoot = stpParent->stpParent;
			}
			//------------------------------------------------------------------------------
			// 형제가 블랙이고, 형제의 두 자식이 모두 블랙인 경우
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpLeft->eColor == eBLACK && stpSibling->stpRight->eColor == eBLACK)
			{
				stpSibling->eColor = eRED;
				stpReplace = stpParent;
			}
			//------------------------------------------------------------------------------
			// 형제가 블랙이고, 형제의 오른자식 레드, 왼자식은 레드가 아닌 경우에만 해당 로직 수행
			// 형제가 블랙이고, 형제의 오른자식 레드, 왼자식도 레드인 경우는
			// 이미 형제가 블랙이고, 형제의 왼자식이 레드인 경우에 해당하므로,
			// 아래의 로직을 수행할 필요 없음
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpRight->eColor == eRED && stpSibling->stpLeft->eColor != eRED)
			{
				stpSibling->stpRight->eColor = eBLACK;
				stpSibling->eColor = eRED;
				Rotation_Left(stpSibling);
				stpSibling = stpSibling->stpParent;

				stpSibling->eColor = stpParent->eColor;
				stpSibling->stpParent->eColor = eBLACK;
				stpSibling->stpLeft->eColor = eBLACK;
				Rotation_Right(stpParent);

				if (stpReplace->stpParent == m_stpRoot)
					m_stpRoot = stpParent->stpParent;

				stpReplace = m_stpRoot;
			}
			//------------------------------------------------------------------------------
			// 형제가 블랙이고, 형제의 왼자식이 레드인 경우, 형제의 오른자식은 어떤색이든 상관 X
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpLeft->eColor == eRED)
			{
				stpSibling->eColor = stpParent->eColor;
				stpSibling->stpParent->eColor = eBLACK;
				stpSibling->stpLeft->eColor = eBLACK;
				Rotation_Right(stpParent);

				if (stpReplace->stpParent == m_stpRoot)
					m_stpRoot = stpParent->stpParent;

				stpReplace = m_stpRoot;
			}
		}
	}


	stpReplace->eColor = eBLACK;
}

CRedBlackTree::ST_RBNODE* CRedBlackTree::FindNodeForDelete(ST_RBNODE *stpRoot, ST_RBNODE *stpCurrent)
{
	ST_RBNODE *stpMoveNode = NULL;

	//------------------------------------------------------------------------------
	// 현재 노드의 오른쪽 노드가 없을 때 까지 재귀함수 호출.
	//------------------------------------------------------------------------------
	if (stpCurrent->stpRight != m_stpNillNode)
	{
		stpMoveNode = FindNodeForDelete(stpCurrent, stpCurrent->stpRight);
	}
	//------------------------------------------------------------------------------
	// 현재 노드의 오른쪽 노드가 없으면, 
	// 현재 노드의 왼쪽 노드를 부모 노드와 연결 현재 노드를 리턴
	//------------------------------------------------------------------------------
	else
	{
		//------------------------------------------------------------------------------
		// root 노드의 왼쪽 자식이 현재노드(삭제할 노드)이면, 
		// root의 왼쪽 노드와 현재노드의 왼쪽을 연결
		// root 노드의 오른쪽 자식이 현재노드(삭제할 노드)이면, 
		// root의 오른쪽 노드와 현재노드의 왼쪽을 연결
		//------------------------------------------------------------------------------
		if (stpRoot->stpLeft == stpCurrent)
		{
			stpRoot->stpLeft = stpCurrent->stpLeft;
			stpCurrent->stpLeft->stpParent = stpRoot;
		}
		else
		{
			stpRoot->stpRight = stpCurrent->stpLeft;
			stpCurrent->stpLeft->stpParent = stpRoot;
		}


		return stpCurrent;
	}

	return stpMoveNode;
}

void CRedBlackTree::InorderPrint()
{
	Inorder(m_stpRoot, 0);
}

void CRedBlackTree::Inorder(ST_RBNODE *stpRoot, int iDepth)
{
	if (stpRoot == NULL)
		return;

	if (iDepth < 0)
		return;

	if (stpRoot->stpLeft != m_stpNillNode)
		Inorder(stpRoot->stpLeft, iDepth + 5);

	printf("%*d : %d\n", iDepth, stpRoot->iData, stpRoot->eColor);

	if (stpRoot->stpRight != m_stpNillNode)
		Inorder(stpRoot->stpRight, iDepth + 5);
}
*/
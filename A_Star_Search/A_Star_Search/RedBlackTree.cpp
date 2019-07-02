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
	// m_stpRoot�� NULL �̸�, ������ ���� ��.
	// ���� ��Ʈ��忡 �ش��ϴ� ������ �ʱ�ȭ
	//-------------------------------------------------------------------
	if (m_stpRoot == NULL)
	{
		stpNew->eColor = E_NODECOLOR::eBLACK;
		m_stpRoot = stpNew;
		bStatus = TRUE;
		return bStatus;
	}
	//-------------------------------------------------------------------
	// stpRoot�� data�� stpNew�� data��
	// �����ϴ� ����, �ߺ��� ���̶�� �ǹ��̱� ������
	// �ƹ� ó�� ���� �ʴ´�.
	//-------------------------------------------------------------------
	else if (stpRoot->stpTileAddr == stpNew->stpTileAddr)
	{
		bStatus = FALSE;
		return bStatus;
	}
	//-------------------------------------------------------------------
	// stpRoot�� data�� stpNew�� data���� ũ��/�۰�,
	// stpRoot�� ����/������ �ڽĳ�尡 nill ��尡 �ƴϸ�
	// �ڽĳ�尡 �ִٴ� �ǹ��̹Ƿ� ����Լ� ȣ��
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
	// stpNew ��尡 ���Ե� ��ġ�� ã�� ���
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
		// �θ��尡 �Ҿƹ�������� ����
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
				// ���� ������ ����, �θ� �������� �뷱���� ��ȭ�� ����.
				// ������ �Ҿƹ��� ��带 ����� �ٲ�� ������, �����Ҿƹ����� ���� 
				// ���غ��� ��. ����, newNode�� �Ҿƹ��� ��带 �����ؼ� ���ؾ� ��
				//-------------------------------------------------------------------
				if (stpGrandParentNode != m_stpRoot)
					stpNew = stpGrandParentNode;
			}
			else
			{
				//-------------------------------------------------------------------
				// �θ��尡 �Ҿƹ�������� ����,
				// �θ��� ������ �ڽ��� stpNew�� �θ� �������� ��ȸ�� ��,
				// �Ҿƹ����� �������� ��ȸ��
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
					// �ڽİ� �θ� ��尡 ������ ���, stpNew�� �ڽĳ�尡 �Ǿ�� ��
					// �θ����� ���ʿ� stpNew�� �ִ� ��Ȳ�� ����� ����
					//-------------------------------------------------------------------
					//stpNew = stpNew->stpLeft;
				}
				//-------------------------------------------------------------------
				// �θ��尡 �Ҿƹ�������� ����,
				// �θ��� ���� �ڽ��� stpNew�� �Ҿƹ����� �������� ��ȸ��
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
					// �ڽİ� �θ� ��尡 ������ ���, stpNew�� �ڽĳ�尡 �Ǿ�� ��
					// �θ����� �����ʿ� stpNew�� �ִ� ��Ȳ�� ����� ����
					//-------------------------------------------------------------------
					//stpNew = stpNew->stpParent;
				}
			}
		}
		//-------------------------------------------------------------------
		// �θ��尡 �Ҿƹ�������� ������
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
				// �θ��尡 �Ҿƹ�������� ������,
				// �θ��� ���� �ڽ��� stpNew�� �θ� �������� ��ȸ�� ��,
				// �Ҿƹ����� �������� ��ȸ��
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
				// �θ��尡 �Ҿƹ�������� ������,
				// �θ��� ������ �ڽ��� stpNew�� �Ҿƹ����� �������� ��ȸ��
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
	// ������ ����� �ڽ��� 2���� �̻��� ���� balance�� ����
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
	// ����Լ��� ���ؼ� ������ ���� ������ ����� �θ��带 ã�� ���� �ش� ���� ����
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
	// ������ ���� ������ ����� �θ��带 ã�� ��� �Ʒ� �������� ����
	//------------------------------------------------------------------------------
	else
	{
		//------------------------------------------------------------------------------
		// �ڽ� ��尡 ���� ���
		//------------------------------------------------------------------------------
		if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
		{
			//------------------------------------------------------------------------------
			// ������ ��尡 Root �� ���
			//------------------------------------------------------------------------------
			if (m_stpRoot == stpCurrent)
			{
				m_stpRoot = NULL;
			}
			//------------------------------------------------------------------------------
			// ������ ��尡 �θ� ����� ���� �ڽĳ���� ���
			//------------------------------------------------------------------------------
			else if (stpRoot->stpLeft == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// root�� �������� �ڽ��� �θ�� �������ش�.
				//------------------------------------------------------------------------------
				stpRoot->stpLeft = stpCurrent->stpLeft;
				stpCurrent->stpLeft->stpParent = stpRoot;
			}
			//------------------------------------------------------------------------------
			// ������ ��尡 �θ� ����� ������ �ڽĳ���� ���
			//------------------------------------------------------------------------------
			else if (stpRoot->stpRight == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// root�� �������� �ڽ��� �θ�� �������ش�.
				//------------------------------------------------------------------------------
				stpRoot->stpRight = stpCurrent->stpLeft;
				stpCurrent->stpLeft->stpParent = stpRoot;
			}

			return stpCurrent;
		}
		//------------------------------------------------------------------------------
		// �ڽ� ��尡 1�� �ִ� ���
		//------------------------------------------------------------------------------
		else if ((stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode) || (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode))
		{
			//------------------------------------------------------------------------------
			// ������ ��尡 ��Ʈ ����� ���
			//------------------------------------------------------------------------------
			if (m_stpRoot == stpCurrent)
			{
				if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// ������ ��尡 ��Ʈ�ϱ� �������� �ڽ��� root�� �Ǵ� ����̴�. 
					// ����, �ڽ��� �θ� null
					//------------------------------------------------------------------------------
					m_stpRoot = stpCurrent->stpLeft;
					stpCurrent->stpLeft->stpParent = NULL;
				}
				else if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// ������ ��尡 ��Ʈ�ϱ� �������� �ڽ��� root�� �Ǵ� ����̴�. 
					// ����, �ڽ��� �θ� null
					//------------------------------------------------------------------------------
					m_stpRoot = stpCurrent->stpRight;
					stpCurrent->stpRight->stpParent = NULL;
				}
			}
			//------------------------------------------------------------------------------
			// ������ ��尡 �θ� ����� ���� �ڽĳ���� ���
			//------------------------------------------------------------------------------
			if (stpRoot->stpLeft == stpCurrent)
			{
				if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root�� �������� �ڽ��� �θ�� �������ش�.
					//------------------------------------------------------------------------------
					stpRoot->stpLeft = stpCurrent->stpLeft;
					stpCurrent->stpLeft->stpParent = stpRoot;
				}
				else if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root�� �������� �ڽ��� �θ�� �������ش�.
					//------------------------------------------------------------------------------
					stpRoot->stpLeft = stpCurrent->stpRight;
					stpCurrent->stpRight->stpParent = stpRoot;
				}
			}
			//------------------------------------------------------------------------------
			// ������ ��尡 �θ� ����� ������ �ڽĳ���� ���
			//------------------------------------------------------------------------------
			else if (stpRoot->stpRight == stpCurrent)
			{
				if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight == m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root�� �������� �ڽ��� �θ�� �������ش�.
					//------------------------------------------------------------------------------
					stpRoot->stpRight = stpCurrent->stpLeft;
					stpCurrent->stpLeft->stpParent = stpRoot;
				}
				else if (stpCurrent->stpLeft == m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
				{
					//------------------------------------------------------------------------------
					// root�� �������� �ڽ��� �θ�� �������ش�.
					//------------------------------------------------------------------------------
					stpRoot->stpRight = stpCurrent->stpRight;
					stpCurrent->stpRight->stpParent = stpRoot;
				}
			}
			return stpCurrent;
		}
		//------------------------------------------------------------------------------
		// �ڽ� ��尡 2�� �ִ� ���
		//------------------------------------------------------------------------------
		else if (stpCurrent->stpLeft != m_stpNillNode && stpCurrent->stpRight != m_stpNillNode)
		{
			//------------------------------------------------------------------------------
			// ������ ��尡 ��Ʈ ����� ���
			//------------------------------------------------------------------------------
			if (m_stpRoot == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// ������ �������� �̵��� ��带 ã�´�.
				// �� ���, findnodefordelete �Լ����� root�� �ڽ��� ������ ���ش�.
				//------------------------------------------------------------------------------
				stpReplace = FindNodeForDelete(stpCurrent, stpCurrent->stpLeft);

				stpCurrent->stpTileAddr = stpReplace->stpTileAddr;
			}
			//------------------------------------------------------------------------------
			// ������ ��尡 �θ� ����� ���� �ڽĳ���� ���
			//------------------------------------------------------------------------------
			else if (stpRoot->stpLeft == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// ������ �������� �̵��� ��带 ã�´�.
				// �� ���, findnodefordelete �Լ����� root�� �ڽ��� ������ ���ش�.
				//------------------------------------------------------------------------------
				stpReplace = FindNodeForDelete(stpCurrent, stpCurrent->stpLeft);

				stpCurrent->stpTileAddr = stpReplace->stpTileAddr;
			}
			//------------------------------------------------------------------------------
			// ������ ��尡 �θ� ����� ������ �ڽĳ���� ���
			//------------------------------------------------------------------------------
			else if (stpRoot->stpRight == stpCurrent)
			{
				//------------------------------------------------------------------------------
				// ������ �������� �̵��� ��带 ã�´�.
				// �� ���, findnodefordelete �Լ����� root�� �ڽ��� ������ ���ش�.
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
	// ������ ������ ����� ���� �����, ������ ����.
	// ���� �� ���� ��ü �� ����� ���� ���尡 �ƴ϶�� ��.
	//------------------------------------------------------------------------------
	if (stpDelete->eColor == eRED)
		return;

	//------------------------------------------------------------------------------
	// ������ ������ ����� �����ڽ��� nill�̰�, ������ �ڽ��� nill�� �ƴ� ���� 
	// replace ���� ���� ������ ����� ������ �ڽ�.
	// ������ ������ ����� �������ڽ��� nill�̰�, ���� �ڽ��� nill�� �ƴ� ����
	// replace ���� ���� ������ ����� ���� �ڽ�.
	//------------------------------------------------------------------------------
	if (stpDelete->stpLeft == m_stpNillNode && stpDelete->stpRight != m_stpNillNode)
		stpReplace = stpDelete->stpRight;
	else
		stpReplace = stpDelete->stpLeft;

	while (stpReplace != m_stpRoot && stpReplace->eColor == eBLACK && m_stpRoot != NULL)
	{
		stpParent = stpReplace->stpParent;

		//------------------------------------------------------------------------------
		// �θ�������� replacenode�� ������ ���
		//------------------------------------------------------------------------------
		if (stpParent->stpLeft == stpReplace)
		{
			stpSibling = stpParent->stpRight;

			//------------------------------------------------------------------------------
			// ������ ������ ���
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
			// ������ ���̰�, ������ �� �ڽ��� ��� ���� ���
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpLeft->eColor == eBLACK && stpSibling->stpRight->eColor == eBLACK)
			{
				stpSibling->eColor = eRED;
				stpReplace = stpParent;
			}
			//------------------------------------------------------------------------------
			// ������ ���̰�, ������ ���ڽ��� ����, �����ڽ��� ���尡 �ƴ� ��쿡�� �ش� ���� ����
			// ������ ���̰�, ������ ���ڽ��� ����, �����ڽĵ� ������ ����
			// �̹� ������ ���̰�, ������ �����ڽ��� ������ ��쿡 �ش��ϹǷ�,
			// �Ʒ��� ������ ������ �ʿ� ����
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
			// ������ ���̰�, ������ �����ڽ��� ������ ���, ������ ���ڽ��� ����̵� ��� X
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
		// �θ�������� replacenode�� �������� ���
		//------------------------------------------------------------------------------
		else
		{
			stpSibling = stpParent->stpLeft;

			//------------------------------------------------------------------------------
			// ������ ������ ���
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
			// ������ ���̰�, ������ �� �ڽ��� ��� ���� ���
			//------------------------------------------------------------------------------
			else if (stpSibling->eColor == eBLACK && stpSibling->stpLeft->eColor == eBLACK && stpSibling->stpRight->eColor == eBLACK)
			{
				stpSibling->eColor = eRED;
				stpReplace = stpParent;
			}
			//------------------------------------------------------------------------------
			// ������ ���̰�, ������ �����ڽ� ����, ���ڽ��� ���尡 �ƴ� ��쿡�� �ش� ���� ����
			// ������ ���̰�, ������ �����ڽ� ����, ���ڽĵ� ������ ����
			// �̹� ������ ���̰�, ������ ���ڽ��� ������ ��쿡 �ش��ϹǷ�,
			// �Ʒ��� ������ ������ �ʿ� ����
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
			// ������ ���̰�, ������ ���ڽ��� ������ ���, ������ �����ڽ��� ����̵� ��� X
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
	// ���� ����� ������ ��尡 ���� �� ���� ����Լ� ȣ��.
	//------------------------------------------------------------------------------
	if (stpCurrent->stpRight != m_stpNillNode)
	{
		stpMoveNode = FindNodeForDelete(stpCurrent, stpCurrent->stpRight);
	}
	//------------------------------------------------------------------------------
	// ���� ����� ������ ��尡 ������, 
	// ���� ����� ���� ��带 �θ� ���� ���� ���� ��带 ����
	//------------------------------------------------------------------------------
	else
	{
		//------------------------------------------------------------------------------
		// root ����� ���� �ڽ��� ������(������ ���)�̸�, 
		// root�� ���� ���� �������� ������ ����
		// root ����� ������ �ڽ��� ������(������ ���)�̸�, 
		// root�� ������ ���� �������� ������ ����
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
#pragma once

#include "AStar.h"


class CRedBlackTree
{
public:
	enum E_NODECOLOR
	{
		eRED = 0,
		eBLACK
	};


public:
	struct ST_RBNODE
	{
		ST_RBNODE					*stpParent;
		ST_RBNODE					*stpLeft;
		ST_RBNODE					*stpRight;

		E_NODECOLOR					eColor;
		CAStar::ST_TILENODE			*stpTileAddr;
	};

public:
	CRedBlackTree();
	~CRedBlackTree();

	BOOL InsertNode(CAStar::ST_TILENODE *stpInsertAddr);
	CAStar::ST_TILENODE* InorderIter(int iTileX, int iTileY);

	/*
	BOOL DeleteNode(CAStar::ST_TILENODE *stpDeletetAddr);
	void PreorderPrint();
	void InorderPrint();
	void PostorderPrint();
	*/

private:
	void Init();
	void Release(ST_RBNODE *stpRoot);
	BOOL Link_Node(ST_RBNODE *stpRoot, ST_RBNODE *stpNew);
	void Balance_Insert(ST_RBNODE *stpNew);
	void Rotation_Left(ST_RBNODE *stpRoot);
	void Rotation_Right(ST_RBNODE *stpRoot);
	CAStar::ST_TILENODE* Inorder(ST_RBNODE *stpRoot, int iTileX, int iTileY);

	/*
	void Balance_Delete(ST_RBNODE *stpDelete);
	ST_RBNODE* Delete(CAStar::ST_TILENODE *stpDeletetAddr, ST_RBNODE *stpRoot, ST_RBNODE *stpCurrent);
	ST_RBNODE* FindNodeForDelete(ST_RBNODE *stpRoot, ST_RBNODE *stpCurrent);
	void Preorder(ST_RBNODE *stpRoot, int iDepth);
	void Inorder(ST_RBNODE *stpRoot, int iDepth);
	void Postorder(ST_RBNODE *stpRoot, int iDepth);
	*/

private:
	ST_RBNODE	*m_stpRoot;
	ST_RBNODE	*m_stpNillNode;
};
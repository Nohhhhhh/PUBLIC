#pragma once 

#include <map>
#include <algorithm> 
#include <iterator>

using namespace std;

#define		df_BESTROUTEMAX	30

#define		df_RECTINTERVAL	7
#define		df_ARRAYMAPX	400//266//400		
#define		df_ARRAYMAPY	200//140//200

#define		df_JUMPBRUSH1	0
#define		df_JUMPBRUSH2	1
#define		df_JUMPBRUSH3	2
#define		df_JUMPBRUSH4	3
#define		df_JUMPBRUSH5	4
#define		df_JUMPBRUSH6	5
#define		df_JUMPBRUSH7	6
#define		df_JUMPBRUSH8	7
#define		df_JUMPBRUSH9	8
#define		df_JUMPBRUSH10	9

#define		df_NONBLOCK		10
#define		df_BLOCK		11
#define		df_OPENLIST		12
#define		df_CLOSELIST	13

#define		df_UU			14
#define		df_UR			15
#define		df_RR			16
#define		df_RD			17
#define		df_DD			18
#define		df_DL			19
#define		df_LL			20
#define		df_LU			21

#define		dfDEPTH_MAX			100
#define		dfLIST_MAX			1000

class CJumpPoint
{
public:
	enum e_BRUSH
	{
		eSTART = 0,
		eEND,
		eBLOCK,
		eMOVE,
		eMOVERANGE,
		eOPENLIST,
		eCLOSELIST
	};

	enum e_PEN
	{
		eGRAYPEN = 0,
		eREDPEN
	};

public:
	enum en_MAP
	{
		// ������ ���� ����.
		en_BUFFER_SIZE = 1000000
	};

	struct st_BESTROUTE
	{
		int iX;
		int iY;
	};

	struct st_TILENODE
	{
		int iX;
		int iY;
		int fH;		// ���������� �ܼ��� �Ÿ�. �����Ÿ� �ƴ�. 
		float fG;	// ��������� �� ��ġ���� �̵��� Ÿ���� ����
		float fF;	// ���������� �̵��ϱ⿡ ������ Ÿ�������� ���� �� (fH + fG)

		st_TILENODE *stpParent;
	};

	//--------------------------------------------------------------
	// ������ ��
	//--------------------------------------------------------------
	//multimap <float, st_TILENODE *> m_mmOpenList;
	st_TILENODE *	m_pOpenList[dfLIST_MAX];
	int				m_iOpenListNum;

	//--------------------------------------------------------------
	// ������ ��
	//--------------------------------------------------------------
	//multimap <float, st_TILENODE *> m_mmCloseList;
	//CRedBlackTree					*m_stpCloseList;
	st_TILENODE *	m_pCloseList[dfLIST_MAX];
	int				m_iCloseListNum;

public:
	BOOL						m_bCheckStart;
	BOOL						m_bCheckEnd;

	BYTE						m_byTileAttribute[df_ARRAYMAPY][df_ARRAYMAPX];
	BYTE						m_byOriginTileAttribute[df_ARRAYMAPY][df_ARRAYMAPX];
	int							m_iTempStartX;
	int							m_iTempStartY;
	int							m_iStartX;
	int							m_iStartY;
	int							m_iTempEndX;
	int							m_iTempEndY;
	int							m_iEndX;
	int							m_iEndY;

	int							m_iJumpIdx;
	st_TILENODE					*m_stpDest;
	int							m_iMaxLoop;
	int							m_iLoopCnt;
	//int							m_iDepth;

public:
	HDC							h_MemDC;
	HBITMAP						h_MyBitMap;
	HPEN						*h_pMyPen;
	HBRUSH						*h_pBrush;
	HBRUSH						*h_pJumpBrush;
	HBRUSH						h_OldBrush;

public:
	//-------------------------------------------------
	// ���� ... �ε�� ������ �˴� �д´�.
	//-------------------------------------------------
	WCHAR						*m_wpBuffer;
	int							m_iLoadSize;

	st_BESTROUTE				m_BestRoute[df_BESTROUTEMAX];
	int							m_iBestRoutNodeNum;

public:
	//-----------------------------------------------------------------------
	// Func	  : CJumpPoint()
	// return : none
	//
	// ������
	//-----------------------------------------------------------------------
	CJumpPoint(int iMaxLoop);

	//-----------------------------------------------------------------------
	// Func	  : ~CJumpPoint()
	// return : none
	//
	// �Ҹ���
	//-----------------------------------------------------------------------
	~CJumpPoint();

	//-----------------------------------------------------------------------
	// Func	  : InitTile(void)
	// return : void
	//
	// 2���� Ÿ�� �� �� ��� ���� �ʱ�ȭ
	//-----------------------------------------------------------------------
	void InitTile(void);

	//-----------------------------------------------------------------------
	// Func	  : CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
	// return : void
	//
	// �̵� �Ұ����� Ÿ�� üũ
	//-----------------------------------------------------------------------
	void CheckBlockPoint(int iXPos, int iYPos, BOOL bButton);

	//-----------------------------------------------------------------------
	// Func	  : MakeStartNode(int iStarX, int iStartY, int iEndX, int iEndY)
	// return : void
	//
	// ��� ������ ���� ��� ���� �� �ʱ�ȭ
	//-----------------------------------------------------------------------
	void MakeStartNode(int iStartX, int iStartY, int iEndX, int iEndY);

	//-----------------------------------------------------------------------
	// Func	  : FindTargetSpot(HWND hWnd)
	// return : BOOL
	//
	// �� ĭ �̵��� ������ ��� �� ����ġ ���� ���� ���� �� ã��
	//-----------------------------------------------------------------------
	BOOL FindTargetSpot(void);

	void SetBestRout(st_BESTROUTE *pBestRoute);

	bool LoadGameMap(WCHAR *szFilePath);
	bool LoadFile(WCHAR  *szFilePath);

	void InitTile_Loop(void);

private:
	//-----------------------------------------------------------------------
	// Func	  : InitGDI(void)
	// return : void
	//
	// GDI ������Ʈ ���� �ʱ�ȭ 
	//-----------------------------------------------------------------------
	void InitGDI(void);

	//-----------------------------------------------------------------------
	// Func	  : CheckNeighbour(ST_TILENODE *stpCurrent)
	// return : void
	//
	// 8���⿡ ���� �̵��� ������ ������� Ȯ��
	//-----------------------------------------------------------------------
	void CheckNeighbour(st_TILENODE *stpCurrent);

	//-----------------------------------------------------------------------
	// Func	  : CheckTile(int iCurrentX, int iCurrentY)
	// return : BOOL
	//
	// �ش� ��ǥ�� block ��ǥ����, ������ ������ ���� ��ǥ���� Ȯ��
	//-----------------------------------------------------------------------
	//BOOL CheckTile(int iCurrentX, int iCurrentY);

	//-----------------------------------------------------------------------
	// Func	  : MakeNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY, int iDir)
	// return : BOOL
	//
	// JumpNode�Լ� ȣ�� ��, ������ �� ����� �ش� �Լ����� ��� �˻� �� ����
	//-----------------------------------------------------------------------
	BOOL MakeNode(st_TILENODE *stpCurrent, int iCurrentX, int iCurrentY, int iDir);

	//-----------------------------------------------------------------------
	// Func	  : JumpNode(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir)
	// return : BOOL
	//
	// jump �Ϸ��� ��忡 ���� �˻縦 ����. 
	// ��� ȣ���� �ؾ� �ϴ� �Լ�.
	// ���̴� �κ��� �ִ��� / ��� jump�� �ؾ� �ϴ��� ��..
	//-----------------------------------------------------------------------
	BOOL JumpNode(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir);
	BOOL JumpNode_UU(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	BOOL JumpNode_UR(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	BOOL JumpNode_RR(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	BOOL JumpNode_RD(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	BOOL JumpNode_DD(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	BOOL JumpNode_DL(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	BOOL JumpNode_LL(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	BOOL JumpNode_LU(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG);
	
	//-----------------------------------------------------------------------
	// Func	  : CheckJumpNode(int iCurrentX, int iCurrentY, int iDir)
	// return : BOOL
	//
	// jump �Ϸ��� ���⿡ ���� ���̴� �κ��� �ִ��� üũ
	//-----------------------------------------------------------------------
	//BOOL CheckJumpNode(int iCurrentX, int iCurrentY, int iDir);

	//BOOL JumpNode_Dir(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir);
public:
	//-----------------------------------------------------------------------
	// Func	  : DrawTile(HWND hWnd, HDC hdc)
	// return : void
	//
	// Ÿ�� �׸���
	//-----------------------------------------------------------------------
	void DrawTile(HWND hWnd, HDC hdc);

	//-----------------------------------------------------------------------
	// Func	  : DrawProcessOfMovement(HWND hWnd, HDC hdc)
	// return : void
	//
	// �� ã�� ���� �׸���
	//-----------------------------------------------------------------------
	void DrawProcessOfMovement(HWND hWnd, HDC hdc);

	//-----------------------------------------------------------------------
	// Func	  : DrawBestRoute(HWND hWnd, HDC hdc, ST_TILENODE *stpDest)
	// return : void
	//
	// ����ȭ�� ��Ʈ�� �׷���
	//-----------------------------------------------------------------------
	void DrawBestRoute(HWND hWnd, HDC hdc, st_TILENODE *stpDest);

	//-----------------------------------------------------------------------
	// Func	  : DrawBitMap(HWND hWnd, HDC hdc)
	// return : void
	//
	// ��Ʈ�� ���
	// �޸� DC���� ȭ�� DC�� ��� ���� ����
	//-----------------------------------------------------------------------
	void DrawBitMap(HWND hWnd, HDC hdc);
};
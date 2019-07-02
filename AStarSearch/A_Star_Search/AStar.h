#pragma once 

#include <map>
#include <algorithm> 
#include <iterator>

class CRedBlackTree;

using namespace std;

#define		df_RECTINTERVAL	7
#define		df_ARRAYMAPX	266//63		
#define		df_ARRAYMAPY	200//33
#define		df_NONBLOCK		0
#define		df_BLOCK		1

class CAStar
{
public:
	enum e_BRUSH
	{
		eSTART = 0,
		eEND,
		eBLOCK,
		eMOVE,
		eMOVERANGE
	};

	enum e_PEN
	{
		eGRAYPEN = 0,
		eREDPEN
	};

public:
	struct ST_TILENODE
	{
		int iX;
		int iY;	
		int fH;		// ���������� �ܼ��� �Ÿ�. �����Ÿ� �ƴ�. 
		float fG;	// ��������� �� ��ġ���� �̵��� Ÿ���� ����
		float fF;	// ���������� �̵��ϱ⿡ ������ Ÿ�������� ���� �� (fH + fG)

		ST_TILENODE *stpParent;	// 
	};

	//--------------------------------------------------------------
	// ������ ��
	//--------------------------------------------------------------
	multimap <float, ST_TILENODE *> m_mmOpenList;
	
	//--------------------------------------------------------------
	// ������ ��
	//--------------------------------------------------------------
	multimap <float, ST_TILENODE *> m_mmCloseList;

public:
	BOOL						m_bCheckStart;
	BOOL						m_bCheckEnd;

	BYTE						m_bTileArr[df_ARRAYMAPY][df_ARRAYMAPX];
    BYTE						m_byOriginTileAttribute[df_ARRAYMAPY][df_ARRAYMAPX];
	int							m_iTempStartX;
	int							m_iTempStartY;
	int							m_iStartX;
	int							m_iStartY;
	int							m_iTempEndX;
	int							m_iTempEndY;
	int							m_iEndX;
	int							m_iEndY;

	ST_TILENODE					*m_stpDest;

    WCHAR						*m_wpBuffer;
	int							m_iLoadSize;

public:
	HDC							h_MemDC;
	HBITMAP						h_MyBitMap;
	HPEN						*h_pMyPen;
	HBRUSH						*h_pBrush;
	HBRUSH						h_OldBrush;

public:
	//-----------------------------------------------------------------------
	// Func	  : CAStar()
	// return : none
	//
	// ������
	//-----------------------------------------------------------------------
	CAStar();

	//-----------------------------------------------------------------------
	// Func	  : ~CAStar()
	// return : none
	//
	// �Ҹ���
	//-----------------------------------------------------------------------
	~CAStar();

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
	void MakeStartNode(int iStarX, int iStartY, int iEndX, int iEndY);

	//-----------------------------------------------------------------------
	// Func	  : FindTargetSpot(HWND hWnd)
	// return : BOOL
	//
	// �� ĭ �̵��� ������ ��� �� ����ġ ���� ���� ���� �� ã��
	//-----------------------------------------------------------------------
	BOOL FindTargetSpot(HWND hWnd);

private:
    bool LoadMap(WCHAR *szFilePath);
	bool LoadFile(WCHAR  *szFilePath);

	//-----------------------------------------------------------------------
	// Func	  : InitGDI(void)
	// return : void
	//
	// GDI ������Ʈ ���� �ʱ�ȭ 
	//-----------------------------------------------------------------------
	void InitGDI(void);

	//-----------------------------------------------------------------------
	// Func	  : MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY)
	// return : void
	//
	// 8������ ��忡 ���ؼ� ���� ������ ��� ������ Ȯ��
	// stpCurrent ���� ������ �Ǵ� ����̰�,
	// ������ ���� ����� parent�� stpCurrent ��尡 ��
	//-----------------------------------------------------------------------
	void MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY);

	//-----------------------------------------------------------------------
	// Func	  : MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState)
	// return : void
	//
	// ��� ����
	// stpCurrent ���� ������ �Ǵ� ����̰�,
	// ������ ���� ����� parent�� stpCurrent ��尡 ��
	//-----------------------------------------------------------------------
	void CAStar::MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState);

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
	void DrawBestRoute(HWND hWnd, HDC hdc, ST_TILENODE *stpDest);

	//-----------------------------------------------------------------------
	// Func	  : DrawBitMap(HWND hWnd, HDC hdc)
	// return : void
	//
	// ��Ʈ�� ���
	// �޸� DC���� ȭ�� DC�� ��� ���� ����
	//-----------------------------------------------------------------------
	void DrawBitMap(HWND hWnd, HDC hdc);
};
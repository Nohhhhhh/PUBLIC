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
		int fH;		// 목적지까지 단순한 거리. 직선거리 아님. 
		float fG;	// 출발점에서 현 위치까지 이동한 타일의 갯수
		float fF;	// 목적지까지 이동하기에 최적의 타일인지에 대한 값 (fH + fG)

		ST_TILENODE *stpParent;	// 
	};

	//--------------------------------------------------------------
	// 지나갈 길
	//--------------------------------------------------------------
	multimap <float, ST_TILENODE *> m_mmOpenList;
	
	//--------------------------------------------------------------
	// 지나온 길
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
	// 생성자
	//-----------------------------------------------------------------------
	CAStar();

	//-----------------------------------------------------------------------
	// Func	  : ~CAStar()
	// return : none
	//
	// 소멸자
	//-----------------------------------------------------------------------
	~CAStar();

	//-----------------------------------------------------------------------
	// Func	  : InitTile(void)
	// return : void
	//
	// 2차원 타일 맵 및 멤버 변수 초기화
	//-----------------------------------------------------------------------
	void InitTile(void);

	//-----------------------------------------------------------------------
	// Func	  : CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
	// return : void
	//
	// 이동 불가능한 타일 체크
	//-----------------------------------------------------------------------
	void CheckBlockPoint(int iXPos, int iYPos, BOOL bButton);

	//-----------------------------------------------------------------------
	// Func	  : MakeStartNode(int iStarX, int iStartY, int iEndX, int iEndY)
	// return : void
	//
	// 출발 지점에 대한 노드 생성 및 초기화
	//-----------------------------------------------------------------------
	void MakeStartNode(int iStarX, int iStartY, int iEndX, int iEndY);

	//-----------------------------------------------------------------------
	// Func	  : FindTargetSpot(HWND hWnd)
	// return : BOOL
	//
	// 한 칸 이동이 가능한 노드 중 가중치 값이 가장 작은 것 찾기
	//-----------------------------------------------------------------------
	BOOL FindTargetSpot(HWND hWnd);

private:
    bool LoadMap(WCHAR *szFilePath);
	bool LoadFile(WCHAR  *szFilePath);

	//-----------------------------------------------------------------------
	// Func	  : InitGDI(void)
	// return : void
	//
	// GDI 오브젝트 관련 초기화 
	//-----------------------------------------------------------------------
	void InitGDI(void);

	//-----------------------------------------------------------------------
	// Func	  : MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY)
	// return : void
	//
	// 8방향의 노드에 대해서 생성 가능한 노드 들인지 확인
	// stpCurrent 노드는 기준이 되는 노드이고,
	// 앞으로 만들 노드의 parent가 stpCurrent 노드가 됨
	//-----------------------------------------------------------------------
	void MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY);

	//-----------------------------------------------------------------------
	// Func	  : MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState)
	// return : void
	//
	// 노드 생성
	// stpCurrent 노드는 기준이 되는 노드이고,
	// 앞으로 만들 노드의 parent가 stpCurrent 노드가 됨
	//-----------------------------------------------------------------------
	void CAStar::MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState);

public:
	//-----------------------------------------------------------------------
	// Func	  : DrawTile(HWND hWnd, HDC hdc)
	// return : void
	//
	// 타일 그리기
	//-----------------------------------------------------------------------
	void DrawTile(HWND hWnd, HDC hdc);

	//-----------------------------------------------------------------------
	// Func	  : DrawProcessOfMovement(HWND hWnd, HDC hdc)
	// return : void
	//
	// 길 찾기 과정 그리기
	//-----------------------------------------------------------------------
	void DrawProcessOfMovement(HWND hWnd, HDC hdc);

	//-----------------------------------------------------------------------
	// Func	  : DrawBestRoute(HWND hWnd, HDC hdc, ST_TILENODE *stpDest)
	// return : void
	//
	// 최적화된 루트를 그려줌
	//-----------------------------------------------------------------------
	void DrawBestRoute(HWND hWnd, HDC hdc, ST_TILENODE *stpDest);

	//-----------------------------------------------------------------------
	// Func	  : DrawBitMap(HWND hWnd, HDC hdc)
	// return : void
	//
	// 비트맵 출력
	// 메모리 DC에서 화면 DC로 고속 복사 수행
	//-----------------------------------------------------------------------
	void DrawBitMap(HWND hWnd, HDC hdc);
};
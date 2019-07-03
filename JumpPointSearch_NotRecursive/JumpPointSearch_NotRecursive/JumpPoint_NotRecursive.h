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
		// 구역을 읽을 버퍼.
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
		int fH;		// 목적지까지 단순한 거리. 직선거리 아님. 
		float fG;	// 출발점에서 현 위치까지 이동한 타일의 갯수
		float fF;	// 목적지까지 이동하기에 최적의 타일인지에 대한 값 (fH + fG)

		st_TILENODE *stpParent;
	};

	//--------------------------------------------------------------
	// 지나갈 길
	//--------------------------------------------------------------
	//multimap <float, st_TILENODE *> m_mmOpenList;
	st_TILENODE *	m_pOpenList[dfLIST_MAX];
	int				m_iOpenListNum;

	//--------------------------------------------------------------
	// 지나온 길
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
	// 버퍼 ... 로드시 파일을 죄다 읽는다.
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
	// 생성자
	//-----------------------------------------------------------------------
	CJumpPoint(int iMaxLoop);

	//-----------------------------------------------------------------------
	// Func	  : ~CJumpPoint()
	// return : none
	//
	// 소멸자
	//-----------------------------------------------------------------------
	~CJumpPoint();

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
	void MakeStartNode(int iStartX, int iStartY, int iEndX, int iEndY);

	//-----------------------------------------------------------------------
	// Func	  : FindTargetSpot(HWND hWnd)
	// return : BOOL
	//
	// 한 칸 이동이 가능한 노드 중 가중치 값이 가장 작은 것 찾기
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
	// GDI 오브젝트 관련 초기화 
	//-----------------------------------------------------------------------
	void InitGDI(void);

	//-----------------------------------------------------------------------
	// Func	  : CheckNeighbour(ST_TILENODE *stpCurrent)
	// return : void
	//
	// 8방향에 대한 이동이 가능한 노드인지 확인
	//-----------------------------------------------------------------------
	void CheckNeighbour(st_TILENODE *stpCurrent);

	//-----------------------------------------------------------------------
	// Func	  : CheckTile(int iCurrentX, int iCurrentY)
	// return : BOOL
	//
	// 해당 좌표가 block 좌표인지, 실제로 생성이 가능 좌표인지 확인
	//-----------------------------------------------------------------------
	//BOOL CheckTile(int iCurrentX, int iCurrentY);

	//-----------------------------------------------------------------------
	// Func	  : MakeNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY, int iDir)
	// return : BOOL
	//
	// JumpNode함수 호출 후, 만들어야 할 노드라면 해당 함수에서 노드 검사 후 생성
	//-----------------------------------------------------------------------
	BOOL MakeNode(st_TILENODE *stpCurrent, int iCurrentX, int iCurrentY, int iDir);

	//-----------------------------------------------------------------------
	// Func	  : JumpNode(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir)
	// return : BOOL
	//
	// jump 하려는 노드에 대한 검사를 진행. 
	// 재귀 호출을 해야 하는 함수.
	// 꺽이는 부분이 있는지 / 계속 jump를 해야 하는지 등..
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
	// jump 하려는 방향에 따라서 꺽이는 부분이 있는지 체크
	//-----------------------------------------------------------------------
	//BOOL CheckJumpNode(int iCurrentX, int iCurrentY, int iDir);

	//BOOL JumpNode_Dir(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir);
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
	void DrawBestRoute(HWND hWnd, HDC hdc, st_TILENODE *stpDest);

	//-----------------------------------------------------------------------
	// Func	  : DrawBitMap(HWND hWnd, HDC hdc)
	// return : void
	//
	// 비트맵 출력
	// 메모리 DC에서 화면 DC로 고속 복사 수행
	//-----------------------------------------------------------------------
	void DrawBitMap(HWND hWnd, HDC hdc);
};
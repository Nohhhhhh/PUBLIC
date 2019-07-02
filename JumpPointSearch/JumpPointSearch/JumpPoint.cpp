#include "stdafx.h"
#include "JumpPoint.h"

using namespace std;

//-----------------------------------------------------------------------
// Func	  : CJumpPoint()
// return : none
//
// 생성자
//-----------------------------------------------------------------------
CJumpPoint::CJumpPoint(int iMaxLoop)
{
	LoadGameMap(L".\\MAP\\Map.txt");
	InitTile();
	InitGDI();

	m_iMaxLoop = iMaxLoop;
	m_iLoopCnt = 0;
}

//-----------------------------------------------------------------------
// Func	  : ~CJumpPoint()
// return : none
//
// 소멸자
//-----------------------------------------------------------------------
CJumpPoint::~CJumpPoint()
{
	delete[] h_pMyPen;
	delete[] h_pBrush;
}

//-----------------------------------------------------------------------
// Func	  : InitTile(void)
// return : void
//
// 2차원 타일 맵 및 멤버 변수 초기화
//-----------------------------------------------------------------------
void CJumpPoint::InitTile(void)
{
	//int iCnt;
	//
	//for (iCnt = 0; iCnt < df_ARRAYMAPY; iCnt++)
	//{
	//	memset(m_bTileArr[iCnt], df_NONBLOCK, sizeof(char) * df_ARRAYMAPX);
	//}
	memset(m_BestRoute, 0, sizeof(m_BestRoute));

	memcpy_s(m_byTileAttribute, sizeof(m_byTileAttribute), m_byOriginTileAttribute, sizeof(m_byOriginTileAttribute));

	m_iBestRoutNodeNum = 0;

	m_bCheckStart = FALSE;
	m_iTempStartX = -1;
	m_iTempStartY = -1;
	m_iStartX = -1;
	m_iStartY = -1;

	m_bCheckEnd = FALSE;
	m_iTempEndX = -1;
	m_iTempEndY = -1;
	m_iEndX = -1;
	m_iEndY = -1;

	m_stpDest = NULL;


	if (0 < m_mmOpenList.size())
	{
		/*map<float, st_TILENODE *>::iterator OpenListIter = m_mmOpenList.begin();
		while (OpenListIter != m_mmOpenList.end())
		{
		m_byTileAttribute[OpenListIter->second->iY][OpenListIter->second->iX] = en_TILE_ATTRIBUTE::en_NONBLOCK;
		delete OpenListIter->second;
		m_mmOpenList.erase(OpenListIter++);
		}*/
		for_each(m_mmOpenList.begin(), m_mmOpenList.end(), [&](pair<float, st_TILENODE *> OpenList)
		{
			m_byTileAttribute[OpenList.second->iY][OpenList.second->iX] = df_NONBLOCK;
			delete OpenList.second;
		});

		m_mmOpenList.clear();
	}

	if (0 < m_mmCloseList.size())
	{
		/*map<float, st_TILENODE *>::iterator CloseListIter = m_mmCloseList.begin();
		while (CloseListIter != m_mmCloseList.end())
		{
		m_byTileAttribute[CloseListIter->second->iY][CloseListIter->second->iX] = en_TILE_ATTRIBUTE::en_NONBLOCK;
		delete CloseListIter->second;
		m_mmCloseList.erase(CloseListIter++);
		}*/

		for_each(m_mmCloseList.begin(), m_mmCloseList.end(), [&](pair<float, st_TILENODE *> CloseList)
		{
			m_byTileAttribute[CloseList.second->iY][CloseList.second->iX] = df_NONBLOCK;
			delete CloseList.second;
		});

		m_mmCloseList.clear();
	}
	//m_stpCloseList = new CRedBlackTree;
}

//-----------------------------------------------------------------------
// Func	  : InitGDI(void)
// return : void
//
// GDI 오브젝트 관련 초기화 
//-----------------------------------------------------------------------
void CJumpPoint::InitGDI(void)
{
	//--------------------------------------------------------------------------
	// 펜 초기화	
	//--------------------------------------------------------------------------
	h_pMyPen = new HPEN[2];
	h_pMyPen[eGRAYPEN] = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
	h_pMyPen[eREDPEN] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	//--------------------------------------------------------------------------
	// 브러쉬 초기화	
	//--------------------------------------------------------------------------
	h_pBrush = new HBRUSH[7];
	h_pBrush[eSTART] = CreateSolidBrush(RGB(255, 0, 0));
	h_pBrush[eEND] = CreateSolidBrush(RGB(0, 255, 0));
	//h_pBrush[eBLOCK] = CreateSolidBrush(RGB(153, 153, 153));
	h_pBrush[eBLOCK] = CreateSolidBrush(RGB(0, 0, 0));
	h_pBrush[eMOVE] = CreateSolidBrush(RGB(255, 255, 153));
	h_pBrush[eMOVERANGE] = CreateSolidBrush(RGB(0, 0, 255));
	h_pBrush[eOPENLIST] = CreateSolidBrush(RGB(255, 255, 0));
	h_pBrush[eCLOSELIST] = CreateSolidBrush(RGB(0, 0, 255));

	//--------------------------------------------------------------------------
	// JUMP 전용 BRUSH 초기화
	//--------------------------------------------------------------------------
	h_pJumpBrush = new HBRUSH[10];
	h_pJumpBrush[df_JUMPBRUSH1] = CreateSolidBrush(RGB(102, 102, 51));
	h_pJumpBrush[df_JUMPBRUSH2] = CreateSolidBrush(RGB(102, 102, 204));
	h_pJumpBrush[df_JUMPBRUSH3] = CreateSolidBrush(RGB(102, 153, 102));
	h_pJumpBrush[df_JUMPBRUSH4] = CreateSolidBrush(RGB(255, 102, 255));
	h_pJumpBrush[df_JUMPBRUSH5] = CreateSolidBrush(RGB(153, 102, 153));
	h_pJumpBrush[df_JUMPBRUSH6] = CreateSolidBrush(RGB(255, 204, 102));
	h_pJumpBrush[df_JUMPBRUSH7] = CreateSolidBrush(RGB(051, 153, 153));
	h_pJumpBrush[df_JUMPBRUSH8] = CreateSolidBrush(RGB(204, 255, 153));
	h_pJumpBrush[df_JUMPBRUSH9] = CreateSolidBrush(RGB(0, 102, 153));
	h_pJumpBrush[df_JUMPBRUSH10] = CreateSolidBrush(RGB(255, 153, 204));
}

//-----------------------------------------------------------------------
// Func	  : DrawTile(HWND hWnd, HDC hdc)
// return : void
//
// 타일 그리기
//-----------------------------------------------------------------------
void CJumpPoint::DrawTile(HWND hWnd, HDC hdc)
{
	CRect cRect;

	GetClientRect(hWnd, &cRect);

	if (h_MemDC == NULL)
		h_MemDC = CreateCompatibleDC(hdc);

	if (h_MyBitMap == NULL)
	{
		h_MyBitMap = CreateCompatibleBitmap(hdc, cRect.right, cRect.bottom);
		SelectObject(h_MemDC, h_MyBitMap);
	}

	SelectObject(h_MemDC, h_pMyPen[eGRAYPEN]);

	h_OldBrush = (HBRUSH)SelectObject(h_MemDC, h_pBrush[eEND]);

	//--------------------------------------------------------------------------
	// PatBlt	:													
	// PatBlt는 직접적으로 비트맵을 출력하는 함수는 아니며 패턴을 출력	
	// 메모리 DC의 화면 색상이 어떤 것인지 모르기 때문에					
	// 메모리 DC의 화면 색상을 흰색으로 변경해준다.						
	//--------------------------------------------------------------------------
	PatBlt(h_MemDC, 0, 0, cRect.right, cRect.bottom, WHITENESS);

	//--------------------------------------------------------------------------
	// 드로잉 모드를 지정						
	// R2_COPYPEN은 PEN으로 지정한 색을 가져옴	
	//--------------------------------------------------------------------------
	SetROP2(h_MemDC, R2_COPYPEN);

	//--------------------------------------------------------------------------
	// 바둑판 모양의 맵을 그린다.
	//--------------------------------------------------------------------------
	for (int iX = 0; iX < df_ARRAYMAPX; iX++)
	{
		for (int iY = 0; iY < df_ARRAYMAPY; iY++)
		{
			
			//--------------------------------------------------------------------------
			// block된 tile 처리
			//--------------------------------------------------------------------------
			if (m_byTileAttribute[iY][iX] == df_BLOCK)
			{
				SelectObject(h_MemDC, h_pBrush[eBLOCK]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			//--------------------------------------------------------------------------
			// nonblock된 tile 처리
			// 단, 시작, 끝 지점도 nonblock이므로 따로 처리 해줘야 함
			//--------------------------------------------------------------------------
			else if (m_byTileAttribute[iY][iX] == df_NONBLOCK)
			{
				if (m_iTempStartX == iX && m_iTempStartY == iY)
					SelectObject(h_MemDC, h_pBrush[eSTART]);
				else if (m_iTempEndX == iX && m_iTempEndY == iY)
					SelectObject(h_MemDC, h_pBrush[eEND]);
				else
					SelectObject(h_MemDC, h_OldBrush);

				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			//--------------------------------------------------------------------------
			// tile은 생성됬지만, 지나간 길은 아닌 tile 처리
			//--------------------------------------------------------------------------
			else if ((m_byTileAttribute[iY][iX] == df_OPENLIST))
			{
				SelectObject(h_MemDC, h_pBrush[eOPENLIST]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			//--------------------------------------------------------------------------
			// openlist에 등록된 것 중 지나간 tile인 closelist에 등록된 tile 처리
			//--------------------------------------------------------------------------
			else if ((m_byTileAttribute[iY][iX] == df_CLOSELIST))
			{
				SelectObject(h_MemDC, h_pBrush[eCLOSELIST]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			//--------------------------------------------------------------------------
			// jump된 tile 그리기
			//--------------------------------------------------------------------------
			else
			{
				SelectObject(h_MemDC, h_pJumpBrush[m_byTileAttribute[iY][iX]]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			
		}
	}

	SelectObject(h_MemDC, h_OldBrush);
}

//-----------------------------------------------------------------------
// Func	  : DrawProcessOfMovement(HWND hWnd, HDC hdc)
// return : void
//
// 길 찾기 과정 그리기
//-----------------------------------------------------------------------
void CJumpPoint::DrawProcessOfMovement(HWND hWnd, HDC hdc)
{
	int iX = 0;
	int iY = 0;
	CRect cRect;

	GetClientRect(hWnd, &cRect);

	if (h_MemDC == NULL)
		h_MemDC = CreateCompatibleDC(hdc);

	if (h_MyBitMap == NULL)
	{
		h_MyBitMap = CreateCompatibleBitmap(hdc, cRect.right, cRect.bottom);
		SelectObject(h_MemDC, h_MyBitMap);
	}

	h_OldBrush = (HBRUSH)SelectObject(h_MemDC, h_pBrush[eOPENLIST]);
	
	//--------------------------------------------------------------------------
	// PatBlt	:													
	// PatBlt는 직접적으로 비트맵을 출력하는 함수는 아니며 패턴을 출력	
	// 메모리 DC의 화면 색상이 어떤 것인지 모르기 때문에					
	// 메모리 DC의 화면 색상을 흰색으로 변경해준다.						
	//--------------------------------------------------------------------------
	//PatBlt(h_MemDC, 0, 0, cRect.right, cRect.bottom, WHITENESS);

	//--------------------------------------------------------------------------
	// 드로잉 모드를 지정						
	// R2_COPYPEN은 PEN으로 지정한 색을 가져옴	
	//--------------------------------------------------------------------------
	SetROP2(h_MemDC, R2_COPYPEN);

	//--------------------------------------------------------------------------
	// 바둑판 모양의 맵을 그린다.
	//--------------------------------------------------------------------------
	for (int iX = 0; iX < df_ARRAYMAPX; iX++)
	{
		for (int iY = 0; iY < df_ARRAYMAPY; iY++)
		{

			if ((m_byTileAttribute[iY][iX] == df_OPENLIST))
			{
				SelectObject(h_MemDC, h_pBrush[eOPENLIST]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			else if ((m_byTileAttribute[iY][iX] == df_CLOSELIST))
			{
				SelectObject(h_MemDC, h_pBrush[eCLOSELIST]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			else if (m_byTileAttribute[iY][iX] != df_BLOCK && m_byTileAttribute[iY][iX] != df_NONBLOCK)
			{
				SelectObject(h_MemDC, h_pJumpBrush[m_byTileAttribute[iY][iX]]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
		}
	}

	SelectObject(h_MemDC, h_OldBrush);
}

//-----------------------------------------------------------------------
// Func	  : DrawBestRoute(HWND hWnd, HDC hdc, ST_TILENODE *stpDest)
// return : void
//
// 최적화된 루트를 그려줌
//-----------------------------------------------------------------------
void CJumpPoint::DrawBestRoute(HWND hWnd, HDC hdc, st_TILENODE *stpDest)
{
	CRect cRect;

	GetClientRect(hWnd, &cRect);

	if (h_MemDC == NULL)
		h_MemDC = CreateCompatibleDC(hdc);

	if (h_MyBitMap == NULL)
	{
		h_MyBitMap = CreateCompatibleBitmap(hdc, cRect.right, cRect.bottom);
		SelectObject(h_MemDC, h_MyBitMap);
	}

	SelectObject(h_MemDC, h_pMyPen[eREDPEN]);

	//--------------------------------------------------------------------------
	// PatBlt	:													
	// PatBlt는 직접적으로 비트맵을 출력하는 함수는 아니며 패턴을 출력	
	// 메모리 DC의 화면 색상이 어떤 것인지 모르기 때문에					
	// 메모리 DC의 화면 색상을 흰색으로 변경해준다.						
	//--------------------------------------------------------------------------
	//PatBlt(h_MemDC, 0, 0, cRect.right, cRect.bottom, WHITENESS);

	//--------------------------------------------------------------------------
	// 드로잉 모드를 지정						
	// R2_COPYPEN은 PEN으로 지정한 색을 가져옴	
	//--------------------------------------------------------------------------
	SetROP2(h_MemDC, R2_COPYPEN);

	while (1)
	{
		//--------------------------------------------------------------------------
		// 검색 도중에 검색 초기화가 되면 while 문 빠져 나와야 함.
		//--------------------------------------------------------------------------
		if (NULL == stpDest)
			return;

		if (stpDest->iX == m_iStartX && stpDest->iY == m_iStartY)
			break;

		MoveToEx(h_MemDC, stpDest->iX * df_RECTINTERVAL + df_RECTINTERVAL / 2, stpDest->iY * df_RECTINTERVAL + df_RECTINTERVAL / 2, NULL);
		LineTo(h_MemDC, stpDest->stpParent->iX * df_RECTINTERVAL + df_RECTINTERVAL / 2, stpDest->stpParent->iY * df_RECTINTERVAL + df_RECTINTERVAL / 2);

		stpDest = stpDest->stpParent;
	}

	//--------------------------------------------------------------------------
	// ReleaseDC가 필요가 없다. 이 후에 최선의 길에 대한 라인을 그려줘야 하기 때문에.
	//--------------------------------------------------------------------------
	//ReleaseDC(hWnd, hdc);
}

//-----------------------------------------------------------------------
// Func	  : DrawBitMap(HWND hWnd, HDC hdc)
// return : void
//
// 비트맵 출력
// 메모리 DC에서 화면 DC로 고속 복사 수행
//-----------------------------------------------------------------------
void CJumpPoint::DrawBitMap(HWND hWnd, HDC hdc)
{
	CRect cRect;

	GetClientRect(hWnd, &cRect);
	SelectObject(h_MemDC, h_MyBitMap);

	// 메모리 DC에서 화면 DC로 고속 복사 수행
	BitBlt(hdc, 0, 0, cRect.right, cRect.bottom, h_MemDC, 0, 0, SRCCOPY);
}

//-----------------------------------------------------------------------
// Func	  : CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
// return : void
//
// 이동 불가능한 타일 체크
//-----------------------------------------------------------------------
void CJumpPoint::CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
{
	//--------------------------------------------------------------------------
	// 마우스 왼쪽 버튼을 클릭 했는지 오른쪽 버튼을 클릭 했는지에 따라서
	// bButton의 값이 TRUE와 FALSE로 정해진다.
	// TRUE라면, 왼쪽 버튼이기 때문에 전부 block 처리
	// FASLE라면, 오른쪽 버튼이기 때문에 접누 nonblock 처리
	//--------------------------------------------------------------------------
	if (bButton == TRUE)
		m_byTileAttribute[iYPos][iXPos] = df_BLOCK;
	else
		m_byTileAttribute[iYPos][iXPos] = df_NONBLOCK;
}

//-----------------------------------------------------------------------
// Func	  : MakeStartNode(int iStarX, int iStartY, int iEndX, int iEndY)
// return : void
//
// 출발 지점에 대한 노드 생성 및 초기화
//----------------------------------------------------------------------
void CJumpPoint::MakeStartNode(int iStartX, int iStartY, int iEndX, int iEndY)
{
	st_TILENODE *stpNew = new st_TILENODE;

	m_iLoopCnt = 0;

	//--------------------------------------------------------------------------
	// 기존에 검색을 한 번 한 후, 시작 지점과 종료 지점을 옮기면
	// 멤버 시작/종료 좌표가 변한다. 그런대 이 멤버 시작/종료 좌표는 
	// 길 찾기 시작 전/후 항상 참조를 하는 부분이 있기 때문에
	// 시작/종료 지점을 찍을 때는 임시 멤버변수에 담은 후에
	// 길찾기를 시작할 때 비로소 실제 시작/종료 멤버 변수에 대입하도록 되어 있음
	// 만약, 임시변수를 두지 않을 경우 길찾기 후에 시작/종료 지점을 바꾸게 되면
	// 표시가 잘못 되거나, 좌표를 찾을 수 없기 때문에 프로그램이 다운됨
	//--------------------------------------------------------------------------
	m_iStartX = iStartX;
	m_iStartY = iStartY;
	m_iEndX = iEndX;
	m_iEndY = iEndY;

	stpNew->iX = m_iStartX;
	stpNew->iY = m_iStartY;
	stpNew->fH = abs(m_iStartX - m_iEndX) + abs(m_iStartY - m_iEndY);
	stpNew->fG = 0;
	stpNew->fF = stpNew->fH + stpNew->fG;
	stpNew->stpParent = NULL;

	//--------------------------------------------------------------------------
	// 오픈리스트 초기화 후, 오픈리스트에 삽입
	//--------------------------------------------------------------------------
	m_mmOpenList.clear();
	m_mmOpenList.insert(pair<float, st_TILENODE *>(stpNew->fF, stpNew));
}

//-----------------------------------------------------------------------
// Func	  : FindTargetSpot(HWND hWnd)
// return : BOOL
//
// 한 칸 이동이 가능한 노드 중 가중치 값이 가장 작은 것 찾기
//-----------------------------------------------------------------------
BOOL CJumpPoint::FindTargetSpot(void)
{
	st_TILENODE *stpTempNode;

	if (m_mmOpenList.empty() == TRUE)
		return FALSE;

	//--------------------------------------------------------------------------
	// f값이 가장 작은 값 뽑기.
	// multimap을 오름차순으로 정렬(default 값)
	// 따라서, 가장 첫 번째의 second 값을 뽑음.
	//--------------------------------------------------------------------------
	stpTempNode = m_mmOpenList.begin()->second;
	m_mmOpenList.erase(m_mmOpenList.begin());
	m_mmCloseList.insert(pair<float, st_TILENODE *>(stpTempNode->fF, stpTempNode));

	//--------------------------------------------------------------------------
	// close list에 추가된 노드가 시작 지점 또는 종료 지점이 아니라면, 
	// 해당 좌표의 배열의 속성을 closelist로 변경
	//--------------------------------------------------------------------------
	if ((stpTempNode->iX != m_iStartX || stpTempNode->iY != m_iStartY) && (stpTempNode->iX != m_iEndX || stpTempNode->iY != m_iEndY))
		m_byTileAttribute[stpTempNode->iY][stpTempNode->iX] = df_CLOSELIST;

	//--------------------------------------------------------------------------
	// open list에서 뽑은 노드의 좌표가 실제 end x, y 좌표라면 목적지 노드에 현재 뽑은
	// 노드 포인터 저장하고 return true.
	// return true하면, 해당 함수를 반복하는 것이 끝남.
	//--------------------------------------------------------------------------
	if ((stpTempNode->iX == m_iEndX && stpTempNode->iY == m_iEndY) || m_iLoopCnt == m_iMaxLoop)
	{
		m_stpDest = stpTempNode;

		SetBestRout(m_BestRoute);

		return TRUE;
	}

	//--------------------------------------------------------------------------
	// 8방향에 대한 jump 노드 생성을 위한 함수
	//--------------------------------------------------------------------------
	CheckNeighbour(stpTempNode);

	++m_iLoopCnt;

	return FALSE;
}

void CJumpPoint::SetBestRout(st_BESTROUTE * pBestRoute)
{
	st_TILENODE *stpDest = m_stpDest;
	int iTempBestRoutNodeNum = 0;
	m_iBestRoutNodeNum = 0;

	while (1)
	{
		//--------------------------------------------------------------------------
		// 검색 도중에 검색 초기화가 되면 while 문 빠져 나와야 함.
		//--------------------------------------------------------------------------
		if (NULL == m_stpDest)
			return;

		if (stpDest->iX == m_iStartX && stpDest->iY == m_iStartY)
		{
			m_iBestRoutNodeNum = iTempBestRoutNodeNum;
			stpDest = m_stpDest;

			while (1)
			{
				--iTempBestRoutNodeNum;

				pBestRoute[iTempBestRoutNodeNum].iX = stpDest->iX;
				pBestRoute[iTempBestRoutNodeNum].iY = stpDest->iY;

				stpDest = stpDest->stpParent;

				if (iTempBestRoutNodeNum == 0)
					break;
			}

			break;
		}

		//MoveToEx(h_MemDC, m_stpDest->iX * df_RECTINTERVAL + df_RECTINTERVAL / 2, stpDest->iY * df_RECTINTERVAL + df_RECTINTERVAL / 2, NULL);
		//LineTo(h_MemDC, stpDest->stpParent->iX * df_RECTINTERVAL + df_RECTINTERVAL / 2, stpDest->stpParent->iY * df_RECTINTERVAL + df_RECTINTERVAL / 2);

		stpDest = stpDest->stpParent;
		++iTempBestRoutNodeNum;
	}
}

bool CJumpPoint::LoadGameMap(WCHAR *szFilePath)
{
	for (int iCnt = 0; iCnt < df_ARRAYMAPY; iCnt++)
	{
		memset(m_byOriginTileAttribute[iCnt], df_NONBLOCK, sizeof(char) * df_ARRAYMAPX);
	}

	if (!LoadFile(szFilePath))
	{
		// 실패 로그 & 콘솔 출력
		//wprintf(L"Open File Fail: DB_INFO.ini\n");
		return false;
	}

	int iBufPos = 0;

	int iX = 0;
	int iY = 0;

	while (1)
	{
		// 'X,' 또는 ' X'
		if (m_wpBuffer[iBufPos] == 0x58bf || m_wpBuffer[iBufPos] == 0x2c58 || m_wpBuffer[iBufPos] == 0x5820)
		{
			// 블럭
			m_byOriginTileAttribute[iY][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY][iX + 1] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX + 1] = df_BLOCK;

			// 2칸 이동. 파일과 서버의 비율이 2배 차이 이므로.
			iX += 2;
		
			// x좌표 배열 끝까지 다 썻으며, 다음 노드가 개행인 경우
			if (iX >= df_ARRAYMAPX)
				iX = df_ARRAYMAPX - 1;
		}
		// 'X.' X 후, 개행
		else if (m_wpBuffer[iBufPos] == 0x0d58)
		{
			iX = iX;
			iY = iY;
		
			// 블럭 후, 개행
			m_byOriginTileAttribute[iY][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY][iX + 1] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX + 1] = df_BLOCK;
		
			// 2칸 이동. 파일과 서버의 비율이 2배 차이 이므로.
			iY += 2;
			iX = 0;
		}
		// ' ,' 
		else if (m_wpBuffer[iBufPos] == 0x2c20 || m_wpBuffer[iBufPos] == 0x2020)
		{
			iX = iX;
			iY = iY;
		
			// 논 블럭
			m_byOriginTileAttribute[iY][iX] = df_NONBLOCK;
			m_byOriginTileAttribute[iY][iX + 1] = df_NONBLOCK;
			m_byOriginTileAttribute[iY + 1][iX] = df_NONBLOCK;
			m_byOriginTileAttribute[iY + 1][iX + 1] = df_NONBLOCK;
		
			// 2칸 이동. 파일과 서버의 비율이 2배 차이 이므로.
			iX += 2;
		}
        else if (m_wpBuffer[iBufPos] == 0x0a0d)
        {
            iX = 0;
            iY += 2;
        }

		if (++iBufPos >= m_iLoadSize / 2)
			break;

		// ', '인 0x202c면 패스 
		if(m_wpBuffer[iBufPos] == 0x202c)
			++iBufPos;
	}

	return true;
}

bool CJumpPoint::LoadFile(WCHAR * szFilePath)
{
	m_wpBuffer = new WCHAR[en_MAP::en_BUFFER_SIZE];

	HANDLE hFile;
	DWORD dwRead;

	m_iLoadSize = 0;

	hFile = CreateFile(szFilePath, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL || FILE_FLAG_DELETE_ON_CLOSE, NULL);

	// 파일 오픈 여부
	if (INVALID_HANDLE_VALUE == hFile)
		return false;

	// 파일 크기 검사
	m_iLoadSize = GetFileSize(hFile, NULL);

	// 버퍼보다 크다면
	if (en_MAP::en_BUFFER_SIZE < m_iLoadSize)
	{
		CloseHandle(hFile);

		return false;
	}

	// BOM 코드 삭제
	char cBOMCODE[10] = { NULL };

	// BOM 코드 삭제
	// 3byte만 읽자.
	ReadFile(hFile, cBOMCODE, 3, &dwRead, NULL);

	// 0xef 같이 1byte hex 값은 int형으로 인식 한다.
	// 그리고 비교되는 cMOBCODE가 unsigned char가 아니라면 0xffffffff와 같이 표현 될 것이다.
	// 결국, 0xef는 int형으로 이므로 0x000000ef로 표현될 것이고, char형인 cBOMCODE는 0xffffffef로 표현될 것이기 때문에 서로 다르다고 판단한다.
	// 따라서, unsigned char 형으로 cBOMCODE를 변경하여 0x000000ef와 같이 표현 되도록 하던지, 0xef를 char형으로 형변환 시켜 1byte만 비교하던지 해야 한다.

	if (cBOMCODE[0] != (char)0xef || cBOMCODE[1] != (char)0xbb || cBOMCODE[2] != (char)0xbf)
	{
		CloseHandle(hFile);
		return false;
	}

	m_iLoadSize -= 3;

	// 전부 읽기
	ReadFile(hFile, m_wpBuffer, m_iLoadSize, &dwRead, NULL);

	if (dwRead != (DWORD)m_iLoadSize)
	{
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);


	return true;
}

void CJumpPoint::InitTile_Loop(void)
{
	memset(m_BestRoute, 0, sizeof(m_BestRoute));

	memcpy_s(m_byTileAttribute, sizeof(m_byTileAttribute), m_byOriginTileAttribute, sizeof(m_byOriginTileAttribute));

	m_iBestRoutNodeNum = 0;

	m_iStartX = -1;
	m_iStartY = -1;

	m_iEndX = -1;
	m_iEndY = -1;

	m_stpDest = NULL;


	if (0 < m_mmOpenList.size())
	{
		/*map<float, st_TILENODE *>::iterator OpenListIter = m_mmOpenList.begin();
		while (OpenListIter != m_mmOpenList.end())
		{
		m_byTileAttribute[OpenListIter->second->iY][OpenListIter->second->iX] = en_TILE_ATTRIBUTE::en_NONBLOCK;
		delete OpenListIter->second;
		m_mmOpenList.erase(OpenListIter++);
		}*/
		for_each(m_mmOpenList.begin(), m_mmOpenList.end(), [&](pair<float, st_TILENODE *> OpenList)
		{
			m_byTileAttribute[OpenList.second->iY][OpenList.second->iX] = df_NONBLOCK;
			delete OpenList.second;
		});

		m_mmOpenList.clear();
	}

	if (0 < m_mmCloseList.size())
	{
		/*map<float, st_TILENODE *>::iterator CloseListIter = m_mmCloseList.begin();
		while (CloseListIter != m_mmCloseList.end())
		{
		m_byTileAttribute[CloseListIter->second->iY][CloseListIter->second->iX] = en_TILE_ATTRIBUTE::en_NONBLOCK;
		delete CloseListIter->second;
		m_mmCloseList.erase(CloseListIter++);
		}*/

		for_each(m_mmCloseList.begin(), m_mmCloseList.end(), [&](pair<float, st_TILENODE *> CloseList)
		{
			m_byTileAttribute[CloseList.second->iY][CloseList.second->iX] = df_NONBLOCK;
			delete CloseList.second;
		});

		m_mmCloseList.clear();
	}
}

//-----------------------------------------------------------------------
// Func	  : CheckNeighbour(ST_TILENODE *stpCurrent)
// return : void
//
// 8방향에 대한 이동이 가능한 노드인지 확인
//-----------------------------------------------------------------------
void CJumpPoint::CheckNeighbour(st_TILENODE *stpCurrent)
{
	int iDX = 0;
	int iDY = 0;
	int iCurrentX = stpCurrent->iX;
	int iCurrentY = stpCurrent->iY;
	bool bCheckTile = false;
	//--------------------------------------------------------------------------
	// 부모가 없을 때 8방향에 대한 체크
	//--------------------------------------------------------------------------
	if (stpCurrent->stpParent == NULL)
	{
		// UU
		//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY - 1);
		if (CHECK_TILE(iCurrentX, iCurrentY - 1))
			MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);

		// UR
		//bCheckTile = CHECK_TILE(iCurrentX + 1, iCurrentY - 1);
		if (CHECK_TILE(iCurrentX + 1, iCurrentY - 1))
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);

		// RR --
		//bCheckTile = CHECK_TILE(iCurrentX + 1, iCurrentY);
		if (CHECK_TILE(iCurrentX + 1, iCurrentY))
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);

		// RD
		//bCheckTile = CHECK_TILE(iCurrentX + 1, iCurrentY + 1);
		if (CHECK_TILE(iCurrentX + 1, iCurrentY + 1))
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);

		// DD
		//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY + 1);
		if (CHECK_TILE(iCurrentX, iCurrentY + 1))
			MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, df_DD);

		// DL
		//bCheckTile = CHECK_TILE(iCurrentX - 1, iCurrentY + 1);
		if (CHECK_TILE(iCurrentX - 1, iCurrentY + 1))
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);

		// LL
		//bCheckTile = CHECK_TILE(iCurrentX - 1, iCurrentY);
		if (CHECK_TILE(iCurrentX - 1, iCurrentY))
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);

		// LU
		//bCheckTile = CHECK_TILE(iCurrentX - 1, iCurrentY - 1);
		if (CHECK_TILE(iCurrentX - 1, iCurrentY - 1))
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);

		//// UU
		//if (CheckTile(iCurrentX, iCurrentY - 1))
		//	MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);

		//// UR
		//if (CheckTile(iCurrentX + 1, iCurrentY - 1))
		//	MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);

		//// RR
		//if (CheckTile(iCurrentX + 1, iCurrentY))
		//	MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);

		//// RD
		//if (CheckTile(iCurrentX + 1, iCurrentY + 1))
		//	MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);

		//// DD
		//if (CheckTile(iCurrentX, iCurrentY + 1))
		//	MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, df_DD);

		//// DL
		//if (CheckTile(iCurrentX - 1, iCurrentY + 1))
		//	MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);

		//// LL
		//if (CheckTile(iCurrentX - 1, iCurrentY))
		//	MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);

		//// LU
		//if (CheckTile(iCurrentX - 1, iCurrentY - 1))
		//	MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);
	}
	//--------------------------------------------------------------------------
	// 부모가 있을 때 8방향에 대한 체크
	//--------------------------------------------------------------------------
	else
	{
		//--------------------------------------------------------------------------
		// 부모노드와 현재노드의 기울기를 파악하기 위한 로직
		// 부모노드와 현재노드의 기울기를 통해 해당 방향에 대해서 체크를 함
		// x축과 달리, y축은 상위로 올라갈 수록 수가 적어지기 때문에
		// 부모노드에서 현재노드를 뺌
		//--------------------------------------------------------------------------
		iDX = stpCurrent->iX - stpCurrent->stpParent->iX;
		iDY = stpCurrent->stpParent->iY - stpCurrent->iY;

		if (iDX != 0)
			iDX = abs(iDX) / iDX;

		if (iDY != 0)
			iDY = abs(iDY) / iDY;

		// UU
		if (0 == iDX && 1 == iDY)
		{
			//--------------------------------------------------------------------------
			// 상/하/좌/우 방향은 기본적으로 1방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);

			//--------------------------------------------------------------------------
			// 막힌 부분 체크 후, 막힌 부분이 있다면 꺽일 수 있는 방향에 대해서 추가적으로 노드 생성
			//--------------------------------------------------------------------------
			//bCheckTile = CHECK_TILE(iCurrentX + 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);
			
			//bCheckTile = CHECK_TILE(iCurrentX - 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);

			/*if (!CheckTile(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);

			if (!CheckTile(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);*/
		}
		// UR
		else if (1 == iDX && 1 == iDY)
		{
			//--------------------------------------------------------------------------
			// 대각선 방향은 기본적으로 3방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);
			MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);

			//--------------------------------------------------------------------------
			// 막힌 부분 체크 후, 막힌 부분이 있다면 해당 방향으로 추가적으로 체크
			//--------------------------------------------------------------------------
			//bCheckTile = CHECK_TILE(iCurrentX - 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);

			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY + 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY + 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);

			/*if (!CheckTile(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);

			if (!CheckTile(iCurrentX, iCurrentY + 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);*/
		}
		// RR
		else if (1 == iDX && 0 == iDY)
		{
			//--------------------------------------------------------------------------
			// 상/하/좌/우 방향은 기본적으로 1방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);

			//--------------------------------------------------------------------------
			// 막힌 부분 체크 후, 막힌 부분이 있다면 해당 방향으로 추가적으로 체크
			//--------------------------------------------------------------------------
			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY - 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);
			
			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY + 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY + 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);

			/*if (!CheckTile(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);

			if (!CheckTile(iCurrentX, iCurrentY + 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);*/
		}
		// RD
		else if (1 == iDX && -1 == iDY)
		{
			//--------------------------------------------------------------------------
			// 대각선 방향은 기본적으로 3방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);
			MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, df_DD);

			//--------------------------------------------------------------------------
			// 막힌 부분 체크 후, 막힌 부분이 있다면 해당 방향으로 추가적으로 체크
			//--------------------------------------------------------------------------
			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY - 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);
			
			//bCheckTile = CHECK_TILE(iCurrentX - 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);

			/*if (!CheckTile(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);

			if (!CheckTile(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);*/
		}
		// DD
		else if (0 == iDX && -1 == iDY)
		{
			//--------------------------------------------------------------------------
			// 상/하/좌/우 방향은 기본적으로 1방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, df_DD);

			//bCheckTile = CHECK_TILE(iCurrentX - 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);
			
			//bCheckTile = CHECK_TILE(iCurrentX + 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);

			/*if (!CheckTile(iCurrentX - 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);

			if (!CheckTile(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);*/
		}
		// DL
		else if (-1 == iDX && -1 == iDY)
		{
			//--------------------------------------------------------------------------
			// 대각선 방향은 기본적으로 3방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);
			MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, df_DD);

			//--------------------------------------------------------------------------
			// 막힌 부분 체크 후, 막힌 부분이 있다면 해당 방향으로 추가적으로 체크
			//--------------------------------------------------------------------------
			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY - 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);
			
			//bCheckTile = CHECK_TILE(iCurrentX + 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);

			/*if (!CheckTile(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);

			if (!CheckTile(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);*/
		}
		// LL
		else if (-1 == iDX && 0 == iDY)
		{
			//--------------------------------------------------------------------------
			// 상/하/좌/우 방향은 기본적으로 1방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);

			//--------------------------------------------------------------------------
			// 막힌 부분 체크 후, 막힌 부분이 있다면 해당 방향으로 추가적으로 체크
			//--------------------------------------------------------------------------
			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY - 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);
			
			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY + 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY + 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);

			/*if (!CheckTile(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);

			if (!CheckTile(iCurrentX, iCurrentY + 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);*/
		}
		// LU
		else if (-1 == iDX && 1 == iDY)
		{
			//--------------------------------------------------------------------------
			// 대각선 방향은 기본적으로 3방향으로 진행
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);
			MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);

			//--------------------------------------------------------------------------
			// 막힌 부분 체크 후, 막힌 부분이 있다면 해당 방향으로 추가적으로 체크
			//--------------------------------------------------------------------------
			//bCheckTile = CHECK_TILE(iCurrentX + 1, iCurrentY);
			if (!CHECK_TILE(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);

			//bCheckTile = CHECK_TILE(iCurrentX, iCurrentY - 1);
			if (!CHECK_TILE(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_DL);

			/*if (!CheckTile(iCurrentX + 1, iCurrentY))
				MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);
			
			if (!CheckTile(iCurrentX, iCurrentY - 1))
				MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_DL);*/
		}
	}
}

//-----------------------------------------------------------------------
// Func	  : MakeNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY, int iDir)
// return : BOOL
//
// JumpNode함수 호출 후, 만들어야 할 노드라면 해당 함수에서 노드 검사 후 생성
//-----------------------------------------------------------------------
BOOL CJumpPoint::MakeNode(st_TILENODE *stpCurrent, int iCurrentX, int iCurrentY, int iDir)
{
	m_iDepth = dfDEPTH_MAX;

	int iJumpX = 0;
	int iJumpY = 0;
	float fJumpG = 0;

	//bool bCheckTile = CHECK_TILE(iCurrentX, iCurrentY);
	if (!CHECK_TILE(iCurrentX, iCurrentY))
		return FALSE;

	//--------------------------------------------------------------------------
	// JumpNode 함수는 재귀호출 되며, 
	// JUmpNode 함수가 TRUE면 node를 만들 좌표를 openlist, closelist 검사 후,
	// 없다면 생성한다.
	//--------------------------------------------------------------------------
	if (JumpNode(iCurrentX, iCurrentY, &iJumpX, &iJumpY, stpCurrent->fG + 1, &fJumpG, iDir))
	{
		//--------------------------------------------------------------------------
		// openlist에서 검사
		//--------------------------------------------------------------------------
		auto aOpenNode = find_if(m_mmOpenList.begin(), m_mmOpenList.end(), [iJumpX, iJumpY](const pair<float, st_TILENODE *> &pOpenList) {return pOpenList.second->iX == iJumpX && pOpenList.second->iY == iJumpY; });

		//--------------------------------------------------------------------------
		// map에 없으면 end()를 반환해준다.
		//--------------------------------------------------------------------------
		if (aOpenNode != m_mmOpenList.end())
		{
			if (aOpenNode->second->fG > fJumpG)
			{
				aOpenNode->second->fG = fJumpG;
				aOpenNode->second->stpParent = stpCurrent;
			}

			return TRUE;
		}

		//--------------------------------------------------------------------------
		// closelist에서 검사
		//--------------------------------------------------------------------------
		auto aCloseNode = find_if(m_mmCloseList.begin(), m_mmCloseList.end(), [iJumpX, iJumpY](const pair<float, st_TILENODE *> &pCloseList) {return pCloseList.second->iX == iJumpX && pCloseList.second->iY == iJumpY; });

		if (aCloseNode != m_mmCloseList.end())
		{
			if (aCloseNode->second->fG > fJumpG)
			{
				aCloseNode->second->fG = fJumpG;
				aCloseNode->second->stpParent = stpCurrent;
			}

			return TRUE;
		}

		st_TILENODE *stpNew = new st_TILENODE;

		stpNew->iX = iJumpX;
		stpNew->iY = iJumpY;
		stpNew->fH = abs(iJumpX - m_iEndX) + abs(iJumpY - m_iEndY);
		stpNew->fG = fJumpG;
		stpNew->fF = stpNew->fH + stpNew->fG;
		stpNew->stpParent = stpCurrent;

		m_mmOpenList.insert(pair<float, st_TILENODE *>(stpNew->fF, stpNew));

		//--------------------------------------------------------------------------
		// 시작, 끝 지점을 제외하고 노드가 만들어졌다면, 
		// 해당 좌표는 openlist에 해당 하는 값 지정
		//--------------------------------------------------------------------------
		if ((iJumpX != m_iStartX || iJumpY != m_iStartY) && (iJumpX != m_iEndX || iJumpY != m_iEndY))
			m_byTileAttribute[iJumpY][iJumpX] = df_OPENLIST;

		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------
// Func	  : JumpNode(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir)
// return : BOOL
//
// jump 하려는 노드에 대한 검사를 진행. 
// 재귀 호출을 해야 하는 함수.
// 꺽이는 부분이 있는지 / 계속 jump를 해야 하는지 등..
//-----------------------------------------------------------------------
BOOL CJumpPoint::JumpNode(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir)
{
	float fTempG = 0;

	//bool bCheckTile = CHECK_TILE(iCurrentX, iCurrentY);
	if (!CHECK_TILE(iCurrentX, iCurrentY))
		return FALSE;

	//--------------------------------------------------------------------------
	// 대각선과 상/하/좌/우 의 G 값이 다름
	//--------------------------------------------------------------------------
	if (df_UR == iDir || df_RD == iDir || df_DL == iDir || df_LU == iDir)
		fTempG = (float)(fCurrentG + 1.5);
	else
		fTempG = (float)(fCurrentG + 1);

	//--------------------------------------------------------------------------
	// 시작, 끝 지점을 제외하고 JumpNode 함수에 들어왔다면, 
	// 해당 좌표에 0~9 사이의 임의 상수 값 저장
	//--------------------------------------------------------------------------
	if ((iCurrentX != m_iStartX || iCurrentY != m_iStartY) && (iCurrentX != m_iEndX || iCurrentY != m_iEndY))
		m_byTileAttribute[iCurrentY][iCurrentX] = m_iJumpIdx;

	//// 해당 깊이 이상으로 들어가지 않기.
	//if (0 == --m_iDepth)
	//{
	//	//*ipJumpX = iCurrentX;
	//	//*ipJumpY = iCurrentY;
	//	//*fpJumpG = fCurrentG;

	//	return FALSE;
	//}

	//--------------------------------------------------------------------------
	// 현재 좌표가 끝지점이라면 값 세팅하고 리턴
	//--------------------------------------------------------------------------
	if (iCurrentX == m_iEndX && iCurrentY == m_iEndY)
	{
		*ipJumpX = iCurrentX;
		*ipJumpY = iCurrentY;
		*fpJumpG = fCurrentG;

		return TRUE;
	}
	else
	{
		//--------------------------------------------------------------------------
		// 방향 체크
		//--------------------------------------------------------------------------
		switch (iDir)
		{
		case df_UU:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_UR:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 대각선인 경우, 대각선 이외에 2방향에 대해 추가적으로 진행
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_UU) || JumpNode(iCurrentX + 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_RR))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX + 1, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_RR:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX + 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_RD:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 대각선인 경우, 대각선 이외에 2방향에 대해 추가적으로 진행
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX + 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_RR) || JumpNode(iCurrentX, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_DD))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX + 1, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_DD:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_DL:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 대각선인 경우, 대각선 이외에 2방향에 대해 추가적으로 진행
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_DD) || JumpNode(iCurrentX - 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_LL))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX - 1, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_LL:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX - 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_LU:
			//--------------------------------------------------------------------------
			// 꺽임이 가능한지 체크
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 대각선인 경우, 대각선 이외에 2방향에 대해 추가적으로 진행
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX - 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_LL) || JumpNode(iCurrentX, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_UU))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// 여기까지 왔다는 것은, 꺽이는 부분이 없다는 것이므로 방향대로 1칸 진행
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX - 1, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		}
	}

	return TRUE;
}

//-----------------------------------------------------------------------
// Func	  : CheckJumpNode(int iCurrentX, int iCurrentY, int iDir)
// return : BOOL
//
// jump 하려는 방향에 따라서 꺽이는 부분이 있는지 체크
//-----------------------------------------------------------------------
BOOL CJumpPoint::CheckJumpNode(int iCurrentX, int iCurrentY, int iDir)
{
	//bool bCheckTile1 = false;
	//bool bCheckTile2 = false;

	//--------------------------------------------------------------------------
	// 꺽임에 대해서 검사
	//--------------------------------------------------------------------------
	switch (iDir)
	{
	case df_UU:
		//bCheckTile1 = CHECK_TILE(iCurrentX - 1, iCurrentY - 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX + 1, iCurrentY - 1);
		if ((CHECK_TILE(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX + 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY][iCurrentX + 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;

		/*if ((CheckTile(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| ((CheckTile(iCurrentX + 1, iCurrentY - 1) && m_byTileAttribute[iCurrentY][iCurrentX + 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	case df_UR:
		//bCheckTile1 = CHECK_TILE(iCurrentX - 1, iCurrentY - 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX + 1, iCurrentY + 1);
		if ((CHECK_TILE(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX + 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY + 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;

		/*if ((CheckTile(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| ((CheckTile(iCurrentX + 1, iCurrentY + 1) && m_byTileAttribute[iCurrentY + 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	case df_RR:
		//bCheckTile1 = CHECK_TILE(iCurrentX + 1, iCurrentY - 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX + 1, iCurrentY + 1);
		if ((CHECK_TILE(iCurrentX + 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX + 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY + 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;

		/*if ((CheckTile(iCurrentX + 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK))
			|| ((CheckTile(iCurrentX + 1, iCurrentY + 1) && m_byTileAttribute[iCurrentY + 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	case df_RD:
		//bCheckTile1 = CHECK_TILE(iCurrentX + 1, iCurrentY - 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX - 1, iCurrentY + 1);
		if ((CHECK_TILE(iCurrentX + 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX - 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK)))
			return TRUE;

		/*if ((CheckTile(iCurrentX + 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK))
			|| ((CheckTile(iCurrentX - 1, iCurrentY + 1) && m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	case df_DD:
		//bCheckTile1 = CHECK_TILE(iCurrentX - 1, iCurrentY + 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX + 1, iCurrentY + 1);
		if ((CHECK_TILE(iCurrentX - 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX + 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX + 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;

		/*if ((CheckTile(iCurrentX - 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CheckTile(iCurrentX + 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX + 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	case df_DL:
		//bCheckTile1 = CHECK_TILE(iCurrentX - 1, iCurrentY - 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX + 1, iCurrentY + 1);
		if ((CHECK_TILE(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX + 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX + 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;

		/*if ((CheckTile(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CheckTile(iCurrentX + 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX + 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX + 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	case df_LL:
		//bCheckTile1 = CHECK_TILE(iCurrentX - 1, iCurrentY - 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX - 1, iCurrentY + 1);
		if ((CHECK_TILE(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX - 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY + 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK)))
			return TRUE;

		/*if ((CheckTile(iCurrentX - 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX - 1] == df_NONBLOCK))
			|| (CheckTile(iCurrentX - 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY + 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	case df_LU:
		//bCheckTile1 = CHECK_TILE(iCurrentX + 1, iCurrentY - 1);
		//bCheckTile2 = CHECK_TILE(iCurrentX - 1, iCurrentY + 1);
		if ((CHECK_TILE(iCurrentX + 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK))
			|| (CHECK_TILE(iCurrentX - 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK)))
			return TRUE;
		/*if ((CheckTile(iCurrentX + 1, iCurrentY - 1) && (m_byTileAttribute[iCurrentY - 1][iCurrentX] == df_BLOCK) && (m_byTileAttribute[iCurrentY - 1][iCurrentX + 1] == df_NONBLOCK))
			|| (CheckTile(iCurrentX - 1, iCurrentY + 1) && (m_byTileAttribute[iCurrentY][iCurrentX - 1] == df_BLOCK) && (m_byTileAttribute[iCurrentY + 1][iCurrentX - 1] == df_NONBLOCK)))
			return TRUE;*/
		break;
	}

	return FALSE;
}

//-----------------------------------------------------------------------
// Func	  : CheckTile(int iCurrentX, int iCurrentY)
// return : BOOL
//
// 해당 좌표가 block 좌표인지, 실제로 생성이 가능 좌표인지 확인
//-----------------------------------------------------------------------
BOOL CJumpPoint::CheckTile(int iCurrentX, int iCurrentY)
{
	//--------------------------------------------------------------------------
	// 타일 범위에서 벗어나는 위치가 넘어오면 만들지 않는다.
	//--------------------------------------------------------------------------
	if (df_BLOCK == m_byTileAttribute[iCurrentY][iCurrentX])
		return FALSE;

	if (iCurrentX < 0 || iCurrentX >= df_ARRAYMAPX)
		return FALSE;

	if (iCurrentY < 0 || iCurrentY >= df_ARRAYMAPY)
		return FALSE;

	return TRUE;
}
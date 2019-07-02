#include "stdafx.h"
#include "AStar.h"
#include "RedBlackTree.h"

using namespace std;

//-----------------------------------------------------------------------
// Func	  : CAStar()
// return : none
//
// 생성자
//-----------------------------------------------------------------------
CAStar::CAStar()
{
    LoadMap(L".\\MAP\\Map.txt");
	InitTile();
	InitGDI();
}

//-----------------------------------------------------------------------
// Func	  : ~CAStar()
// return : none
//
// 소멸자
//-----------------------------------------------------------------------
CAStar::~CAStar()
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
void CAStar::InitTile(void)
{
	//-----------------------------------------------------------------------
	// non block 값으로 memset
	//-----------------------------------------------------------------------
    memcpy_s(m_bTileArr, sizeof(m_bTileArr), m_byOriginTileAttribute, sizeof(m_byOriginTileAttribute));

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
	m_mmOpenList.clear();
	m_mmCloseList.clear();
	//m_stpCloseList = new CRedBlackTree;
}

//-----------------------------------------------------------------------
// Func	  : InitGDI(void)
// return : void
//
// GDI 오브젝트 관련 초기화 
//-----------------------------------------------------------------------
void CAStar::InitGDI()
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
	h_pBrush = new HBRUSH[5];
	h_pBrush[eSTART] = CreateSolidBrush(RGB(255, 0, 0));
	h_pBrush[eEND] = CreateSolidBrush(RGB(0, 255, 0));
	h_pBrush[eBLOCK] = CreateSolidBrush(RGB(153, 153, 153));
	h_pBrush[eMOVE] = CreateSolidBrush(RGB(255, 255, 153));
	h_pBrush[eMOVERANGE] = CreateSolidBrush(RGB(0, 0, 255));
}

//-----------------------------------------------------------------------
// Func	  : DrawTile(HWND hWnd, HDC hdc)
// return : void
//
// 타일 그리기
//-----------------------------------------------------------------------
void CAStar::DrawTile(HWND hWnd, HDC hdc)
{
	CRect cRect;

	GetClientRect(hWnd, &cRect);

	if (h_MemDC == NULL)
	{
		h_MemDC = CreateCompatibleDC(hdc);

		if (h_MyBitMap == NULL)
		{
			h_MyBitMap = CreateCompatibleBitmap(hdc, cRect.right, cRect.bottom);
			SelectObject(h_MemDC, h_MyBitMap);
		}
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
	//SetROP2(h_MemDC, R2_COPYPEN);

	//--------------------------------------------------------------------------
	// 바둑판 모양의 맵을 그린다.
	//--------------------------------------------------------------------------
	for (int iX = 0; iX < df_ARRAYMAPX; iX++)
	{
		for (int iY = 0; iY < df_ARRAYMAPY; iY++)
		{
			if (m_bTileArr[iY][iX] == df_NONBLOCK)
			{
				if (m_iTempStartX == iX && m_iTempStartY == iY)
					SelectObject(h_MemDC, h_pBrush[eSTART]);
				else if (m_iTempEndX == iX && m_iTempEndY == iY)
					SelectObject(h_MemDC, h_pBrush[eEND]);
				else
					SelectObject(h_MemDC, h_OldBrush);

				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			else
			{
				SelectObject(h_MemDC, h_pBrush[eBLOCK]);
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
void CAStar::DrawProcessOfMovement(HWND hWnd, HDC hdc)
{
	int iX = 0;
	int iY = 0;
	CRect cRect;

	GetClientRect(hWnd, &cRect);

	if (h_MemDC == NULL)
	{
		h_MemDC = CreateCompatibleDC(hdc);

		if (h_MyBitMap == NULL)
		{
			h_MyBitMap = CreateCompatibleBitmap(hdc, cRect.right, cRect.bottom);
			SelectObject(h_MemDC, h_MyBitMap);
		}
	}

	h_OldBrush = (HBRUSH)SelectObject(h_MemDC, h_pBrush[eMOVE]);

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

	multimap<float, ST_TILENODE *>::iterator iter;

	//--------------------------------------------------------------------------
	// 실제 이동 경로. (최적 경로 아님)
	// close list에는 이동한 경로가 리스트 형태로 연결되어 있음.
	// 따라서 close list를 순회하면서 찍으면 됨.
	//--------------------------------------------------------------------------
	for (iter = m_mmCloseList.begin(); iter != m_mmCloseList.end(); iter++)
	{
		iX = iter->second->iX;
		iY = iter->second->iY;

		if (iX == m_iTempStartX && iY == m_iTempStartY)
			continue;

		if (iX == m_iTempEndX && iY == m_iTempEndY)
			continue;

		if (m_bTileArr[iY][iX] != df_BLOCK)
		{
			SelectObject(h_MemDC, h_pBrush[eMOVE]);
			Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
		}
	}

	//--------------------------------------------------------------------------
	// 이동 할 수 있는 노드들은 open list에 리스트 형태로 연결되어 있음.
	// 단, 노드가 만들어져 이동 가능 한 노드들이지만, F 값이 커서 이동 경로로
	// 적합하지 않은 노드들임.
	//--------------------------------------------------------------------------
	for (iter = m_mmOpenList.begin(); iter != m_mmOpenList.end(); iter++)
	{
		iX = iter->second->iX;
		iY = iter->second->iY;

		if (iX == m_iTempStartX && iY == m_iTempStartY)
			continue;

		if (iX == m_iTempEndX && iY == m_iTempEndY)
			continue;

		if (m_bTileArr[iY][iX] != df_BLOCK)
		{
			SelectObject(h_MemDC, h_pBrush[eMOVERANGE]);
			Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
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
void CAStar::DrawBestRoute(HWND hWnd, HDC hdc, ST_TILENODE *stpDest)
{
	CRect cRect;

	GetClientRect(hWnd, &cRect);

	if (h_MemDC == NULL)
	{
		h_MemDC = CreateCompatibleDC(hdc);

		if (h_MyBitMap == NULL)
		{
			h_MyBitMap = CreateCompatibleBitmap(hdc, cRect.right, cRect.bottom);
			SelectObject(h_MemDC, h_MyBitMap);
		}
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

		// 목적지에서부터 시작점으로 이동하는 구조
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
void CAStar::DrawBitMap(HWND hWnd, HDC hdc)
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
void CAStar::CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
{
	//--------------------------------------------------------------------------
	// 마우스 왼쪽 버튼을 클릭 했는지 오른쪽 버튼을 클릭 했는지에 따라서
	// bButton의 값이 TRUE와 FALSE로 정해진다.
	// TRUE라면, 왼쪽 버튼이기 때문에 전부 block 처리
	// FASLE라면, 오른쪽 버튼이기 때문에 접누 nonblock 처리
	//--------------------------------------------------------------------------
	if (bButton == TRUE)
		m_bTileArr[iYPos][iXPos] = df_BLOCK;
	else
		m_bTileArr[iYPos][iXPos] = df_NONBLOCK;
}

//-----------------------------------------------------------------------
// Func	  : MakeStartNode(int iStarX, int iStartY, int iEndX, int iEndY)
// return : void
//
// 출발 지점에 대한 노드 생성 및 초기화
//-----------------------------------------------------------------------
void CAStar::MakeStartNode(int iStartX, int iStartY, int iEndX, int iEndY)
{
	ST_TILENODE *stpNew = new ST_TILENODE;

	//--------------------------------------------------------------------------
	// 기존에 검색을 한 번 한 후, 시작 지점과 종료 지점을 옮기면
	// 멤버 변수의 시작/종료 좌표가 변한다. 그런대 이 멤버 변수의 시작/종료 좌표는 
	// 길 찾기 시작 전/후 항상 참조를 하는 부분이 있다.
	// 따라서 시작/종료 지점을 찍을 때는 임시 멤버변수에 담은 후에
	// 길찾기를 시작할 때 비로소 실제 시작/종료 멤버 변수에 대입하도록 되어야 한다.
	// 만약, 임시변수를 두지 않을 경우 길찾기 후에 시작/종료 지점을 바꾸게 되면
	// 표시가 잘못 되거나, 좌표를 찾을 수 없기 때문에 프로그램이 다운됨
	//--------------------------------------------------------------------------
	m_iStartX = iStartX;
	m_iStartY = iStartY;
	m_iEndX = iEndX;
	m_iEndY = iEndY;

	stpNew->iX = iStartX;
	stpNew->iY = iStartY;
	stpNew->fH = abs(iStartX - iEndX) + abs(iStartY - iEndY);
	stpNew->fG = 0;
	stpNew->fF = stpNew->fH + stpNew->fG;
	stpNew->stpParent = NULL;

	//--------------------------------------------------------------------------
	// 오픈리스트 초기화 후, 오픈리스트에 삽입
	//--------------------------------------------------------------------------
	m_mmOpenList.clear();
	m_mmOpenList.insert(pair<float, ST_TILENODE *>(stpNew->fF, stpNew));

	/*while (1)
	{
		if (m_mmOpenList.empty() == TRUE)
			return;

		//--------------------------------------------------------------------------
		// f값이 가장 작은 값 뽑기.
		// multimap을 오름차순으로 정렬(default 값)
		// 따라서, 가장 첫 번째의 second 값을 뽑음.
		//--------------------------------------------------------------------------
		stpTempNode = m_mmOpenList.begin()->second;
		m_mmOpenList.erase(m_mmOpenList.begin());
		m_mmCloseList.insert(pair<float, ST_TILENODE *>(stpTempNode->fF, stpTempNode));
		//m_stpCloseList->InsertNode(stpTempNode);

		if (stpTempNode->iX == iEndX && stpTempNode->iY == iEndY)
		{

			return;
		}

		MakeEightDirectionNode(stpTempNode, stpTempNode->iX, stpTempNode->iY);
	}*/
}

//-----------------------------------------------------------------------
// Func	  : FindTargetSpot(HWND hWnd)
// return : BOOL
//
// 한 칸 이동이 가능한 노드 중 가중치 값이 가장 작은 것 찾기
//-----------------------------------------------------------------------
BOOL CAStar::FindTargetSpot(HWND hWnd)
{
	ST_TILENODE *stpTempNode;

	if (m_mmOpenList.empty() == TRUE)
		return FALSE;

	//--------------------------------------------------------------------------
	// f값이 가장 작은 값 뽑기.
	// multimap을 오름차순으로 정렬(default 값)
	// 따라서, 가장 첫 번째의 second 값을 뽑음.
	//--------------------------------------------------------------------------
	stpTempNode = m_mmOpenList.begin()->second;
	m_mmOpenList.erase(m_mmOpenList.begin());
	m_mmCloseList.insert(pair<float, ST_TILENODE *>(stpTempNode->fF, stpTempNode));
	//m_stpCloseList->InsertNode(stpTempNode);

	//--------------------------------------------------------------------------
	// open list에서 뽑은 노드의 좌표가 실제 end x, y 좌표라면 목적지 노드에 현재 뽑은
	// 노드 포인터 저장하고 return true.
	// return true하면, 해당 함수를 반복하는 것이 끝남.
	//--------------------------------------------------------------------------
	if (stpTempNode->iX == m_iEndX && stpTempNode->iY == m_iEndY)
	{
		m_stpDest = stpTempNode;
		return TRUE;
	}

	//--------------------------------------------------------------------------
	// 8방향에 대해서 이동 가능한 tile인지 체크 후, 노드 생성 함수 호출
	//--------------------------------------------------------------------------
	MakeEightDirectionNode(stpTempNode, stpTempNode->iX, stpTempNode->iY);

	return FALSE;
}

bool CAStar::LoadMap(WCHAR * szFilePath)
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
		// '.X' 개행 후, X
		//else if (m_wpBuffer[iBufPos] == 0x580a)
		//{
		//	// 개행 후, 블럭
		//	// 2칸 이동. 파일과 서버의 비율이 2배 차이 이므로.
		//	iY += 2;
		//	iX = 0;
		//
		//	m_byOriginTileAttribute[iY][iX] = df_BLOCK;
		//	m_byOriginTileAttribute[iY][iX + 1] = df_BLOCK;
		//	m_byOriginTileAttribute[iY + 1][iX] = df_BLOCK;
		//	m_byOriginTileAttribute[iY + 1][iX + 1] = df_BLOCK;
		//
		//	// 2칸 이동. 파일과 서버의 비율이 2배 차이 이므로.
		//	iX += 2;
		//}
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

bool CAStar::LoadFile(WCHAR * szFilePath)
{
    m_wpBuffer = new WCHAR[1000000];

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
	if (1000000 < m_iLoadSize)
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

//-----------------------------------------------------------------------
// Func	  : MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY)
// return : void
//
// 8방향의 노드에 대해서 생성 가능한 노드 들인지 확인
// stpCurrent 노드는 기준이 되는 노드이고,
// 앞으로 만들 노드의 parent가 stpCurrent 노드가 됨
//-----------------------------------------------------------------------
void CAStar::MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY)
{
	//--------------------------------------------------------------------------
	// 타일 범위에서 벗어나는 위치가 넘어오면 만들지 않는다.
	//--------------------------------------------------------------------------

	// 위
	if ((df_NONBLOCK == m_bTileArr[iCurrentY - 1][iCurrentX]) && 
		(iCurrentX >= 0 && iCurrentX <= df_ARRAYMAPX - 1) && 
		((iCurrentY - 1) >= 0 && (iCurrentY - 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, FALSE);


	// 오른쪽 위 대각선
	if (df_NONBLOCK == m_bTileArr[iCurrentY - 1][iCurrentX + 1] && 
		((iCurrentX + 1) >= 0 && (iCurrentX + 1) <= df_ARRAYMAPX - 1) 
		&& ((iCurrentY - 1) >= 0 && (iCurrentY - 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, TRUE);

	// 오른쪽
	if (df_NONBLOCK == m_bTileArr[iCurrentY][iCurrentX + 1] && 
		((iCurrentX + 1) >= 0 && (iCurrentX + 1) <= df_ARRAYMAPX - 1)
		&& (iCurrentY >= 0 && iCurrentY <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, FALSE);

	// 오른쪽 아래 대각선
	if (df_NONBLOCK == m_bTileArr[iCurrentY + 1][iCurrentX + 1] &&
		((iCurrentX + 1) >= 0 && (iCurrentX + 1) <= df_ARRAYMAPX - 1) && 
		((iCurrentY + 1) >= 0 && (iCurrentY + 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, TRUE);

	// 아래
	if (df_NONBLOCK == m_bTileArr[iCurrentY + 1][iCurrentX] && 
		(iCurrentX >= 0 && iCurrentX <= df_ARRAYMAPX - 1) && 
		((iCurrentY + 1) >= 0 && (iCurrentY + 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, FALSE);

	// 아래 왼쪽 대각선
	if (df_NONBLOCK == m_bTileArr[iCurrentY + 1][iCurrentX - 1] && 
		((iCurrentX - 1) >= 0 && (iCurrentX - 1) <= df_ARRAYMAPX - 1) && 
		((iCurrentY + 1) >= 0 && (iCurrentY + 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, TRUE);

	// 왼쪽
	if (df_NONBLOCK == m_bTileArr[iCurrentY][iCurrentX - 1] && 
		((iCurrentX - 1) >= 0 && (iCurrentX - 1) <= df_ARRAYMAPX - 1) &&
		(iCurrentY >= 0 && iCurrentY <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, FALSE);

	// 왼쪽 위 대각선
	if (df_NONBLOCK == m_bTileArr[iCurrentY - 1][iCurrentX - 1] && 
		((iCurrentX - 1) >= 0 && (iCurrentX - 1) <= df_ARRAYMAPX - 1) &&
		((iCurrentY - 1) >= 0 && (iCurrentY - 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, TRUE);
}

//-----------------------------------------------------------------------
// Func	  : MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState)
// return : void
//
// 노드 생성
// stpCurrent 노드는 기준이 되는 노드이고,
// 앞으로 만들 노드의 parent가 stpCurrent 노드가 됨
//-----------------------------------------------------------------------
void CAStar::MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState)
{
	float fTempG = 0;

	//--------------------------------------------------------------------------
	// 대각선 가중치와 상,하,좌,우 가중치를 다르게 줌
	// 만약 가중치가 같으면, 불필요한 이동을 함
	// 대각선의 길이가 상,하,좌,우 보다 길기 때문에 대각선의 가중치를 더 높게 주어야 함
	//--------------------------------------------------------------------------
	if (bDiagonalState == TRUE)
		fTempG = (float)(stpCurrent->fG + 1.5);
	else
		fTempG = (float)(stpCurrent->fG + 1);

	// 만들려는 노드의 좌표가 open list에 있는 노드인지 확인 후,
	// 만약 기존에 만들어진 노드라면, 기존의 가중치 값과 현재 계산된 가중치 값을 비교.
	// 기존의 가중치 값이 더 크다면, 현재의 가중치 값으로 기존의 가중치 값을 바꾸고 
	// 부모 노드를 기준 노드인 stpCurrent로 교체
	// 이것은 지금 만들려는 노드까지의 이동이 기존에 진행됬던 루트보다 현재 진행된 루트가 더 최적의 루트라는 의미이다.
	auto aOpenNode = find_if(m_mmOpenList.begin(), m_mmOpenList.end(), [iMakeX, iMakeY](const pair<float, ST_TILENODE *> &pOpenList) {return pOpenList.second->iX == iMakeX && pOpenList.second->iY == iMakeY; });

	if (aOpenNode != m_mmOpenList.end())
	{
		if (aOpenNode->second->fG > fTempG)
		{
			aOpenNode->second->fG = fTempG;
			aOpenNode->second->stpParent = stpCurrent;
		}

		return;
	}

	// 만들려는 노드의 좌표가 close list에 있는지 확인 후,
	// 만약 기존에 만들어진 노드라면, 기존의 가중치 값과 현재 계산된 가중치 값을 비교.
	// 기존의 가중치 값이 더 크다면, 현재의 가중치 값으로 기존의 가중치 값을 바꾸고 
	// 부모 노드를 기준 노드인 stpCurrent로 교체
	// 이것은 지금 만들려는 노드까지의 이동이 기존에 진행됬던 루트보다 현재 진행된 루트가 더 최적의 루트라는 의미이다.
	auto aCloseNode = find_if(m_mmCloseList.begin(), m_mmCloseList.end(), [iMakeX, iMakeY](const pair<float, ST_TILENODE *> &pCloseList) {return pCloseList.second->iX == iMakeX && pCloseList.second->iY == iMakeY; });

	if (aCloseNode != m_mmCloseList.end())
	{
		if (aCloseNode->second->fG > fTempG)
		{
			aCloseNode->second->fG = fTempG;
			aCloseNode->second->stpParent = stpCurrent;
		}

		return;
	}

	ST_TILENODE *stpNew = new ST_TILENODE;

	stpNew->iX = iMakeX;
	stpNew->iY = iMakeY;
	stpNew->fH = abs(iMakeX - m_iEndX) + abs(iMakeX - m_iEndY);
	stpNew->fG = fTempG;
	stpNew->fF = stpNew->fH + stpNew->fG;
	stpNew->stpParent = stpCurrent;

	m_mmOpenList.insert(pair<float, ST_TILENODE *>(stpNew->fF, stpNew));
}
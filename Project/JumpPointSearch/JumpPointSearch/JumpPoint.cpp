#include "stdafx.h"
#include "JumpPoint.h"

using namespace std;

//-----------------------------------------------------------------------
// Func	  : CJumpPoint()
// return : none
//
// ������
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
// �Ҹ���
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
// 2���� Ÿ�� �� �� ��� ���� �ʱ�ȭ
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
// GDI ������Ʈ ���� �ʱ�ȭ 
//-----------------------------------------------------------------------
void CJumpPoint::InitGDI(void)
{
	//--------------------------------------------------------------------------
	// �� �ʱ�ȭ	
	//--------------------------------------------------------------------------
	h_pMyPen = new HPEN[2];
	h_pMyPen[eGRAYPEN] = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
	h_pMyPen[eREDPEN] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));

	//--------------------------------------------------------------------------
	// �귯�� �ʱ�ȭ	
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
	// JUMP ���� BRUSH �ʱ�ȭ
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
// Ÿ�� �׸���
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
	// PatBlt�� ���������� ��Ʈ���� ����ϴ� �Լ��� �ƴϸ� ������ ���	
	// �޸� DC�� ȭ�� ������ � ������ �𸣱� ������					
	// �޸� DC�� ȭ�� ������ ������� �������ش�.						
	//--------------------------------------------------------------------------
	PatBlt(h_MemDC, 0, 0, cRect.right, cRect.bottom, WHITENESS);

	//--------------------------------------------------------------------------
	// ����� ��带 ����						
	// R2_COPYPEN�� PEN���� ������ ���� ������	
	//--------------------------------------------------------------------------
	SetROP2(h_MemDC, R2_COPYPEN);

	//--------------------------------------------------------------------------
	// �ٵ��� ����� ���� �׸���.
	//--------------------------------------------------------------------------
	for (int iX = 0; iX < df_ARRAYMAPX; iX++)
	{
		for (int iY = 0; iY < df_ARRAYMAPY; iY++)
		{
			
			//--------------------------------------------------------------------------
			// block�� tile ó��
			//--------------------------------------------------------------------------
			if (m_byTileAttribute[iY][iX] == df_BLOCK)
			{
				SelectObject(h_MemDC, h_pBrush[eBLOCK]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			//--------------------------------------------------------------------------
			// nonblock�� tile ó��
			// ��, ����, �� ������ nonblock�̹Ƿ� ���� ó�� ����� ��
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
			// tile�� ����������, ������ ���� �ƴ� tile ó��
			//--------------------------------------------------------------------------
			else if ((m_byTileAttribute[iY][iX] == df_OPENLIST))
			{
				SelectObject(h_MemDC, h_pBrush[eOPENLIST]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			//--------------------------------------------------------------------------
			// openlist�� ��ϵ� �� �� ������ tile�� closelist�� ��ϵ� tile ó��
			//--------------------------------------------------------------------------
			else if ((m_byTileAttribute[iY][iX] == df_CLOSELIST))
			{
				SelectObject(h_MemDC, h_pBrush[eCLOSELIST]);
				Rectangle(h_MemDC, iX * df_RECTINTERVAL + 1, iY * df_RECTINTERVAL + 1, (iX + 1) * df_RECTINTERVAL, (iY + 1) * df_RECTINTERVAL);
			}
			//--------------------------------------------------------------------------
			// jump�� tile �׸���
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
// �� ã�� ���� �׸���
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
	// PatBlt�� ���������� ��Ʈ���� ����ϴ� �Լ��� �ƴϸ� ������ ���	
	// �޸� DC�� ȭ�� ������ � ������ �𸣱� ������					
	// �޸� DC�� ȭ�� ������ ������� �������ش�.						
	//--------------------------------------------------------------------------
	//PatBlt(h_MemDC, 0, 0, cRect.right, cRect.bottom, WHITENESS);

	//--------------------------------------------------------------------------
	// ����� ��带 ����						
	// R2_COPYPEN�� PEN���� ������ ���� ������	
	//--------------------------------------------------------------------------
	SetROP2(h_MemDC, R2_COPYPEN);

	//--------------------------------------------------------------------------
	// �ٵ��� ����� ���� �׸���.
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
// ����ȭ�� ��Ʈ�� �׷���
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
	// PatBlt�� ���������� ��Ʈ���� ����ϴ� �Լ��� �ƴϸ� ������ ���	
	// �޸� DC�� ȭ�� ������ � ������ �𸣱� ������					
	// �޸� DC�� ȭ�� ������ ������� �������ش�.						
	//--------------------------------------------------------------------------
	//PatBlt(h_MemDC, 0, 0, cRect.right, cRect.bottom, WHITENESS);

	//--------------------------------------------------------------------------
	// ����� ��带 ����						
	// R2_COPYPEN�� PEN���� ������ ���� ������	
	//--------------------------------------------------------------------------
	SetROP2(h_MemDC, R2_COPYPEN);

	while (1)
	{
		//--------------------------------------------------------------------------
		// �˻� ���߿� �˻� �ʱ�ȭ�� �Ǹ� while �� ���� ���;� ��.
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
	// ReleaseDC�� �ʿ䰡 ����. �� �Ŀ� �ּ��� �濡 ���� ������ �׷���� �ϱ� ������.
	//--------------------------------------------------------------------------
	//ReleaseDC(hWnd, hdc);
}

//-----------------------------------------------------------------------
// Func	  : DrawBitMap(HWND hWnd, HDC hdc)
// return : void
//
// ��Ʈ�� ���
// �޸� DC���� ȭ�� DC�� ��� ���� ����
//-----------------------------------------------------------------------
void CJumpPoint::DrawBitMap(HWND hWnd, HDC hdc)
{
	CRect cRect;

	GetClientRect(hWnd, &cRect);
	SelectObject(h_MemDC, h_MyBitMap);

	// �޸� DC���� ȭ�� DC�� ��� ���� ����
	BitBlt(hdc, 0, 0, cRect.right, cRect.bottom, h_MemDC, 0, 0, SRCCOPY);
}

//-----------------------------------------------------------------------
// Func	  : CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
// return : void
//
// �̵� �Ұ����� Ÿ�� üũ
//-----------------------------------------------------------------------
void CJumpPoint::CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
{
	//--------------------------------------------------------------------------
	// ���콺 ���� ��ư�� Ŭ�� �ߴ��� ������ ��ư�� Ŭ�� �ߴ����� ����
	// bButton�� ���� TRUE�� FALSE�� ��������.
	// TRUE���, ���� ��ư�̱� ������ ���� block ó��
	// FASLE���, ������ ��ư�̱� ������ ���� nonblock ó��
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
// ��� ������ ���� ��� ���� �� �ʱ�ȭ
//----------------------------------------------------------------------
void CJumpPoint::MakeStartNode(int iStartX, int iStartY, int iEndX, int iEndY)
{
	st_TILENODE *stpNew = new st_TILENODE;

	m_iLoopCnt = 0;

	//--------------------------------------------------------------------------
	// ������ �˻��� �� �� �� ��, ���� ������ ���� ������ �ű��
	// ��� ����/���� ��ǥ�� ���Ѵ�. �׷��� �� ��� ����/���� ��ǥ�� 
	// �� ã�� ���� ��/�� �׻� ������ �ϴ� �κ��� �ֱ� ������
	// ����/���� ������ ���� ���� �ӽ� ��������� ���� �Ŀ�
	// ��ã�⸦ ������ �� ��μ� ���� ����/���� ��� ������ �����ϵ��� �Ǿ� ����
	// ����, �ӽú����� ���� ���� ��� ��ã�� �Ŀ� ����/���� ������ �ٲٰ� �Ǹ�
	// ǥ�ð� �߸� �ǰų�, ��ǥ�� ã�� �� ���� ������ ���α׷��� �ٿ��
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
	// ���¸���Ʈ �ʱ�ȭ ��, ���¸���Ʈ�� ����
	//--------------------------------------------------------------------------
	m_mmOpenList.clear();
	m_mmOpenList.insert(pair<float, st_TILENODE *>(stpNew->fF, stpNew));
}

//-----------------------------------------------------------------------
// Func	  : FindTargetSpot(HWND hWnd)
// return : BOOL
//
// �� ĭ �̵��� ������ ��� �� ����ġ ���� ���� ���� �� ã��
//-----------------------------------------------------------------------
BOOL CJumpPoint::FindTargetSpot(void)
{
	st_TILENODE *stpTempNode;

	if (m_mmOpenList.empty() == TRUE)
		return FALSE;

	//--------------------------------------------------------------------------
	// f���� ���� ���� �� �̱�.
	// multimap�� ������������ ����(default ��)
	// ����, ���� ù ��°�� second ���� ����.
	//--------------------------------------------------------------------------
	stpTempNode = m_mmOpenList.begin()->second;
	m_mmOpenList.erase(m_mmOpenList.begin());
	m_mmCloseList.insert(pair<float, st_TILENODE *>(stpTempNode->fF, stpTempNode));

	//--------------------------------------------------------------------------
	// close list�� �߰��� ��尡 ���� ���� �Ǵ� ���� ������ �ƴ϶��, 
	// �ش� ��ǥ�� �迭�� �Ӽ��� closelist�� ����
	//--------------------------------------------------------------------------
	if ((stpTempNode->iX != m_iStartX || stpTempNode->iY != m_iStartY) && (stpTempNode->iX != m_iEndX || stpTempNode->iY != m_iEndY))
		m_byTileAttribute[stpTempNode->iY][stpTempNode->iX] = df_CLOSELIST;

	//--------------------------------------------------------------------------
	// open list���� ���� ����� ��ǥ�� ���� end x, y ��ǥ��� ������ ��忡 ���� ����
	// ��� ������ �����ϰ� return true.
	// return true�ϸ�, �ش� �Լ��� �ݺ��ϴ� ���� ����.
	//--------------------------------------------------------------------------
	if ((stpTempNode->iX == m_iEndX && stpTempNode->iY == m_iEndY) || m_iLoopCnt == m_iMaxLoop)
	{
		m_stpDest = stpTempNode;

		SetBestRout(m_BestRoute);

		return TRUE;
	}

	//--------------------------------------------------------------------------
	// 8���⿡ ���� jump ��� ������ ���� �Լ�
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
		// �˻� ���߿� �˻� �ʱ�ȭ�� �Ǹ� while �� ���� ���;� ��.
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
		// ���� �α� & �ܼ� ���
		//wprintf(L"Open File Fail: DB_INFO.ini\n");
		return false;
	}

	int iBufPos = 0;

	int iX = 0;
	int iY = 0;

	while (1)
	{
		// 'X,' �Ǵ� ' X'
		if (m_wpBuffer[iBufPos] == 0x58bf || m_wpBuffer[iBufPos] == 0x2c58 || m_wpBuffer[iBufPos] == 0x5820)
		{
			// ��
			m_byOriginTileAttribute[iY][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY][iX + 1] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX + 1] = df_BLOCK;

			// 2ĭ �̵�. ���ϰ� ������ ������ 2�� ���� �̹Ƿ�.
			iX += 2;
		
			// x��ǥ �迭 ������ �� ������, ���� ��尡 ������ ���
			if (iX >= df_ARRAYMAPX)
				iX = df_ARRAYMAPX - 1;
		}
		// 'X.' X ��, ����
		else if (m_wpBuffer[iBufPos] == 0x0d58)
		{
			iX = iX;
			iY = iY;
		
			// �� ��, ����
			m_byOriginTileAttribute[iY][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY][iX + 1] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX] = df_BLOCK;
			m_byOriginTileAttribute[iY + 1][iX + 1] = df_BLOCK;
		
			// 2ĭ �̵�. ���ϰ� ������ ������ 2�� ���� �̹Ƿ�.
			iY += 2;
			iX = 0;
		}
		// ' ,' 
		else if (m_wpBuffer[iBufPos] == 0x2c20 || m_wpBuffer[iBufPos] == 0x2020)
		{
			iX = iX;
			iY = iY;
		
			// �� ��
			m_byOriginTileAttribute[iY][iX] = df_NONBLOCK;
			m_byOriginTileAttribute[iY][iX + 1] = df_NONBLOCK;
			m_byOriginTileAttribute[iY + 1][iX] = df_NONBLOCK;
			m_byOriginTileAttribute[iY + 1][iX + 1] = df_NONBLOCK;
		
			// 2ĭ �̵�. ���ϰ� ������ ������ 2�� ���� �̹Ƿ�.
			iX += 2;
		}
        else if (m_wpBuffer[iBufPos] == 0x0a0d)
        {
            iX = 0;
            iY += 2;
        }

		if (++iBufPos >= m_iLoadSize / 2)
			break;

		// ', '�� 0x202c�� �н� 
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

	// ���� ���� ����
	if (INVALID_HANDLE_VALUE == hFile)
		return false;

	// ���� ũ�� �˻�
	m_iLoadSize = GetFileSize(hFile, NULL);

	// ���ۺ��� ũ�ٸ�
	if (en_MAP::en_BUFFER_SIZE < m_iLoadSize)
	{
		CloseHandle(hFile);

		return false;
	}

	// BOM �ڵ� ����
	char cBOMCODE[10] = { NULL };

	// BOM �ڵ� ����
	// 3byte�� ����.
	ReadFile(hFile, cBOMCODE, 3, &dwRead, NULL);

	// 0xef ���� 1byte hex ���� int������ �ν� �Ѵ�.
	// �׸��� �񱳵Ǵ� cMOBCODE�� unsigned char�� �ƴ϶�� 0xffffffff�� ���� ǥ�� �� ���̴�.
	// �ᱹ, 0xef�� int������ �̹Ƿ� 0x000000ef�� ǥ���� ���̰�, char���� cBOMCODE�� 0xffffffef�� ǥ���� ���̱� ������ ���� �ٸ��ٰ� �Ǵ��Ѵ�.
	// ����, unsigned char ������ cBOMCODE�� �����Ͽ� 0x000000ef�� ���� ǥ�� �ǵ��� �ϴ���, 0xef�� char������ ����ȯ ���� 1byte�� ���ϴ��� �ؾ� �Ѵ�.

	if (cBOMCODE[0] != (char)0xef || cBOMCODE[1] != (char)0xbb || cBOMCODE[2] != (char)0xbf)
	{
		CloseHandle(hFile);
		return false;
	}

	m_iLoadSize -= 3;

	// ���� �б�
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
// 8���⿡ ���� �̵��� ������ ������� Ȯ��
//-----------------------------------------------------------------------
void CJumpPoint::CheckNeighbour(st_TILENODE *stpCurrent)
{
	int iDX = 0;
	int iDY = 0;
	int iCurrentX = stpCurrent->iX;
	int iCurrentY = stpCurrent->iY;
	bool bCheckTile = false;
	//--------------------------------------------------------------------------
	// �θ� ���� �� 8���⿡ ���� üũ
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
	// �θ� ���� �� 8���⿡ ���� üũ
	//--------------------------------------------------------------------------
	else
	{
		//--------------------------------------------------------------------------
		// �θ���� �������� ���⸦ �ľ��ϱ� ���� ����
		// �θ���� �������� ���⸦ ���� �ش� ���⿡ ���ؼ� üũ�� ��
		// x��� �޸�, y���� ������ �ö� ���� ���� �������� ������
		// �θ��忡�� �����带 ��
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
			// ��/��/��/�� ������ �⺻������ 1�������� ����
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);

			//--------------------------------------------------------------------------
			// ���� �κ� üũ ��, ���� �κ��� �ִٸ� ���� �� �ִ� ���⿡ ���ؼ� �߰������� ��� ����
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
			// �밢�� ������ �⺻������ 3�������� ����
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, df_UR);
			MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);

			//--------------------------------------------------------------------------
			// ���� �κ� üũ ��, ���� �κ��� �ִٸ� �ش� �������� �߰������� üũ
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
			// ��/��/��/�� ������ �⺻������ 1�������� ����
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);

			//--------------------------------------------------------------------------
			// ���� �κ� üũ ��, ���� �κ��� �ִٸ� �ش� �������� �߰������� üũ
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
			// �밢�� ������ �⺻������ 3�������� ����
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, df_RD);
			MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, df_RR);
			MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, df_DD);

			//--------------------------------------------------------------------------
			// ���� �κ� üũ ��, ���� �κ��� �ִٸ� �ش� �������� �߰������� üũ
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
			// ��/��/��/�� ������ �⺻������ 1�������� ����
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
			// �밢�� ������ �⺻������ 3�������� ����
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, df_DL);
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);
			MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, df_DD);

			//--------------------------------------------------------------------------
			// ���� �κ� üũ ��, ���� �κ��� �ִٸ� �ش� �������� �߰������� üũ
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
			// ��/��/��/�� ������ �⺻������ 1�������� ����
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);

			//--------------------------------------------------------------------------
			// ���� �κ� üũ ��, ���� �κ��� �ִٸ� �ش� �������� �߰������� üũ
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
			// �밢�� ������ �⺻������ 3�������� ����
			//--------------------------------------------------------------------------
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, df_LU);
			MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, df_LL);
			MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, df_UU);

			//--------------------------------------------------------------------------
			// ���� �κ� üũ ��, ���� �κ��� �ִٸ� �ش� �������� �߰������� üũ
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
// JumpNode�Լ� ȣ�� ��, ������ �� ����� �ش� �Լ����� ��� �˻� �� ����
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
	// JumpNode �Լ��� ���ȣ�� �Ǹ�, 
	// JUmpNode �Լ��� TRUE�� node�� ���� ��ǥ�� openlist, closelist �˻� ��,
	// ���ٸ� �����Ѵ�.
	//--------------------------------------------------------------------------
	if (JumpNode(iCurrentX, iCurrentY, &iJumpX, &iJumpY, stpCurrent->fG + 1, &fJumpG, iDir))
	{
		//--------------------------------------------------------------------------
		// openlist���� �˻�
		//--------------------------------------------------------------------------
		auto aOpenNode = find_if(m_mmOpenList.begin(), m_mmOpenList.end(), [iJumpX, iJumpY](const pair<float, st_TILENODE *> &pOpenList) {return pOpenList.second->iX == iJumpX && pOpenList.second->iY == iJumpY; });

		//--------------------------------------------------------------------------
		// map�� ������ end()�� ��ȯ���ش�.
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
		// closelist���� �˻�
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
		// ����, �� ������ �����ϰ� ��尡 ��������ٸ�, 
		// �ش� ��ǥ�� openlist�� �ش� �ϴ� �� ����
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
// jump �Ϸ��� ��忡 ���� �˻縦 ����. 
// ��� ȣ���� �ؾ� �ϴ� �Լ�.
// ���̴� �κ��� �ִ��� / ��� jump�� �ؾ� �ϴ��� ��..
//-----------------------------------------------------------------------
BOOL CJumpPoint::JumpNode(int iCurrentX, int iCurrentY, int *ipJumpX, int *ipJumpY, float fCurrentG, float *fpJumpG, int iDir)
{
	float fTempG = 0;

	//bool bCheckTile = CHECK_TILE(iCurrentX, iCurrentY);
	if (!CHECK_TILE(iCurrentX, iCurrentY))
		return FALSE;

	//--------------------------------------------------------------------------
	// �밢���� ��/��/��/�� �� G ���� �ٸ�
	//--------------------------------------------------------------------------
	if (df_UR == iDir || df_RD == iDir || df_DL == iDir || df_LU == iDir)
		fTempG = (float)(fCurrentG + 1.5);
	else
		fTempG = (float)(fCurrentG + 1);

	//--------------------------------------------------------------------------
	// ����, �� ������ �����ϰ� JumpNode �Լ��� ���Դٸ�, 
	// �ش� ��ǥ�� 0~9 ������ ���� ��� �� ����
	//--------------------------------------------------------------------------
	if ((iCurrentX != m_iStartX || iCurrentY != m_iStartY) && (iCurrentX != m_iEndX || iCurrentY != m_iEndY))
		m_byTileAttribute[iCurrentY][iCurrentX] = m_iJumpIdx;

	//// �ش� ���� �̻����� ���� �ʱ�.
	//if (0 == --m_iDepth)
	//{
	//	//*ipJumpX = iCurrentX;
	//	//*ipJumpY = iCurrentY;
	//	//*fpJumpG = fCurrentG;

	//	return FALSE;
	//}

	//--------------------------------------------------------------------------
	// ���� ��ǥ�� �������̶�� �� �����ϰ� ����
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
		// ���� üũ
		//--------------------------------------------------------------------------
		switch (iDir)
		{
		case df_UU:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_UR:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// �밢���� ���, �밢�� �̿ܿ� 2���⿡ ���� �߰������� ����
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_UU) || JumpNode(iCurrentX + 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_RR))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX + 1, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_RR:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX + 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_RD:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// �밢���� ���, �밢�� �̿ܿ� 2���⿡ ���� �߰������� ����
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX + 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_RR) || JumpNode(iCurrentX, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_DD))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX + 1, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_DD:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_DL:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// �밢���� ���, �밢�� �̿ܿ� 2���⿡ ���� �߰������� ����
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_DD) || JumpNode(iCurrentX - 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_LL))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX - 1, iCurrentY + 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_LL:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
			//--------------------------------------------------------------------------
			return JumpNode(iCurrentX - 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, iDir);
			break;
		case df_LU:
			//--------------------------------------------------------------------------
			// ������ �������� üũ
			//--------------------------------------------------------------------------
			if (CheckJumpNode(iCurrentX, iCurrentY, iDir))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// �밢���� ���, �밢�� �̿ܿ� 2���⿡ ���� �߰������� ����
			//--------------------------------------------------------------------------
			if (JumpNode(iCurrentX - 1, iCurrentY, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_LL) || JumpNode(iCurrentX, iCurrentY - 1, ipJumpX, ipJumpY, fCurrentG + 1, fpJumpG, df_UU))
			{
				*ipJumpX = iCurrentX;
				*ipJumpY = iCurrentY;
				*fpJumpG = fCurrentG;

				return TRUE;
			}

			//--------------------------------------------------------------------------
			// ������� �Դٴ� ����, ���̴� �κ��� ���ٴ� ���̹Ƿ� ������ 1ĭ ����
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
// jump �Ϸ��� ���⿡ ���� ���̴� �κ��� �ִ��� üũ
//-----------------------------------------------------------------------
BOOL CJumpPoint::CheckJumpNode(int iCurrentX, int iCurrentY, int iDir)
{
	//bool bCheckTile1 = false;
	//bool bCheckTile2 = false;

	//--------------------------------------------------------------------------
	// ���ӿ� ���ؼ� �˻�
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
// �ش� ��ǥ�� block ��ǥ����, ������ ������ ���� ��ǥ���� Ȯ��
//-----------------------------------------------------------------------
BOOL CJumpPoint::CheckTile(int iCurrentX, int iCurrentY)
{
	//--------------------------------------------------------------------------
	// Ÿ�� �������� ����� ��ġ�� �Ѿ���� ������ �ʴ´�.
	//--------------------------------------------------------------------------
	if (df_BLOCK == m_byTileAttribute[iCurrentY][iCurrentX])
		return FALSE;

	if (iCurrentX < 0 || iCurrentX >= df_ARRAYMAPX)
		return FALSE;

	if (iCurrentY < 0 || iCurrentY >= df_ARRAYMAPY)
		return FALSE;

	return TRUE;
}
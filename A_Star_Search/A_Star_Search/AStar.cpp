#include "stdafx.h"
#include "AStar.h"
#include "RedBlackTree.h"

using namespace std;

//-----------------------------------------------------------------------
// Func	  : CAStar()
// return : none
//
// ������
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
// �Ҹ���
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
// 2���� Ÿ�� �� �� ��� ���� �ʱ�ȭ
//-----------------------------------------------------------------------
void CAStar::InitTile(void)
{
	//-----------------------------------------------------------------------
	// non block ������ memset
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
// GDI ������Ʈ ���� �ʱ�ȭ 
//-----------------------------------------------------------------------
void CAStar::InitGDI()
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
// Ÿ�� �׸���
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
	// PatBlt�� ���������� ��Ʈ���� ����ϴ� �Լ��� �ƴϸ� ������ ���	
	// �޸� DC�� ȭ�� ������ � ������ �𸣱� ������					
	// �޸� DC�� ȭ�� ������ ������� �������ش�.						
	//--------------------------------------------------------------------------
	PatBlt(h_MemDC, 0, 0, cRect.right, cRect.bottom, WHITENESS);

	//--------------------------------------------------------------------------
	// ����� ��带 ����						
	// R2_COPYPEN�� PEN���� ������ ���� ������	
	//--------------------------------------------------------------------------
	//SetROP2(h_MemDC, R2_COPYPEN);

	//--------------------------------------------------------------------------
	// �ٵ��� ����� ���� �׸���.
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
// �� ã�� ���� �׸���
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

	multimap<float, ST_TILENODE *>::iterator iter;

	//--------------------------------------------------------------------------
	// ���� �̵� ���. (���� ��� �ƴ�)
	// close list���� �̵��� ��ΰ� ����Ʈ ���·� ����Ǿ� ����.
	// ���� close list�� ��ȸ�ϸ鼭 ������ ��.
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
	// �̵� �� �� �ִ� ������ open list�� ����Ʈ ���·� ����Ǿ� ����.
	// ��, ��尡 ������� �̵� ���� �� ����������, F ���� Ŀ�� �̵� ��η�
	// �������� ���� ������.
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
// ����ȭ�� ��Ʈ�� �׷���
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

		// �������������� ���������� �̵��ϴ� ����
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
void CAStar::DrawBitMap(HWND hWnd, HDC hdc)
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
void CAStar::CheckBlockPoint(int iXPos, int iYPos, BOOL bButton)
{
	//--------------------------------------------------------------------------
	// ���콺 ���� ��ư�� Ŭ�� �ߴ��� ������ ��ư�� Ŭ�� �ߴ����� ����
	// bButton�� ���� TRUE�� FALSE�� ��������.
	// TRUE���, ���� ��ư�̱� ������ ���� block ó��
	// FASLE���, ������ ��ư�̱� ������ ���� nonblock ó��
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
// ��� ������ ���� ��� ���� �� �ʱ�ȭ
//-----------------------------------------------------------------------
void CAStar::MakeStartNode(int iStartX, int iStartY, int iEndX, int iEndY)
{
	ST_TILENODE *stpNew = new ST_TILENODE;

	//--------------------------------------------------------------------------
	// ������ �˻��� �� �� �� ��, ���� ������ ���� ������ �ű��
	// ��� ������ ����/���� ��ǥ�� ���Ѵ�. �׷��� �� ��� ������ ����/���� ��ǥ�� 
	// �� ã�� ���� ��/�� �׻� ������ �ϴ� �κ��� �ִ�.
	// ���� ����/���� ������ ���� ���� �ӽ� ��������� ���� �Ŀ�
	// ��ã�⸦ ������ �� ��μ� ���� ����/���� ��� ������ �����ϵ��� �Ǿ�� �Ѵ�.
	// ����, �ӽú����� ���� ���� ��� ��ã�� �Ŀ� ����/���� ������ �ٲٰ� �Ǹ�
	// ǥ�ð� �߸� �ǰų�, ��ǥ�� ã�� �� ���� ������ ���α׷��� �ٿ��
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
	// ���¸���Ʈ �ʱ�ȭ ��, ���¸���Ʈ�� ����
	//--------------------------------------------------------------------------
	m_mmOpenList.clear();
	m_mmOpenList.insert(pair<float, ST_TILENODE *>(stpNew->fF, stpNew));

	/*while (1)
	{
		if (m_mmOpenList.empty() == TRUE)
			return;

		//--------------------------------------------------------------------------
		// f���� ���� ���� �� �̱�.
		// multimap�� ������������ ����(default ��)
		// ����, ���� ù ��°�� second ���� ����.
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
// �� ĭ �̵��� ������ ��� �� ����ġ ���� ���� ���� �� ã��
//-----------------------------------------------------------------------
BOOL CAStar::FindTargetSpot(HWND hWnd)
{
	ST_TILENODE *stpTempNode;

	if (m_mmOpenList.empty() == TRUE)
		return FALSE;

	//--------------------------------------------------------------------------
	// f���� ���� ���� �� �̱�.
	// multimap�� ������������ ����(default ��)
	// ����, ���� ù ��°�� second ���� ����.
	//--------------------------------------------------------------------------
	stpTempNode = m_mmOpenList.begin()->second;
	m_mmOpenList.erase(m_mmOpenList.begin());
	m_mmCloseList.insert(pair<float, ST_TILENODE *>(stpTempNode->fF, stpTempNode));
	//m_stpCloseList->InsertNode(stpTempNode);

	//--------------------------------------------------------------------------
	// open list���� ���� ����� ��ǥ�� ���� end x, y ��ǥ��� ������ ��忡 ���� ����
	// ��� ������ �����ϰ� return true.
	// return true�ϸ�, �ش� �Լ��� �ݺ��ϴ� ���� ����.
	//--------------------------------------------------------------------------
	if (stpTempNode->iX == m_iEndX && stpTempNode->iY == m_iEndY)
	{
		m_stpDest = stpTempNode;
		return TRUE;
	}

	//--------------------------------------------------------------------------
	// 8���⿡ ���ؼ� �̵� ������ tile���� üũ ��, ��� ���� �Լ� ȣ��
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
		// '.X' ���� ��, X
		//else if (m_wpBuffer[iBufPos] == 0x580a)
		//{
		//	// ���� ��, ��
		//	// 2ĭ �̵�. ���ϰ� ������ ������ 2�� ���� �̹Ƿ�.
		//	iY += 2;
		//	iX = 0;
		//
		//	m_byOriginTileAttribute[iY][iX] = df_BLOCK;
		//	m_byOriginTileAttribute[iY][iX + 1] = df_BLOCK;
		//	m_byOriginTileAttribute[iY + 1][iX] = df_BLOCK;
		//	m_byOriginTileAttribute[iY + 1][iX + 1] = df_BLOCK;
		//
		//	// 2ĭ �̵�. ���ϰ� ������ ������ 2�� ���� �̹Ƿ�.
		//	iX += 2;
		//}
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

bool CAStar::LoadFile(WCHAR * szFilePath)
{
    m_wpBuffer = new WCHAR[1000000];

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
	if (1000000 < m_iLoadSize)
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

//-----------------------------------------------------------------------
// Func	  : MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY)
// return : void
//
// 8������ ��忡 ���ؼ� ���� ������ ��� ������ Ȯ��
// stpCurrent ���� ������ �Ǵ� ����̰�,
// ������ ���� ����� parent�� stpCurrent ��尡 ��
//-----------------------------------------------------------------------
void CAStar::MakeEightDirectionNode(ST_TILENODE *stpCurrent, int iCurrentX, int iCurrentY)
{
	//--------------------------------------------------------------------------
	// Ÿ�� �������� ����� ��ġ�� �Ѿ���� ������ �ʴ´�.
	//--------------------------------------------------------------------------

	// ��
	if ((df_NONBLOCK == m_bTileArr[iCurrentY - 1][iCurrentX]) && 
		(iCurrentX >= 0 && iCurrentX <= df_ARRAYMAPX - 1) && 
		((iCurrentY - 1) >= 0 && (iCurrentY - 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX, iCurrentY - 1, FALSE);


	// ������ �� �밢��
	if (df_NONBLOCK == m_bTileArr[iCurrentY - 1][iCurrentX + 1] && 
		((iCurrentX + 1) >= 0 && (iCurrentX + 1) <= df_ARRAYMAPX - 1) 
		&& ((iCurrentY - 1) >= 0 && (iCurrentY - 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX + 1, iCurrentY - 1, TRUE);

	// ������
	if (df_NONBLOCK == m_bTileArr[iCurrentY][iCurrentX + 1] && 
		((iCurrentX + 1) >= 0 && (iCurrentX + 1) <= df_ARRAYMAPX - 1)
		&& (iCurrentY >= 0 && iCurrentY <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX + 1, iCurrentY, FALSE);

	// ������ �Ʒ� �밢��
	if (df_NONBLOCK == m_bTileArr[iCurrentY + 1][iCurrentX + 1] &&
		((iCurrentX + 1) >= 0 && (iCurrentX + 1) <= df_ARRAYMAPX - 1) && 
		((iCurrentY + 1) >= 0 && (iCurrentY + 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX + 1, iCurrentY + 1, TRUE);

	// �Ʒ�
	if (df_NONBLOCK == m_bTileArr[iCurrentY + 1][iCurrentX] && 
		(iCurrentX >= 0 && iCurrentX <= df_ARRAYMAPX - 1) && 
		((iCurrentY + 1) >= 0 && (iCurrentY + 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX, iCurrentY + 1, FALSE);

	// �Ʒ� ���� �밢��
	if (df_NONBLOCK == m_bTileArr[iCurrentY + 1][iCurrentX - 1] && 
		((iCurrentX - 1) >= 0 && (iCurrentX - 1) <= df_ARRAYMAPX - 1) && 
		((iCurrentY + 1) >= 0 && (iCurrentY + 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX - 1, iCurrentY + 1, TRUE);

	// ����
	if (df_NONBLOCK == m_bTileArr[iCurrentY][iCurrentX - 1] && 
		((iCurrentX - 1) >= 0 && (iCurrentX - 1) <= df_ARRAYMAPX - 1) &&
		(iCurrentY >= 0 && iCurrentY <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX - 1, iCurrentY, FALSE);

	// ���� �� �밢��
	if (df_NONBLOCK == m_bTileArr[iCurrentY - 1][iCurrentX - 1] && 
		((iCurrentX - 1) >= 0 && (iCurrentX - 1) <= df_ARRAYMAPX - 1) &&
		((iCurrentY - 1) >= 0 && (iCurrentY - 1) <= df_ARRAYMAPY - 1))
		MakeNode(stpCurrent, iCurrentX - 1, iCurrentY - 1, TRUE);
}

//-----------------------------------------------------------------------
// Func	  : MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState)
// return : void
//
// ��� ����
// stpCurrent ���� ������ �Ǵ� ����̰�,
// ������ ���� ����� parent�� stpCurrent ��尡 ��
//-----------------------------------------------------------------------
void CAStar::MakeNode(ST_TILENODE *stpCurrent, int iMakeX, int iMakeY, BOOL bDiagonalState)
{
	float fTempG = 0;

	//--------------------------------------------------------------------------
	// �밢�� ����ġ�� ��,��,��,�� ����ġ�� �ٸ��� ��
	// ���� ����ġ�� ������, ���ʿ��� �̵��� ��
	// �밢���� ���̰� ��,��,��,�� ���� ��� ������ �밢���� ����ġ�� �� ���� �־�� ��
	//--------------------------------------------------------------------------
	if (bDiagonalState == TRUE)
		fTempG = (float)(stpCurrent->fG + 1.5);
	else
		fTempG = (float)(stpCurrent->fG + 1);

	// ������� ����� ��ǥ�� open list�� �ִ� ������� Ȯ�� ��,
	// ���� ������ ������� �����, ������ ����ġ ���� ���� ���� ����ġ ���� ��.
	// ������ ����ġ ���� �� ũ�ٸ�, ������ ����ġ ������ ������ ����ġ ���� �ٲٰ� 
	// �θ� ��带 ���� ����� stpCurrent�� ��ü
	// �̰��� ���� ������� �������� �̵��� ������ ������� ��Ʈ���� ���� ����� ��Ʈ�� �� ������ ��Ʈ��� �ǹ��̴�.
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

	// ������� ����� ��ǥ�� close list�� �ִ��� Ȯ�� ��,
	// ���� ������ ������� �����, ������ ����ġ ���� ���� ���� ����ġ ���� ��.
	// ������ ����ġ ���� �� ũ�ٸ�, ������ ����ġ ������ ������ ����ġ ���� �ٲٰ� 
	// �θ� ��带 ���� ����� stpCurrent�� ��ü
	// �̰��� ���� ������� �������� �̵��� ������ ������� ��Ʈ���� ���� ����� ��Ʈ�� �� ������ ��Ʈ��� �ǹ��̴�.
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
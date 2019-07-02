// A_Star_Search.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "A_Star_Search.h"
#include "AStar.h"

#define MAX_LOADSTRING 100

// ���� ����:
HINSTANCE hInst;								// ���� �ν��Ͻ��Դϴ�.
TCHAR szTitle[MAX_LOADSTRING];					// ���� ǥ���� �ؽ�Ʈ�Դϴ�.
TCHAR szWindowClass[MAX_LOADSTRING];			// �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

CAStar						*g_pAStar = new CAStar;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���⿡ �ڵ带 �Է��մϴ�.
	MSG msg;
	HACCEL hAccelTable;

	// ���� ���ڿ��� �ʱ�ȭ�մϴ�.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_A_STAR_SEARCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_A_STAR_SEARCH));

	// �⺻ �޽��� �����Դϴ�.
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_A_STAR_SEARCH));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_A_STAR_SEARCH);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   �Լ�: InitInstance(HINSTANCE, int)
//
//   ����: �ν��Ͻ� �ڵ��� �����ϰ� �� â�� ����ϴ�.
//
//   ����:
//
//        �� �Լ��� ���� �ν��Ͻ� �ڵ��� ���� ������ �����ϰ�
//        �� ���α׷� â�� ���� ���� ǥ���մϴ�.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1280, 720, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	//--------------------------------------------------------------------------
	// ������ �ִ�ȭ�� ���� ���,
	// ShowWindow �Լ��� 2��° flag�� SW_MAIMIZE
	//--------------------------------------------------------------------------
	ShowWindow(hWnd, SW_MAXIMIZE);
	UpdateWindow(hWnd);

	//RECT WindowRect;
	//WindowRect.top = 0;
	//WindowRect.left = 0;
	//WindowRect.right = 1260;
	//WindowRect.bottom = 630;

	//AdjustWindowRectEx(&WindowRect, GetWindowStyle(hWnd), GetMenu(hWnd) != NULL, GetWindowExStyle(hWnd));

	////------------------------------------------------------------------
	//// ���� ����� ��ũ���� ����, ���̸� �� �� �߾��� ��ǥ�� ����	
	////------------------------------------------------------------------
	//int iX = (GetSystemMetrics(SM_CXSCREEN) - 1260) / 2;
	//int iY = (GetSystemMetrics(SM_CYSCREEN) - 630) / 2;

	////------------------------------------------------------------------
	//// ������ ���� �� �߾��� ��ǥ�� window�� �̵� ��Ŵ	
	////------------------------------------------------------------------
	//MoveWindow(hWnd, iX, iY, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);



	return TRUE;
}

//
//  �Լ�: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ����:  �� â�� �޽����� ó���մϴ�.
//
//  WM_COMMAND	- ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT	- �� â�� �׸��ϴ�.
//  WM_DESTROY	- ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	static int iBlockX, iBlockY;
	static int iXTemp, iYTemp;
	static BOOL bNowDraw = FALSE;
	static BOOL bButton = TRUE;
	static BOOL bTimer = FALSE;
	static BOOL bFindDest = FALSE;

	switch (message)
	{
	case WM_TIMER:
		if (TRUE == bTimer)
		{
			//--------------------------------------------------------------------------
			// �������� �۾� ������ ��ȿȭ����					
			// WM_PAINT �޽����� �ش� �����쿡 ����	
			// 3��° ���ڰ�, TRUE�� �̸�						
			// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
			// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
			//--------------------------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			if (g_pAStar->FindTargetSpot(hWnd))
			{
				bTimer = FALSE;
				bFindDest = TRUE;
			}
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			//--------------------------------------------------------------------------
			// �������� �۾� ������ ��ȿȭ����					
			// WM_PAINT �޽����� �ش� �����쿡 ����	
			// 3��° ���ڰ�, TRUE�� �̸�						
			// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
			// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
			//--------------------------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);
			g_pAStar->InitTile();
			bFindDest = FALSE;
			break;
		case VK_RETURN:
			//--------------------------------------------------------------------------
			// ���� ������ �� ������ ��� ���� �Ǿ��� ���� �� ã�� ���� ����
			//--------------------------------------------------------------------------
			g_pAStar->m_mmOpenList.clear();
			g_pAStar->m_mmCloseList.clear();
			bFindDest = FALSE;

			if (g_pAStar->m_bCheckStart == TRUE && g_pAStar->m_bCheckEnd == TRUE)
			{
				g_pAStar->MakeStartNode(g_pAStar->m_iTempStartX, g_pAStar->m_iTempStartY, g_pAStar->m_iTempEndX, g_pAStar->m_iTempEndY);
				bTimer = TRUE;
				SetTimer(hWnd, 1, USER_TIMER_MINIMUM, NULL);
			}
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		//--------------------------------------------------------------------------
		// �������� �۾� ������ ��ȿȭ����					
		// WM_PAINT �޽����� �ش� �����쿡 ����	
		// 3��° ���ڰ�, TRUE�� �̸�						
		// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
		// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// ���� ���콺 ��ư�� Ŭ�� �� TRUE
		// bButton�� TRUE �� ������ block ���·� �����.
		//--------------------------------------------------------------------------
		bButton = TRUE;

		if ((iBlockX != g_pAStar->m_iStartX || iBlockY != g_pAStar->m_iStartY) && (iBlockX != g_pAStar->m_iEndX || iBlockY != g_pAStar->m_iEndY))
			g_pAStar->CheckBlockPoint(iBlockX, iBlockY, bButton);

		bNowDraw = TRUE;
		break;
	case WM_RBUTTONDOWN:
		//--------------------------------------------------------------------------
		// �������� �۾� ������ ��ȿȭ����					
		// WM_PAINT �޽����� �ش� �����쿡 ����	
		// 3��° ���ڰ�, TRUE�� �̸�						
		// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
		// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// ���� ���콺 ��ư�� Ŭ�� �� FALSE
		// bButton�� FALSE �� ������ nonblock ���·� �����.
		//--------------------------------------------------------------------------
		bButton = FALSE;

		if ((iBlockX != g_pAStar->m_iStartX || iBlockY != g_pAStar->m_iStartY) && (iBlockX != g_pAStar->m_iEndX || iBlockY != g_pAStar->m_iEndY))
			g_pAStar->CheckBlockPoint(iBlockX, iBlockY, bButton);

		bNowDraw = TRUE;
		break;
	case WM_MOUSEMOVE:
		if (bNowDraw == TRUE)
		{
			// ������ ����
			//--------------------------------------------------------------------------
			// �������� �۾� ������ ��ȿȭ����					
			// WM_PAINT �޽����� �ش� �����쿡 ����	
			// 3��° ���ڰ�, TRUE�� �̸�						
			// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
			// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
			//--------------------------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
			iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

			//--------------------------------------------------------------------------
			// ���콺 ���� ��ư�� Ŭ�� �ߴ��� ������ ��ư�� Ŭ�� �ߴ����� ����
			// bButton�� ���� TRUE�� FALSE�� ��������.
			// ���� mousemove������ � ��ư�� ���������� bButton�� ���� �� �� �ִ�.
			//--------------------------------------------------------------------------
			if ((iBlockX != g_pAStar->m_iStartX || iBlockY != g_pAStar->m_iStartY) && (iBlockX != g_pAStar->m_iEndX || iBlockY != g_pAStar->m_iEndY))
				g_pAStar->CheckBlockPoint(iBlockX, iBlockY, bButton);

			break;
		}
		break;
	case WM_LBUTTONUP:
		bNowDraw = FALSE;
		break;
	case WM_RBUTTONUP:
		bNowDraw = FALSE;
		break;
	case WM_LBUTTONDBLCLK:
		// �������� ����
		//--------------------------------------------------------------------------
		// �������� �۾� ������ ��ȿȭ����					
		// WM_PAINT �޽����� �ش� �����쿡 ����	
		// 3��° ���ڰ�, TRUE�� �̸�						
		// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
		// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iXTemp = LOWORD(lParam) / df_RECTINTERVAL;
		iYTemp = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// ���� ������ �� ������ ���� �� ����.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pAStar->m_iTempEndX && iYTemp == g_pAStar->m_iTempEndY)
			break;

		g_pAStar->m_bCheckStart = TRUE;
		g_pAStar->m_iTempStartX = iXTemp;
		g_pAStar->m_iTempStartY = iYTemp;

		//--------------------------------------------------------------------------
		// ������ Ÿ���� block�̾ �켱���� �������� ���ÿ� ����.
		// ���� �ش� Ÿ���� ������ unblock���� ó��.
		//--------------------------------------------------------------------------
		g_pAStar->m_bTileArr[iYTemp][iXTemp] = 0;

		break;
	case WM_RBUTTONDBLCLK:
		//--------------------------------------------------------------------------
		// �������� �۾� ������ ��ȿȭ����					
		// WM_PAINT �޽����� �ش� �����쿡 ����	
		// 3��° ���ڰ�, TRUE�� �̸�						
		// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
		// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iXTemp = LOWORD(lParam) / df_RECTINTERVAL;
		iYTemp = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// ���� ������ �� ������ ���� �� ����.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pAStar->m_iTempStartX && iYTemp == g_pAStar->m_iTempStartY)
			break;

		g_pAStar->m_bCheckEnd = TRUE;
		g_pAStar->m_iTempEndX = iXTemp;
		g_pAStar->m_iTempEndY = iYTemp;

		//--------------------------------------------------------------------------
		// ������ Ÿ���� block�̾ �켱���� ������ ���ÿ� ����.
		// ���� �ش� Ÿ���� ������ unblock���� ó��.
		//--------------------------------------------------------------------------
		g_pAStar->m_bTileArr[iYTemp][iXTemp] = 0;

		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �޴� ������ ���� �м��մϴ�.
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
		
		g_pAStar->DrawTile(hWnd, hdc);

		g_pAStar->DrawProcessOfMovement(hWnd, hdc);

		if (TRUE == bFindDest)
			g_pAStar->DrawBestRoute(hWnd, hdc, g_pAStar->m_stpDest);

		g_pAStar->DrawBitMap(hWnd, hdc);
		ReleaseDC(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		KillTimer(hWnd, 1);
		DeleteDC(g_pAStar->h_MemDC);
		DeleteObject(g_pAStar->h_MyBitMap);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ���� ��ȭ ������ �޽��� ó�����Դϴ�.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


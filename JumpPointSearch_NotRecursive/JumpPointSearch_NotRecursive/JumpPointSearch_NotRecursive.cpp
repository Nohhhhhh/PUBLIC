// JumpPointSearch_NotRecursive.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "JumpPointSearch_NotRecursive.h"
#include "JumpPoint_NotRecursive.h"

#define MAX_LOADSTRING 100

// ���� ����:
HINSTANCE hInst;                                // ���� �ν��Ͻ��Դϴ�.
WCHAR szTitle[MAX_LOADSTRING];                  // ���� ǥ���� �ؽ�Ʈ�Դϴ�.
WCHAR szWindowClass[MAX_LOADSTRING];            // �⺻ â Ŭ���� �̸��Դϴ�.

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

CJumpPoint						*g_pJump = new CJumpPoint(50);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ���⿡ �ڵ带 �Է��մϴ�.

    // ���� ���ڿ��� �ʱ�ȭ�մϴ�.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_JUMPPOINTSEARCH_NOTRECURSIVE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // ���� ���α׷� �ʱ�ȭ�� �����մϴ�.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JUMPPOINTSEARCH_NOTRECURSIVE));

    MSG msg;

    // �⺻ �޽��� �����Դϴ�.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  �Լ�: MyRegisterClass()
//
//  ����: â Ŭ������ ����մϴ�.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JUMPPOINTSEARCH_NOTRECURSIVE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_JUMPPOINTSEARCH_NOTRECURSIVE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
//  WM_COMMAND  - ���� ���α׷� �޴��� ó���մϴ�.
//  WM_PAINT    - �� â�� �׸��ϴ�.
//  WM_DESTROY  - ���� �޽����� �Խ��ϰ� ��ȯ�մϴ�.
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
		//-------------------------------------------------------
		// ���� �ð� ���� ���� �ݺ�
		//-------------------------------------------------------

		if (TRUE == bTimer)
		{
			//-------------------------------------------------------
			// �������� �۾� ������ ��ȿȭ����					
			// WM_PAINT �޽����� �ش� �����쿡 ����	
			// 3��° ���ڰ�, TRUE�� �̸�						
			// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
			// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
			//-------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			// ���� ��ȭ�� �ֱ� ���ؼ�
			g_pJump->m_iJumpIdx = rand() % 10;

			//-------------------------------------------------------
			// TRUE�� end node�� ã�� ����.
			// ���� timer flag�� false�� �ϰ�, find flag�� true�� ��
			//-------------------------------------------------------
			if (g_pJump->FindTargetSpot())
			{
				bTimer = FALSE;
				bFindDest = TRUE;

				WCHAR szBestRoute[1000];
				memset(szBestRoute, 0, sizeof(szBestRoute));

				int iBestRoutNum = 0;
				int iWritePos = 0;

				iWritePos += swprintf_s(szBestRoute + iWritePos, sizeof(szBestRoute) - iWritePos, L"(%2d.%2d)", g_pJump->m_iStartX, g_pJump->m_iStartY);

				while (1)
				{
					iWritePos += swprintf_s(szBestRoute + iWritePos, sizeof(szBestRoute) - iWritePos, L"(%2d.%2d)", g_pJump->m_BestRoute[iBestRoutNum].iX, g_pJump->m_BestRoute[iBestRoutNum].iY);

					++iBestRoutNum;

					if (iBestRoutNum == g_pJump->m_iBestRoutNodeNum)
						break;

					iWritePos += swprintf_s(szBestRoute + iWritePos, sizeof(szBestRoute) - iWritePos, L" -> ");
				}

				SetWindowTextW(hWnd, szBestRoute);
			}
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			//-------------------------------------------------------
			// Ÿ�� �ʱ�ȭ ���� �����
			//-------------------------------------------------------

			//-------------------------------------------------------
			// �������� �۾� ������ ��ȿȭ����					
			// WM_PAINT �޽����� �ش� �����쿡 ����	
			// 3��° ���ڰ�, TRUE�� �̸�						
			// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
			// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
			//-------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			//-------------------------------------------------------
			// tile�� �ʱ�ȭ ���·� ����
			//-------------------------------------------------------
			g_pJump->InitTile();
			bFindDest = FALSE;
			break;
		case VK_RETURN:
			//-------------------------------------------------------
			// �� ã�� ����
			//-------------------------------------------------------

			//-------------------------------------------------------
			// ���� ������ �� ������ ��� ���� �Ǿ��� ���� �� ã�� ���� ����
			//-------------------------------------------------------
			//if (g_pJump->m_bCheckStart == TRUE && g_pJump->m_bCheckEnd == TRUE)
			//{
			//	//g_pJump->m_mmOpenList.clear();
			//	//g_pJump->m_mmCloseList.clear();
			//	g_pJump->InitTile_Loop();
			//	bFindDest = FALSE;

			//	//-------------------------------------------------------
			//	// Ÿ�� ���� ��� ��尡 �ƴ� ��� ��� ���� ���� �ʱ�ȭ
			//	//-------------------------------------------------------
			//	for (int iCnt1 = 0; iCnt1 < df_ARRAYMAPY; iCnt1++)
			//		for (int iCnt2 = 0; iCnt2 < df_ARRAYMAPX; iCnt2++)
			//			if (g_pJump->m_byTileAttribute[iCnt1][iCnt2] != df_BLOCK)
			//				g_pJump->m_byTileAttribute[iCnt1][iCnt2] = df_NONBLOCK;

			//	//-------------------------------------------------------
			//	// ���� ��� ���� (���� ��ǥ, ���� ��ǥ �� �ʱ�ȭ)
			//	//-------------------------------------------------------
			//	g_pJump->MakeStartNode(g_pJump->m_iTempStartX, g_pJump->m_iTempStartY, g_pJump->m_iTempEndX, g_pJump->m_iTempEndY);

			//	bTimer = TRUE;
			//	SetTimer(hWnd, 1, 10, NULL);
			//}

			WCHAR szPerformance[1000];
			memset(szPerformance, 0, sizeof(szPerformance));
			int iLoopCnt = 0;
			LARGE_INTEGER		g_lFrequency;
			double				g_dMicroFrequency = 0;
			LARGE_INTEGER		lStartTime;
			LARGE_INTEGER		lEndTime;
			__int64				i64TotalTime = 0;
			__int64				i64Time = 0;
			__int64				i64Call = 0;
			QueryPerformanceFrequency(&g_lFrequency);
			g_dMicroFrequency = (double)(g_lFrequency.QuadPart / 1000000.0f);

			while (10000 != iLoopCnt++)
			{
				g_pJump->InitTile_Loop();
				bFindDest = FALSE;

				if (g_pJump->m_bCheckStart == TRUE && g_pJump->m_bCheckEnd == TRUE)
				{
					//-------------------------------------------------------
					// ���� ��� ���� (���� ��ǥ, ���� ��ǥ �� �ʱ�ȭ)
					//-------------------------------------------------------
					g_pJump->MakeStartNode(g_pJump->m_iTempStartX, g_pJump->m_iTempStartY, g_pJump->m_iTempEndX, g_pJump->m_iTempEndY);
					bTimer = TRUE;

					QueryPerformanceCounter(&lStartTime);

					while (bTimer)
					{
						//-------------------------------------------------------
						// �������� �۾� ������ ��ȿȭ����					
						// WM_PAINT �޽����� �ش� �����쿡 ����	
						// 3��° ���ڰ�, TRUE�� �̸�						
						// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
						// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
						//-------------------------------------------------------
						InvalidateRect(hWnd, NULL, FALSE);

						// ���� ��ȭ�� �ֱ� ���ؼ�
						g_pJump->m_iJumpIdx = rand() % 10;

						//-------------------------------------------------------
						// TRUE�� end node�� ã�� ����.
						// ���� timer flag�� false�� �ϰ�, find flag�� true�� ��
						//-------------------------------------------------------
						if (g_pJump->FindTargetSpot())
						{
							bTimer = FALSE;
							bFindDest = TRUE;
							break;
						}
					}
					QueryPerformanceCounter(&lEndTime);
					i64Time = lEndTime.QuadPart - lStartTime.QuadPart;
					i64TotalTime += i64Time;
					i64Call++;
					lStartTime.QuadPart = 0;
				}
			}

			swprintf_s(szPerformance, sizeof(szPerformance), L"Avr: %0.4lf�� | Loop: %lld", (float)i64TotalTime / g_dMicroFrequency / i64Call, i64Call);
			SetWindowTextW(hWnd, szPerformance);

			break;
		}
		break;
	case WM_LBUTTONDOWN:
		//-------------------------------------------------------
		// �� ��� ����
		//-------------------------------------------------------

		//-------------------------------------------------------
		// �������� �۾� ������ ��ȿȭ����					
		// WM_PAINT �޽����� �ش� �����쿡 ����	
		// 3��° ���ڰ�, TRUE�� �̸�						
		// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
		// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
		//-------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//-------------------------------------------------------
		// ���� ���콺 ��ư�� Ŭ�� �� TRUE
		// bButton�� TRUE �� ������ block ���·� �����.
		//-------------------------------------------------------
		bButton = TRUE;

		if ((iBlockX != g_pJump->m_iStartX || iBlockY != g_pJump->m_iStartY) && (iBlockX != g_pJump->m_iEndX || iBlockY != g_pJump->m_iEndY))
			g_pJump->CheckBlockPoint(iBlockX, iBlockY, bButton);

		//-------------------------------------------------------
		// ���콺 Ŭ�� �� �̵��� ���� Ÿ���� ���� �ǵ��� �ϱ� ����
		//-------------------------------------------------------
		bNowDraw = TRUE;

		break;
	case WM_RBUTTONDOWN:
		//-------------------------------------------------------
		// ��� ��� ����
		//-------------------------------------------------------

		//-------------------------------------------------------
		// �������� �۾� ������ ��ȿȭ����					
		// WM_PAINT �޽����� �ش� �����쿡 ����	
		// 3��° ���ڰ�, TRUE�� �̸�						
		// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
		// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
		//-------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//-------------------------------------------------------
		// ���� ���콺 ��ư�� Ŭ�� �� FALSE
		// bButton�� FALSE �� ������ nonblock ���·� �����.
		//-------------------------------------------------------
		bButton = FALSE;

		if ((iBlockX != g_pJump->m_iStartX || iBlockY != g_pJump->m_iStartY) && (iBlockX != g_pJump->m_iEndX || iBlockY != g_pJump->m_iEndY))
			g_pJump->CheckBlockPoint(iBlockX, iBlockY, bButton);

		//-------------------------------------------------------
		// ���콺 Ŭ�� �� �̵��� ���� Ÿ���� ���� �ǵ��� �ϱ� ����
		//-------------------------------------------------------
		bNowDraw = TRUE;
		break;
	case WM_MOUSEMOVE:
		//-------------------------------------------------------
		// Ŭ�� �� �̵�
		//-------------------------------------------------------

		//-------------------------------------------------------
		// nowdraw flag�� true�� �׸��� Ÿ�� ���� ����
		//-------------------------------------------------------
		if (bNowDraw == TRUE)
		{
			// ������ ����
			//-------------------------------------------------------
			// �������� �۾� ������ ��ȿȭ����					
			// WM_PAINT �޽����� �ش� �����쿡 ����	
			// 3��° ���ڰ�, TRUE�� �̸�						
			// ��ȿȭ �Ǳ� ���� ����� ��� ���� ��, �ٽ� �׸�	
			// FALSE�� ����� ������ ���� ä�� �ٽ� �׸�		
			//-------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
			iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

			//-------------------------------------------------------
			// ���콺 ���� ��ư�� Ŭ�� �ߴ��� ������ ��ư�� Ŭ�� �ߴ����� ����
			// bButton�� ���� TRUE�� FALSE�� ��������.
			// ���� mousemove������ � ��ư�� ���������� bButton�� ���� �� �� �ִ�.
			//-------------------------------------------------------
			if ((iBlockX != g_pJump->m_iStartX || iBlockY != g_pJump->m_iStartY) && (iBlockX != g_pJump->m_iEndX || iBlockY != g_pJump->m_iEndY))
				g_pJump->CheckBlockPoint(iBlockX, iBlockY, bButton);

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
		//-------------------------------------------------------
		// �������� ����
		//-------------------------------------------------------

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
		//iXTemp = 69;
		//iYTemp = 147;
		//--------------------------------------------------------------------------
		// ���� ������ �� ������ ���� �� ����.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pJump->m_iTempEndX && iYTemp == g_pJump->m_iTempEndY)
			break;

		g_pJump->m_bCheckStart = TRUE;
		g_pJump->m_iTempStartX = iXTemp;
		g_pJump->m_iTempStartY = iYTemp;

		//g_pJump->m_iTempStartX = 289;
		//g_pJump->m_iTempStartY = 156;

		//--------------------------------------------------------------------------
		// ������ Ÿ���� block�̾ �켱���� �������� ���ÿ� ����.
		// ���� �ش� Ÿ���� ������ unblock���� ó��.
		//--------------------------------------------------------------------------
		g_pJump->m_byTileAttribute[iYTemp][iXTemp] = df_NONBLOCK;

		break;
	case WM_RBUTTONDBLCLK:
		//-------------------------------------------------------
		// �������� ����
		//-------------------------------------------------------

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
		//iXTemp = 88;
		//iYTemp = 122;
		//--------------------------------------------------------------------------
		// ���� ������ �� ������ ���� �� ����.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pJump->m_iTempStartX && iYTemp == g_pJump->m_iTempStartY)
			break;

		g_pJump->m_bCheckEnd = TRUE;
		g_pJump->m_iTempEndX = iXTemp;
		g_pJump->m_iTempEndY = iYTemp;

		//g_pJump->m_iTempEndX = 288;
		//g_pJump->m_iTempEndY = 109;

		//--------------------------------------------------------------------------
		// ������ Ÿ���� block�̾ �켱���� ������ ���ÿ� ����.
		// ���� �ش� Ÿ���� ������ unblock���� ó��.
		//--------------------------------------------------------------------------
		g_pJump->m_byTileAttribute[iYTemp][iXTemp] = df_NONBLOCK;

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

		g_pJump->DrawTile(hWnd, hdc);

		//g_pJump->DrawProcessOfMovement(hWnd, hdc);

		if (TRUE == bFindDest)
		{
			g_pJump->DrawBestRoute(hWnd, hdc, g_pJump->m_stpDest);
			bFindDest = FALSE;
		}

		g_pJump->DrawBitMap(hWnd, hdc);
		ReleaseDC(hWnd, hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		KillTimer(hWnd, 1);
		DeleteDC(g_pJump->h_MemDC);
		DeleteObject(g_pJump->h_MyBitMap);
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

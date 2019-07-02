// A_Star_Search.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "A_Star_Search.h"
#include "AStar.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;								// 현재 인스턴스입니다.
TCHAR szTitle[MAX_LOADSTRING];					// 제목 표시줄 텍스트입니다.
TCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
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

	// TODO: 여기에 코드를 입력합니다.
	MSG msg;
	HACCEL hAccelTable;

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_A_STAR_SEARCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_A_STAR_SEARCH));

	// 기본 메시지 루프입니다.
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
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
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
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1280, 720, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	//--------------------------------------------------------------------------
	// 윈도우 최대화를 원할 경우,
	// ShowWindow 함수에 2번째 flag를 SW_MAIMIZE
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
	//// 실제 모니터 스크린의 넓이, 높이를 얻어서 정 중앙의 좌표를 구함	
	////------------------------------------------------------------------
	//int iX = (GetSystemMetrics(SM_CXSCREEN) - 1260) / 2;
	//int iY = (GetSystemMetrics(SM_CYSCREEN) - 630) / 2;

	////------------------------------------------------------------------
	//// 위에서 얻은 정 중앙의 좌표로 window를 이동 시킴	
	////------------------------------------------------------------------
	//MoveWindow(hWnd, iX, iY, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);



	return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND	- 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT	- 주 창을 그립니다.
//  WM_DESTROY	- 종료 메시지를 게시하고 반환합니다.
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
			// 윈도우의 작업 영역을 무효화시켜					
			// WM_PAINT 메시지를 해당 윈도우에 보냄	
			// 3번째 인자가, TRUE값 이면						
			// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
			// FALSE면 배경을 지우지 않은 채로 다시 그림		
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
			// 윈도우의 작업 영역을 무효화시켜					
			// WM_PAINT 메시지를 해당 윈도우에 보냄	
			// 3번째 인자가, TRUE값 이면						
			// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
			// FALSE면 배경을 지우지 않은 채로 다시 그림		
			//--------------------------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);
			g_pAStar->InitTile();
			bFindDest = FALSE;
			break;
		case VK_RETURN:
			//--------------------------------------------------------------------------
			// 시작 지점과 끝 지점이 모두 선택 되었을 때만 길 찾기 로직 수행
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
		// 윈도우의 작업 영역을 무효화시켜					
		// WM_PAINT 메시지를 해당 윈도우에 보냄	
		// 3번째 인자가, TRUE값 이면						
		// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
		// FALSE면 배경을 지우지 않은 채로 다시 그림		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// 왼쪽 마우스 버튼을 클릭 시 TRUE
		// bButton이 TRUE 면 무조건 block 상태로 만든다.
		//--------------------------------------------------------------------------
		bButton = TRUE;

		if ((iBlockX != g_pAStar->m_iStartX || iBlockY != g_pAStar->m_iStartY) && (iBlockX != g_pAStar->m_iEndX || iBlockY != g_pAStar->m_iEndY))
			g_pAStar->CheckBlockPoint(iBlockX, iBlockY, bButton);

		bNowDraw = TRUE;
		break;
	case WM_RBUTTONDOWN:
		//--------------------------------------------------------------------------
		// 윈도우의 작업 영역을 무효화시켜					
		// WM_PAINT 메시지를 해당 윈도우에 보냄	
		// 3번째 인자가, TRUE값 이면						
		// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
		// FALSE면 배경을 지우지 않은 채로 다시 그림		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// 왼쪽 마우스 버튼을 클릭 시 FALSE
		// bButton이 FALSE 면 무조건 nonblock 상태로 만든다.
		//--------------------------------------------------------------------------
		bButton = FALSE;

		if ((iBlockX != g_pAStar->m_iStartX || iBlockY != g_pAStar->m_iStartY) && (iBlockX != g_pAStar->m_iEndX || iBlockY != g_pAStar->m_iEndY))
			g_pAStar->CheckBlockPoint(iBlockX, iBlockY, bButton);

		bNowDraw = TRUE;
		break;
	case WM_MOUSEMOVE:
		if (bNowDraw == TRUE)
		{
			// 블럭지점 선택
			//--------------------------------------------------------------------------
			// 윈도우의 작업 영역을 무효화시켜					
			// WM_PAINT 메시지를 해당 윈도우에 보냄	
			// 3번째 인자가, TRUE값 이면						
			// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
			// FALSE면 배경을 지우지 않은 채로 다시 그림		
			//--------------------------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
			iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

			//--------------------------------------------------------------------------
			// 마우스 왼쪽 버튼을 클릭 했는지 오른쪽 버튼을 클릭 했는지에 따라서
			// bButton의 값이 TRUE와 FALSE로 정해진다.
			// 따라서 mousemove에서는 어떤 버튼이 눌려졌는지 bButton을 통해 알 수 있다.
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
		// 시작지점 선택
		//--------------------------------------------------------------------------
		// 윈도우의 작업 영역을 무효화시켜					
		// WM_PAINT 메시지를 해당 윈도우에 보냄	
		// 3번째 인자가, TRUE값 이면						
		// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
		// FALSE면 배경을 지우지 않은 채로 다시 그림		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iXTemp = LOWORD(lParam) / df_RECTINTERVAL;
		iYTemp = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// 시작 지점과 끝 지점이 같을 순 없다.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pAStar->m_iTempEndX && iYTemp == g_pAStar->m_iTempEndY)
			break;

		g_pAStar->m_bCheckStart = TRUE;
		g_pAStar->m_iTempStartX = iXTemp;
		g_pAStar->m_iTempStartY = iYTemp;

		//--------------------------------------------------------------------------
		// 선택한 타일이 block이어도 우선권은 시작지점 선택에 있음.
		// 따라서 해당 타일을 무조건 unblock으로 처리.
		//--------------------------------------------------------------------------
		g_pAStar->m_bTileArr[iYTemp][iXTemp] = 0;

		break;
	case WM_RBUTTONDBLCLK:
		//--------------------------------------------------------------------------
		// 윈도우의 작업 영역을 무효화시켜					
		// WM_PAINT 메시지를 해당 윈도우에 보냄	
		// 3번째 인자가, TRUE값 이면						
		// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
		// FALSE면 배경을 지우지 않은 채로 다시 그림		
		//--------------------------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iXTemp = LOWORD(lParam) / df_RECTINTERVAL;
		iYTemp = HIWORD(lParam) / df_RECTINTERVAL;

		//--------------------------------------------------------------------------
		// 시작 지점과 끝 지점이 같을 순 없다.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pAStar->m_iTempStartX && iYTemp == g_pAStar->m_iTempStartY)
			break;

		g_pAStar->m_bCheckEnd = TRUE;
		g_pAStar->m_iTempEndX = iXTemp;
		g_pAStar->m_iTempEndY = iYTemp;

		//--------------------------------------------------------------------------
		// 선택한 타일이 block이어도 우선권은 끝지점 선택에 있음.
		// 따라서 해당 타일을 무조건 unblock으로 처리.
		//--------------------------------------------------------------------------
		g_pAStar->m_bTileArr[iYTemp][iXTemp] = 0;

		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 메뉴 선택을 구문 분석합니다.
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
		// TODO: 여기에 그리기 코드를 추가합니다.
		
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

// 정보 대화 상자의 메시지 처리기입니다.
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


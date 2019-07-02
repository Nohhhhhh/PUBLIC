// JumpPointSearch.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "JumpPointSearch.h"
#include "JumpPoint.h"

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

CJumpPoint						*g_pJump = new CJumpPoint(50);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	srand((unsigned int)time(NULL));

	// TODO: 여기에 코드를 입력합니다.
	MSG msg;
	HACCEL hAccelTable;

	// 전역 문자열을 초기화합니다.
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_JUMPPOINTSEARCH, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JUMPPOINTSEARCH));

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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JUMPPOINTSEARCH));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_JUMPPOINTSEARCH);
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
		//-------------------------------------------------------
		// 일정 시간 마다 로직 반복
		//-------------------------------------------------------

		if (TRUE == bTimer)
		{
			//-------------------------------------------------------
			// 윈도우의 작업 영역을 무효화시켜					
			// WM_PAINT 메시지를 해당 윈도우에 보냄	
			// 3번째 인자가, TRUE값 이면						
			// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
			// FALSE면 배경을 지우지 않은 채로 다시 그림		
			//-------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			// 색상 변화를 주기 위해서
			g_pJump->m_iJumpIdx = rand() % 10;

			//-------------------------------------------------------
			// TRUE면 end node를 찾은 것임.
			// 따라서 timer flag를 false로 하고, find flag도 true로 함
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
			// 타일 초기화 상태 만들기
			//-------------------------------------------------------

			//-------------------------------------------------------
			// 윈도우의 작업 영역을 무효화시켜					
			// WM_PAINT 메시지를 해당 윈도우에 보냄	
			// 3번째 인자가, TRUE값 이면						
			// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
			// FALSE면 배경을 지우지 않은 채로 다시 그림		
			//-------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			//-------------------------------------------------------
			// tile을 초기화 상태로 만듬
			//-------------------------------------------------------
			g_pJump->InitTile();
			bFindDest = FALSE;
			break;
		case VK_RETURN:
			//-------------------------------------------------------
			// 길 찾기 시작
			//-------------------------------------------------------

			//-------------------------------------------------------
			// 시작 지점과 끝 지점이 모두 선택 되었을 때만 길 찾기 로직 수행
			//-------------------------------------------------------
			//if (g_pJump->m_bCheckStart == TRUE && g_pJump->m_bCheckEnd == TRUE)
			//{
			//	g_pJump->m_mmOpenList.clear();
			//	g_pJump->m_mmCloseList.clear();
			//	bFindDest = FALSE;

			//	//-------------------------------------------------------
			//	// 타일 맵은 블락 노드가 아닌 경우 모두 논블락 노드로 초기화
			//	//-------------------------------------------------------
			//	for (int iCnt1 = 0; iCnt1 < df_ARRAYMAPY; iCnt1++)
			//		for (int iCnt2 = 0; iCnt2 < df_ARRAYMAPX; iCnt2++)
			//			if (g_pJump->m_byTileAttribute[iCnt1][iCnt2] != df_BLOCK)
			//				g_pJump->m_byTileAttribute[iCnt1][iCnt2] = df_NONBLOCK;

			//	//-------------------------------------------------------
			//	// 시작 노드 생성 (시작 좌표, 도착 좌표 등 초기화)
			//	//-------------------------------------------------------
			//	g_pJump->MakeStartNode(g_pJump->m_iTempStartX, g_pJump->m_iTempStartY, g_pJump->m_iTempEndX, g_pJump->m_iTempEndY);
			//	
			//	bTimer = TRUE;
			//	SetTimer(hWnd, 1, 500, NULL);
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
					// 시작 노드 생성 (시작 좌표, 도착 좌표 등 초기화)
					//-------------------------------------------------------
					g_pJump->MakeStartNode(g_pJump->m_iTempStartX, g_pJump->m_iTempStartY, g_pJump->m_iTempEndX, g_pJump->m_iTempEndY);
					bTimer = TRUE;

					QueryPerformanceCounter(&lStartTime);

					while (bTimer)
					{
						//-------------------------------------------------------
						// 윈도우의 작업 영역을 무효화시켜					
						// WM_PAINT 메시지를 해당 윈도우에 보냄	
						// 3번째 인자가, TRUE값 이면						
						// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
						// FALSE면 배경을 지우지 않은 채로 다시 그림		
						//-------------------------------------------------------
						InvalidateRect(hWnd, NULL, FALSE);

						// 색상 변화를 주기 위해서
						g_pJump->m_iJumpIdx = rand() % 10;

						//-------------------------------------------------------
						// TRUE면 end node를 찾은 것임.
						// 따라서 timer flag를 false로 하고, find flag도 true로 함
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
			
			swprintf_s(szPerformance, sizeof(szPerformance), L"Avr: %0.4lf㎲ | Loop: %lld", (float)i64TotalTime / g_dMicroFrequency / i64Call, i64Call);
			SetWindowTextW(hWnd, szPerformance);

			break;
		}
		break;
	case WM_LBUTTONDOWN:
		//-------------------------------------------------------
		// 블럭 노드 선택
		//-------------------------------------------------------

		//-------------------------------------------------------
		// 윈도우의 작업 영역을 무효화시켜					
		// WM_PAINT 메시지를 해당 윈도우에 보냄	
		// 3번째 인자가, TRUE값 이면						
		// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
		// FALSE면 배경을 지우지 않은 채로 다시 그림		
		//-------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//-------------------------------------------------------
		// 왼쪽 마우스 버튼을 클릭 시 TRUE
		// bButton이 TRUE 면 무조건 block 상태로 만든다.
		//-------------------------------------------------------
		bButton = TRUE;

		if ((iBlockX != g_pJump->m_iStartX || iBlockY != g_pJump->m_iStartY) && (iBlockX != g_pJump->m_iEndX || iBlockY != g_pJump->m_iEndY))
			g_pJump->CheckBlockPoint(iBlockX, iBlockY, bButton);

		//-------------------------------------------------------
		// 마우스 클릭 후 이동할 때도 타일이 선택 되도록 하기 위함
		//-------------------------------------------------------
		bNowDraw = TRUE;

		break;
	case WM_RBUTTONDOWN:
		//-------------------------------------------------------
		// 논블럭 노드 선택
		//-------------------------------------------------------

		//-------------------------------------------------------
		// 윈도우의 작업 영역을 무효화시켜					
		// WM_PAINT 메시지를 해당 윈도우에 보냄	
		// 3번째 인자가, TRUE값 이면						
		// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
		// FALSE면 배경을 지우지 않은 채로 다시 그림		
		//-------------------------------------------------------
		InvalidateRect(hWnd, NULL, FALSE);

		iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
		iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

		//-------------------------------------------------------
		// 왼쪽 마우스 버튼을 클릭 시 FALSE
		// bButton이 FALSE 면 무조건 nonblock 상태로 만든다.
		//-------------------------------------------------------
		bButton = FALSE;

		if ((iBlockX != g_pJump->m_iStartX || iBlockY != g_pJump->m_iStartY) && (iBlockX != g_pJump->m_iEndX || iBlockY != g_pJump->m_iEndY))
			g_pJump->CheckBlockPoint(iBlockX, iBlockY, bButton);

		//-------------------------------------------------------
		// 마우스 클릭 후 이동할 때도 타일이 선택 되도록 하기 위함
		//-------------------------------------------------------
		bNowDraw = TRUE;
		break;
	case WM_MOUSEMOVE:
		//-------------------------------------------------------
		// 클릭 후 이동
		//-------------------------------------------------------

		//-------------------------------------------------------
		// nowdraw flag가 true면 그리기 타일 선택 가능
		//-------------------------------------------------------
		if (bNowDraw == TRUE)
		{
			// 블럭지점 선택
			//-------------------------------------------------------
			// 윈도우의 작업 영역을 무효화시켜					
			// WM_PAINT 메시지를 해당 윈도우에 보냄	
			// 3번째 인자가, TRUE값 이면						
			// 무효화 되기 전의 배경을 모두 지운 후, 다시 그림	
			// FALSE면 배경을 지우지 않은 채로 다시 그림		
			//-------------------------------------------------------
			InvalidateRect(hWnd, NULL, FALSE);

			iBlockX = LOWORD(lParam) / df_RECTINTERVAL;
			iBlockY = HIWORD(lParam) / df_RECTINTERVAL;

			//-------------------------------------------------------
			// 마우스 왼쪽 버튼을 클릭 했는지 오른쪽 버튼을 클릭 했는지에 따라서
			// bButton의 값이 TRUE와 FALSE로 정해진다.
			// 따라서 mousemove에서는 어떤 버튼이 눌려졌는지 bButton을 통해 알 수 있다.
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
		// 시작지점 선택
		//-------------------------------------------------------

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
		//iXTemp = 69;
		//iYTemp = 147;
		//--------------------------------------------------------------------------
		// 시작 지점과 끝 지점이 같을 순 없다.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pJump->m_iTempEndX && iYTemp == g_pJump->m_iTempEndY)
			break;

		g_pJump->m_bCheckStart = TRUE;
		g_pJump->m_iTempStartX = iXTemp;
		g_pJump->m_iTempStartY = iYTemp;

		//--------------------------------------------------------------------------
		// 선택한 타일이 block이어도 우선권은 시작지점 선택에 있음.
		// 따라서 해당 타일을 무조건 unblock으로 처리.
		//--------------------------------------------------------------------------
		g_pJump->m_byTileAttribute[iYTemp][iXTemp] = df_NONBLOCK;

		break;
	case WM_RBUTTONDBLCLK:
		//-------------------------------------------------------
		// 도착지점 선택
		//-------------------------------------------------------

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
		//iXTemp = 88;
		//iYTemp = 122;
		//--------------------------------------------------------------------------
		// 시작 지점과 끝 지점이 같을 순 없다.
		//--------------------------------------------------------------------------
		if (iXTemp == g_pJump->m_iTempStartX && iYTemp == g_pJump->m_iTempStartY)
			break;

		g_pJump->m_bCheckEnd = TRUE;
		g_pJump->m_iTempEndX = iXTemp;
		g_pJump->m_iTempEndY = iYTemp;

		//--------------------------------------------------------------------------
		// 선택한 타일이 block이어도 우선권은 끝지점 선택에 있음.
		// 따라서 해당 타일을 무조건 unblock으로 처리.
		//--------------------------------------------------------------------------
		g_pJump->m_byTileAttribute[iYTemp][iXTemp] = df_NONBLOCK;

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


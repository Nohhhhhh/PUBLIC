// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>
#include <windows.h>
#include <atltypes.h>
#include <windowsx.h>

// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cstdlib>
#include <time.h>

#define CHECK_TILE(X, Y) ((df_BLOCK != m_byTileAttribute[Y][X]) & (0 <= X) & (df_ARRAYMAPX > X) & (0 <= Y) & (df_ARRAYMAPY > Y))
// #define CHECK_TILE(X, Y) (df_BLOCK == m_byTileAttribute[Y][X] || 0 > X || df_ARRAYMAPX <= X || 0 > Y || df_ARRAYMAPY <= Y) ? false : true;
// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

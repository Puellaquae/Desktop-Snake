#include <windows.h>
#include <windef.h>
#include <commctrl.h>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <memoryapi.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define KEY_UP(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 0:1)
#define DIRECT_NONE 0x0F
#define DIRECT_UP 0x01
#define DIRECT_DOWN 0x02
#define DIRECT_LEFT 0x04
#define DIRECT_RIGHT 0x08
#define SNAKE_BODY 2
#define SNAKE_HEADER 1
#define NOT_SNAKE 0
#define NewGame() \
do{\
    moveDirect = DIRECT_NONE;\
    snakeLen = 1;\
    snakeTailIndex = 0;\
    snakeHeaderIndex = 0;\
	for (int i = 0; i < iconCnt; i++)\
	{\
		snakes[i].x = 0;\
		snakes[i].y = 0;\
		ListView_SetItemPosition(hSysListView32, i, -100, -100);\
	}\
	ListView_SetItemPosition(hSysListView32, 0, offsetX, offsetY); \
    isSnake = std::vector<std::vector<int>>(blockW, std::vector<int>(blockH, NOT_SNAKE));\
}\
while(0)
#define X2P(pos) (pos * stepX + offsetX)
#define Y2P(pos) (pos * stepY + offsetY)
int delay = 100;
int stepX = 70;
int stepY = 70;
bool playing = false;
bool pause = false;
int moveDirect = DIRECT_NONE;
DWORD WINAPI KeyProc(LPVOID args)
{
	while (true)
	{
		if (KEY_DOWN(VK_LEFT) && moveDirect & (DIRECT_DOWN | DIRECT_UP))
		{
			moveDirect = DIRECT_LEFT;
			while (KEY_DOWN(VK_LEFT));
		}
		else if (KEY_DOWN(VK_RIGHT) && moveDirect & (DIRECT_UP | DIRECT_DOWN))
		{
			moveDirect = DIRECT_RIGHT;
			while (KEY_DOWN(VK_RIGHT));
		}
		else if (KEY_DOWN(VK_UP) && moveDirect & (DIRECT_LEFT | DIRECT_RIGHT))
		{
			moveDirect = DIRECT_UP;
			while (KEY_DOWN(VK_UP));
		}
		else if (KEY_DOWN(VK_DOWN) && moveDirect & (DIRECT_LEFT | DIRECT_RIGHT))
		{
			moveDirect = DIRECT_DOWN;
			while (KEY_DOWN(VK_DOWN));
		}
		else if (KEY_DOWN('P'))
		{
			pause = !pause;
			while (KEY_DOWN('P'));
		}
		else if (KEY_DOWN('F'))
		{
			if (delay >= 10) { delay -= 10; }
			while (KEY_DOWN('F'));
		}
		else if (KEY_DOWN('S'))
		{
			delay += 10;
			while (KEY_DOWN('S'));
		}
		else if (KEY_DOWN(VK_ESCAPE))
		{
			pause = false;
			playing = false;
		}
	}
}
HWND hSysListView32 = NULL;
BOOL CALLBACK FindSysListView32(HWND hwnd, LPARAM lparam)
{
	TCHAR a[255];
	if (GetClassName(hwnd, a, 255) && lstrcmp(a, _T("WorkerW")) == 0)
	{
		HWND hSHELLDLL_DefView = ::FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
		hSysListView32 = ::FindWindowEx(hSHELLDLL_DefView, NULL, "SysListView32", "FolderView");
		return hSysListView32 == 0;
	}
	else
	{
		return TRUE;
	}
}
int iconCnt = 0;
PPOINT iconPrevPos;
void RecoveryIcon()
{
	for (int i = 0; i < iconCnt; i++)
	{
		ListView_SetItemPosition(hSysListView32, i, iconPrevPos[i].x, iconPrevPos[i].y);
	}
	ListView_RedrawItems(hSysListView32, 0, iconCnt - 1);
	::UpdateWindow(hSysListView32);
}
BOOL WINAPI BeforeExit(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
	{
		RecoveryIcon();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
int main()
{
#ifndef _DEBUG
	ShowWindow(GetForegroundWindow(), false);
#endif
	MessageBox(GetForegroundWindow(), "使用方向键移动\nF 键可加速，S 键可减速\nESC 键退出", "贪吃蛇", MB_OK);
	SetConsoleCtrlHandler(BeforeExit, TRUE);
	HWND hParent = ::FindWindow("Progman", "Program Manager");
	HWND hSHELLDLL_DefView = ::FindWindowEx(hParent, NULL, "SHELLDLL_DefView", NULL);
	hSysListView32 = ::FindWindowEx(hSHELLDLL_DefView, NULL, "SysListView32", "FolderView");
	if (hSysListView32 == 0)
	{
		HWND hDesktop = GetDesktopWindow();
		EnumWindows(FindSysListView32, 0);
	}
	if (hSysListView32 == 0)
	{
		MessageBox(GetForegroundWindow(), "初始化失败", "错误", MB_OK);
		return 0;
	}
	iconCnt = ListView_GetItemCount(hSysListView32);
	COLORREF x = ListView_GetTextColor(hSysListView32);
	int r = GetRValue(x);
	int g = GetGValue(x);
	int b = GetBValue(x);
	ListView_SetTextColor(hSysListView32, RGB(0, 0, 0));
	::UpdateWindow(hSysListView32);
	ListView_SetTextColor(hSysListView32, x);
	iconPrevPos = new POINT[iconCnt];
	CreateThread(NULL, 0, KeyProc, NULL, 0, 0);
	DWORD processId;
	GetWindowThreadProcessId(hSysListView32, &processId);
	HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processId);
	RECT rect;
	rect.left = LVIR_ICON; rect.right = 0; rect.bottom = 0; rect.top = 0;
	LPVOID prect = VirtualAllocEx(hProcess, NULL, sizeof(RECT), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, prect, &rect, sizeof(RECT), NULL);
	if ((BOOL)::SendMessage(hSysListView32, LVM_GETITEMRECT, 0, (LPARAM)prect))
	{
		ReadProcessMemory(hProcess, prect, &rect, sizeof(RECT), NULL);
		stepX = stepY = rect.bottom - rect.top;
	}
	VirtualFreeEx(hProcess, prect, 0, MEM_RELEASE);
	int realW = GetSystemMetrics(SM_CXFULLSCREEN);
	int realH = GetSystemMetrics(SM_CYFULLSCREEN);
	int blockW = realW / stepX;
	int blockH = realH / stepY;
	int offsetX = (realW - blockW * stepX) / 2;
	int offsetY = (realH - blockH * stepY) / 2;
	srand(time(0));
	playing = true;
	LPVOID ori = VirtualAllocEx(hProcess, NULL, sizeof(POINT), MEM_COMMIT, PAGE_READWRITE);
	for (int i = 0; i < iconCnt; i++)
	{
		ListView_GetItemPosition(hSysListView32, i, ori);
		ReadProcessMemory(hProcess, ori, iconPrevPos + i, sizeof(POINT), NULL);
	}
	VirtualFreeEx(hProcess, ori, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	std::vector<std::vector<int>> isSnake;
	PPOINT snakes = new POINT[iconCnt];
	int snakeLen, snakeTailIndex, snakeHeaderIndex;
	NewGame();
	bool newFood = true;
	int foodBlockX = 0;
	int foodBlockY = 0;
	int headerBlockX = 0;
	int headerBlockY = 0;
	clock_t watch;
	while (playing)
	{
		while (pause);
		watch = clock();
		if (!newFood && headerBlockX == foodBlockX && headerBlockY == foodBlockY)
		{
			snakeLen++;
			std::cout << snakeLen << std::endl;
			if (snakeLen == iconCnt)
			{
				char buf[255];
				sprintf_s(buf, "你已经把图标都吃完了，想要再来一遍吗？", snakeLen);
				int ans = MessageBox(GetForegroundWindow(), buf, "游戏结束", MB_YESNO | MB_ICONQUESTION);
				if (ans == IDYES)
				{
					NewGame();
					newFood = true;
				}
				else
				{
					break;
				}
			}
			newFood = true;
		}
		if (newFood)
		{
			do
			{
				foodBlockX = rand() % blockW;
				foodBlockY = rand() % blockH;
			} while (isSnake[foodBlockX][foodBlockY] != NOT_SNAKE);
			newFood = false;
			ListView_SetItemPosition(hSysListView32, snakeLen, X2P(foodBlockX), Y2P(foodBlockY));
#ifdef _DEBUG
			std::cout << "Food Pos:" << foodBlockX << "," << foodBlockY << std::endl;
#endif
		}
		if (moveDirect != DIRECT_NONE)
		{
			isSnake[snakes[snakeHeaderIndex].x][snakes[snakeHeaderIndex].y] = SNAKE_BODY;
			isSnake[snakes[snakeTailIndex].x][snakes[snakeTailIndex].y] = NOT_SNAKE;
			headerBlockX = snakes[snakeTailIndex].x = (snakes[snakeHeaderIndex].x
				+ (moveDirect == DIRECT_RIGHT ? 1 : moveDirect == DIRECT_LEFT ? -1 : 0) + blockW) % blockW;
			headerBlockY = snakes[snakeTailIndex].y = (snakes[snakeHeaderIndex].y
				+ (moveDirect == DIRECT_DOWN ? 1 : moveDirect == DIRECT_UP ? -1 : 0) + blockH) % blockH;
			if (isSnake[headerBlockX][headerBlockY] == SNAKE_BODY)
			{
				char buf[255];
				sprintf_s(buf, "%d分", snakeLen);
				int ans = MessageBox(GetForegroundWindow(), "想要重来吗？", buf, MB_YESNO | MB_ICONQUESTION);
				if (ans == IDYES)
				{
					NewGame();
					newFood = true;
				}
				else
				{
					break;
				}
			}
			isSnake[headerBlockX][headerBlockY] = SNAKE_HEADER;
			snakeHeaderIndex = snakeTailIndex;
			if (snakeTailIndex == 0) { snakeTailIndex = snakeLen - 1; }
			else { snakeTailIndex--; }
			ListView_SetItemPosition(hSysListView32, snakeHeaderIndex, X2P(headerBlockX), Y2P(headerBlockY));
#ifdef _DEBUG
			std::cout << "Snake Header Pos:" << headerBlockX << "," << headerBlockY << std::endl;
#endif
		}
		//ListView_RedrawItems(hSysListView32, 0, iconCnt - 1);
		::UpdateWindow(hSysListView32);
		Sleep((delay - clock() + watch) > 0 ? (delay - clock() + watch) : 0);
	}
	RecoveryIcon();
	delete[] snakes;
	delete[] iconPrevPos;
	return 0;
}
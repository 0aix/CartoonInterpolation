#include "stdafx.h"
#include <iostream>
#include <string>

#define WS_DEFAULT ((WS_OVERLAPPEDWINDOW | WS_SYSMENU) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME))
#define CLASSNAME "CartoonInterpolation"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_CLOSE_EVENT)
		Application::Uninitialize();
	return FALSE;
}

/**
Main message and console loop
**/
int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	SetConsoleTitle("CartoonInterpolation Console");

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASSNAME;
	wcex.hIconSm = NULL;

	if (!RegisterClassEx(&wcex))
		return 1;

	RECT rect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&rect, WS_DEFAULT, false);

	HWND hwnd = CreateWindow(CLASSNAME, CLASSNAME, WS_DEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

	if (!hwnd)
		return 2;

	MSG msg = { 0 };

	if (Application::Initialize(hwnd))
	{
		SetConsoleCtrlHandler(HandlerRoutine, TRUE);

		std::string path;

		std::cout << "png/obj name: " << std::flush;
		std::getline(std::cin, path);

		if (Application::LoadTexture(path + ".png") && 
			Application::LoadMesh(path + ".objx"))
		{
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);

			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
				{
					TranslateMessage(&msg); //Translates VK to WM_CHAR 
					DispatchMessageA(&msg); //Sends message to Window Proc
				}

				Application::Render();
			}
		}

		Application::Uninitialize();

		std::cout << "Application closed\n";
	}

	UnregisterClass(CLASSNAME, hInstance);

	std::string temp;
	std::getline(std::cin, temp);

	return (int)msg.wParam;
}

/**
Main window message handler
**/
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		Application::LeftDown(wParam, lParam);
		break;
	case WM_LBUTTONUP:
		Application::LeftUp(wParam, lParam);
		break;
	case WM_RBUTTONDOWN:
		Application::RightDown(wParam, lParam);
		break;
	case WM_RBUTTONUP:
		Application::RightUp(wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		Application::MouseMove(wParam, lParam);
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			Application::KillFocus();
		else if (wParam == VK_RETURN)
			Application::Update();
		else if (wParam == VK_TAB)
			Application::ToggleMesh();
		break;
	case WM_KILLFOCUS:
		Application::KillFocus();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		EndPaint(hwnd, NULL);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
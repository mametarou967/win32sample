#include "stdafx.h"
#include "win32sample.h"
#include "win32wndProc.h"
#include <windows.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        InitLines();
		CreateButtons(hWnd);
        break;
    case WM_SIZE:
        UpdateLayout(hWnd);
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_PAINT:
        DrawContents(hWnd);
	    break;
    case WM_LBUTTONDOWN:
		ButtonDown(hWnd,lParam);
	    break;
    case WM_LBUTTONUP:
		ButtonUp(hWnd);
        break;
    case WM_MOUSEMOVE:
		MouseMove(hWnd,lParam);
	    break;
    case WM_COMMAND:
		CommandExecute(hWnd,wParam);
        break;
    case WM_TIMER:
		TimerAction();
        break;
    case WM_DESTROY:
		DestroyAction(hWnd);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

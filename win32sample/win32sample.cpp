#include "stdafx.h"
#include "win32sample.h"
#include <windows.h>

#define MAX_LINE_COUNT 30
#define VISIBLE_LINE_COUNT 6
#define SCROLL_STEP 1
#define MAX_LINE_LENGTH 128

HINSTANCE g_hInst;
HWND g_hWnd;
HWND g_btnAgree = NULL;
HWND g_btnDisagree = NULL;

TCHAR g_lines[MAX_LINE_COUNT][MAX_LINE_LENGTH];
int g_scrollPos = 0;

RECT g_textArea;
RECT g_scrollBarArea;
RECT g_upButtonRect;
RECT g_downButtonRect;
RECT g_sliderRect;

BOOL g_draggingSlider = FALSE;
int g_dragOffsetY = 0;
BOOL g_draggingText = FALSE;
int g_dragStartY = 0;

HFONT g_hFont = NULL;
const int g_lineHeight = 24;
UINT_PTR g_timerId = 0;
BOOL g_scrollingUp = FALSE;
BOOL g_scrollingDown = FALSE;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void InitLines() {
	int i = 0;

    for (i = 0; i < MAX_LINE_COUNT; ++i) {
        wsprintf(g_lines[i], TEXT("\u30b5\u30f3\u30d7\u30eb\u6587\u7ae0 %d"), i + 1);
    }
}

void UpdateButtonState() {
	int maxScroll = 0;
	
	maxScroll = MAX_LINE_COUNT - VISIBLE_LINE_COUNT;
    if (g_scrollPos >= maxScroll) {
        EnableWindow(g_btnAgree, TRUE);
    } else {
        EnableWindow(g_btnAgree, FALSE);
    }
}

void UpdateLayout(HWND hWnd) {
    RECT client;
    GetClientRect(hWnd, &client);
	int width = 0;
	int height = 0;

	int areaHeight = 0;
	int areaWidth = 0;
	int areaLeft = 0;
	int areaTop = 0;

	int trackHeight = 0;
	int sliderHeight = 0;
	int maxScroll = 0;
	int sliderTop = 0;

    width = client.right - client.left;
    height = client.bottom - client.top;

    areaHeight = VISIBLE_LINE_COUNT * g_lineHeight + 20;
    areaWidth = width / 2;
    areaLeft = (width - areaWidth) / 2;
    areaTop = (height - areaHeight) / 2;

    g_textArea.left = areaLeft;
    g_textArea.top = areaTop;
    g_textArea.right = areaLeft + areaWidth - 60;
    g_textArea.bottom = areaTop + areaHeight;

    g_scrollBarArea.left = g_textArea.right + 10;
    g_scrollBarArea.top = g_textArea.top;
    g_scrollBarArea.right = g_textArea.right + 40;
    g_scrollBarArea.bottom = g_textArea.bottom;

    g_upButtonRect.left = g_scrollBarArea.left;
    g_upButtonRect.top = g_scrollBarArea.top;
    g_upButtonRect.right = g_scrollBarArea.right;
    g_upButtonRect.bottom = g_scrollBarArea.top + 30;

    g_downButtonRect.left = g_scrollBarArea.left;
    g_downButtonRect.top = g_scrollBarArea.bottom - 30;
    g_downButtonRect.right = g_scrollBarArea.right;
    g_downButtonRect.bottom = g_scrollBarArea.bottom;

    trackHeight = g_scrollBarArea.bottom - g_scrollBarArea.top - 60;
    sliderHeight = (trackHeight * VISIBLE_LINE_COUNT) / MAX_LINE_COUNT;
    if (sliderHeight < 20) sliderHeight = 20;
    maxScroll = MAX_LINE_COUNT - VISIBLE_LINE_COUNT;
    sliderTop = g_upButtonRect.bottom + ((trackHeight - sliderHeight) * g_scrollPos / maxScroll);

    g_sliderRect.left = g_scrollBarArea.left;
    g_sliderRect.top = sliderTop;
    g_sliderRect.right = g_scrollBarArea.right;
    g_sliderRect.bottom = sliderTop + sliderHeight;

    if (g_btnAgree && g_btnDisagree) {
        MoveWindow(g_btnAgree, areaLeft, g_textArea.bottom + 20, 100, 30, TRUE);
        MoveWindow(g_btnDisagree, areaLeft + 120, g_textArea.bottom + 20, 100, 30, TRUE);
    }
}

void DrawContent(HDC hdwnd) {
    RECT client;
	int width = 0;
	int height = 0;
	int startY = 0;
	int i = 0;

    GetClientRect(g_hWnd, &client);
    width = client.right - client.left;
    height = client.bottom - client.top;

    HDC memDC = CreateCompatibleDC(hdwnd);
    HBITMAP memBM = CreateCompatibleBitmap(hdwnd, width, height);
    HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

    FillRect(memDC, &client, (HBRUSH)(COLOR_WINDOW + 1));

    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HGDIOBJ oldPen = SelectObject(memDC, hPen);
    HGDIOBJ oldBrush = SelectObject(memDC, GetStockObject(NULL_BRUSH));
    Rectangle(memDC, g_textArea.left, g_textArea.top, g_scrollBarArea.left, g_textArea.bottom);
    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(hPen);

    SetBkMode(memDC, TRANSPARENT);
    HFONT oldFont = (HFONT)SelectObject(memDC, g_hFont);
    startY = g_textArea.top + 10;

    for (i = 0; i < VISIBLE_LINE_COUNT; ++i) {
		int index = 0;

        index = g_scrollPos + i;
        if (index >= MAX_LINE_COUNT) break;
        TextOut(memDC, g_textArea.left + 10, startY + i * g_lineHeight, g_lines[index], lstrlen(g_lines[index]));
    }

    FillRect(memDC, &g_upButtonRect, (HBRUSH)(COLOR_BTNFACE + 1));
    DrawText(memDC, TEXT("\u25b2"), -1, &g_upButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    FillRect(memDC, &g_downButtonRect, (HBRUSH)(COLOR_BTNFACE + 1));
    DrawText(memDC, TEXT("\u25bc"), -1, &g_downButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT trackRect;
    trackRect.left = g_scrollBarArea.left;
    trackRect.top = g_upButtonRect.bottom;
    trackRect.right = g_scrollBarArea.right;
    trackRect.bottom = g_downButtonRect.top;
    FillRect(memDC, &trackRect, (HBRUSH)(COLOR_SCROLLBAR + 1));

    FillRect(memDC, &g_sliderRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

    SelectObject(memDC, oldFont);
    BitBlt(hdwnd, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBM);
    DeleteObject(memBM);
    DeleteDC(memDC);
}

void ScrollText(int delta) {
	int newPos = 0;

    newPos = g_scrollPos + delta;
    newPos = max(0, min(newPos, MAX_LINE_COUNT - VISIBLE_LINE_COUNT));
    if (newPos != g_scrollPos) {
        g_scrollPos = newPos;
        UpdateLayout(g_hWnd);

        RECT redrawArea = {
            g_textArea.left,
            g_textArea.top,
            g_scrollBarArea.right,
            g_textArea.bottom
        };
        InvalidateRect(g_hWnd, &redrawArea, FALSE);
        UpdateButtonState();
    }
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    g_hInst = hInstance;

    g_hWnd = CreateWindow(L"MyWindowClass", L"Win32 Scroll Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL);

    LOGFONT lf = {};
    lf.lfHeight = -g_lineHeight;
    wcscpy_s(lf.lfFaceName, L"BIZ UDGothic");
    g_hFont = CreateFontIndirect(&lf);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(g_hFont);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        InitLines();
        g_btnAgree = CreateWindow(L"BUTTON", L"ìØà”Ç∑ÇÈ", WS_CHILD | WS_VISIBLE | WS_DISABLED,
            160, 280, 100, 30, hWnd, (HMENU)1, g_hInst, NULL);
        g_btnDisagree = CreateWindow(L"BUTTON", L"ìØà”ÇµÇ»Ç¢", WS_CHILD | WS_VISIBLE,
            280, 280, 100, 30, hWnd, (HMENU)2, g_hInst, NULL);
        UpdateLayout(hWnd);
        break;
    case WM_SIZE:
        UpdateLayout(hWnd);
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdwnd = BeginPaint(hWnd, &ps);
        DrawContent(hdwnd);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        if (PtInRect(&g_upButtonRect, pt)) {
            g_scrollingUp = TRUE;
            ScrollText(-SCROLL_STEP);
            g_timerId = SetTimer(hWnd, 1, 100, NULL);
        } else if (PtInRect(&g_downButtonRect, pt)) {
            g_scrollingDown = TRUE;
            ScrollText(SCROLL_STEP);
            g_timerId = SetTimer(hWnd, 1, 100, NULL);
        } else if (PtInRect(&g_sliderRect, pt)) {
            g_draggingSlider = TRUE;
            g_dragOffsetY = pt.y - g_sliderRect.top;
            SetCapture(hWnd);
        } else if (PtInRect(&g_textArea, pt)) {
            g_draggingText = TRUE;
            g_dragStartY = pt.y;
            SetCapture(hWnd);
        }
    }
    break;
    case WM_LBUTTONUP:
        g_draggingSlider = FALSE;
        g_draggingText = FALSE;
        g_scrollingUp = FALSE;
        g_scrollingDown = FALSE;
        if (g_timerId) {
            KillTimer(hWnd, g_timerId);
            g_timerId = 0;
        }
        ReleaseCapture();
        break;
    case WM_MOUSEMOVE:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        if (g_draggingSlider) {
			int trackTop = 0;
			int trackBottom = 0;
			int trackHeight = 0;
			int sliderHeight = 0;

			int newTop = 0;
			int sliderPos = 0;

            trackTop = g_upButtonRect.bottom;
            trackBottom = g_downButtonRect.top;
            trackHeight = trackBottom - trackTop;
            sliderHeight = g_sliderRect.bottom - g_sliderRect.top;

            newTop = pt.y - g_dragOffsetY;
            newTop = max(trackTop, min(newTop, trackBottom - sliderHeight));

            g_sliderRect.top = newTop;
            g_sliderRect.bottom = newTop + sliderHeight;

            sliderPos = newTop - trackTop;
            g_scrollPos = sliderPos * (MAX_LINE_COUNT - VISIBLE_LINE_COUNT) / (trackHeight - sliderHeight);

            RECT redrawArea = {
                g_textArea.left,
                g_textArea.top,
                g_scrollBarArea.right,
                g_textArea.bottom
            };
            InvalidateRect(hWnd, &redrawArea, FALSE);
            UpdateButtonState();
        } else if (g_draggingText) {
            int dy = pt.y - g_dragStartY;
            if (abs(dy) >= g_lineHeight) {
                ScrollText(-dy / g_lineHeight);
                g_dragStartY = pt.y;
            }
        }
    }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1:
            MessageBox(hWnd, L"Ç†ÇËÇ™Ç∆Ç§Ç≤Ç¥Ç¢Ç‹Ç∑ÅB", L"ìØà”", MB_OK);
            break;
        case 2:
            MessageBox(hWnd, L"ìØà”Ç≥ÇÍÇ‹ÇπÇÒÇ≈ÇµÇΩÅB", L"îÒìØà”", MB_OK);
            break;
        }
        break;
    case WM_TIMER:
        if (g_scrollingUp) ScrollText(-SCROLL_STEP);
        else if (g_scrollingDown) ScrollText(SCROLL_STEP);
        break;
    case WM_DESTROY:
        if (g_timerId) KillTimer(hWnd, g_timerId);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

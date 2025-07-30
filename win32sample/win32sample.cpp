#include "stdafx.h"
#include "win32sample.h"
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#define MAX_LINE_COUNT 30
#define VISIBLE_LINE_COUNT 6
#define SCROLL_STEP 1

HINSTANCE g_hInst;
HWND g_hWnd;
std::vector<std::wstring> g_lines;
int g_scrollPos = 0;

RECT g_textArea;
RECT g_scrollBarArea;
RECT g_upButtonRect;
RECT g_downButtonRect;
RECT g_sliderRect;

bool g_draggingSlider = false;
int g_dragOffsetY = 0;
bool g_draggingText = false;
int g_dragStartY = 0;

HFONT g_hFont = nullptr;
const int g_lineHeight = 24; // 調整可能
UINT_PTR g_timerId = 0;
bool g_scrollingUp = false;
bool g_scrollingDown = false;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void InitLines() {
    for (int i = 1; i <= MAX_LINE_COUNT; ++i) {
        std::wstringstream ss;
        ss << L"これはサンプルの文章 " << i;
        g_lines.push_back(ss.str());
    }
}

void UpdateLayout(HWND hWnd) {
    RECT client;
    GetClientRect(hWnd, &client);

    int width = client.right - client.left;
    int height = client.bottom - client.top;

    int areaHeight = VISIBLE_LINE_COUNT * g_lineHeight + 20;
    int areaWidth = width / 2;
    int areaLeft = (width - areaWidth) / 2;
    int areaTop = (height - areaHeight) / 2;

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

    int trackHeight = g_scrollBarArea.bottom - g_scrollBarArea.top - 60;
    int sliderHeight = (trackHeight * VISIBLE_LINE_COUNT) / MAX_LINE_COUNT;
    sliderHeight = max(sliderHeight, 20);
    int maxScroll = MAX_LINE_COUNT - VISIBLE_LINE_COUNT;
    int sliderTop = g_upButtonRect.bottom + ((trackHeight - sliderHeight) * g_scrollPos / maxScroll);

    g_sliderRect.left = g_scrollBarArea.left;
    g_sliderRect.top = sliderTop;
    g_sliderRect.right = g_scrollBarArea.right;
    g_sliderRect.bottom = sliderTop + sliderHeight;
}

void DrawContent(HDC hdc) {
    FillRect(hdc, &g_textArea, (HBRUSH)(COLOR_WINDOW + 1));
    SetBkMode(hdc, TRANSPARENT);

    HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFont);

    int startY = g_textArea.top + 10;
    for (int i = 0; i < VISIBLE_LINE_COUNT; ++i) {
        int index = g_scrollPos + i;
        if (index >= (int)g_lines.size()) break;
        TextOut(hdc, g_textArea.left + 10, startY + i * g_lineHeight, g_lines[index].c_str(), g_lines[index].length());
    }

    FillRect(hdc, &g_upButtonRect, (HBRUSH)(COLOR_BTNFACE + 1));
    DrawText(hdc, L"▲", -1, &g_upButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    FillRect(hdc, &g_downButtonRect, (HBRUSH)(COLOR_BTNFACE + 1));
    DrawText(hdc, L"▼", -1, &g_downButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT trackRect = { g_scrollBarArea.left, g_upButtonRect.bottom, g_scrollBarArea.right, g_downButtonRect.top };
    FillRect(hdc, &trackRect, (HBRUSH)(COLOR_SCROLLBAR + 1));

    FillRect(hdc, &g_sliderRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

    SelectObject(hdc, hOldFont);
}

void ScrollText(int delta) {
    int newPos = g_scrollPos + delta;
    newPos = max(0, min(newPos, MAX_LINE_COUNT - VISIBLE_LINE_COUNT));
    if (newPos != g_scrollPos) {
        g_scrollPos = newPos;
        UpdateLayout(g_hWnd);
        InvalidateRect(g_hWnd, NULL, TRUE);
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
    wcscpy_s(lf.lfFaceName, L"BIZ UDゴシック");
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
        UpdateLayout(hWnd);
        break;
    case WM_SIZE:
        UpdateLayout(hWnd);
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        DrawContent(hdc);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        if (PtInRect(&g_upButtonRect, pt)) {
            g_scrollingUp = true;
            ScrollText(-SCROLL_STEP);
            g_timerId = SetTimer(hWnd, 1, 100, NULL);
        } else if (PtInRect(&g_downButtonRect, pt)) {
            g_scrollingDown = true;
            ScrollText(SCROLL_STEP);
            g_timerId = SetTimer(hWnd, 1, 100, NULL);
        } else if (PtInRect(&g_sliderRect, pt)) {
            g_draggingSlider = true;
            g_dragOffsetY = pt.y - g_sliderRect.top;
            SetCapture(hWnd);
        } else if (PtInRect(&g_textArea, pt)) {
            g_draggingText = true;
            g_dragStartY = pt.y;
            SetCapture(hWnd);
        }
    }
    break;
    case WM_LBUTTONUP:
        g_draggingSlider = false;
        g_draggingText = false;
        g_scrollingUp = false;
        g_scrollingDown = false;
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
            int trackTop = g_upButtonRect.bottom;
            int trackBottom = g_downButtonRect.top;
            int trackHeight = trackBottom - trackTop;
            int sliderHeight = g_sliderRect.bottom - g_sliderRect.top;

            int newTop = pt.y - g_dragOffsetY;
            newTop = max(trackTop, min(newTop, trackBottom - sliderHeight));

            g_sliderRect.top = newTop;
            g_sliderRect.bottom = newTop + sliderHeight;

            int sliderPos = newTop - trackTop;
            g_scrollPos = sliderPos * (MAX_LINE_COUNT - VISIBLE_LINE_COUNT) / (trackHeight - sliderHeight);

            InvalidateRect(hWnd, NULL, TRUE);
        } else if (g_draggingText) {
            int dy = pt.y - g_dragStartY;
            if (abs(dy) >= g_lineHeight) {
                ScrollText(-dy / g_lineHeight);
                g_dragStartY = pt.y;
            }
        }
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

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
#define IDT_SCROLL_TIMER 1
#define SCROLL_INTERVAL 100

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
int g_scrollDirection = 0;  // -1: up, 1: down

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void InitLines() {
    for (int i = 1; i <= MAX_LINE_COUNT; ++i) {
        std::wstringstream ss;
        ss << L"Ç±ÇÍÇÕÉTÉìÉvÉãÇÃï∂èÕ " << i;
        g_lines.push_back(ss.str());
    }
}

void UpdateLayout(HWND hWnd) {
    RECT client;
    GetClientRect(hWnd, &client);

    int width = client.right - client.left;
    int height = client.bottom - client.top;

    int lineHeight = 20;
    int textAreaHeight = lineHeight * VISIBLE_LINE_COUNT + 20; // paddingè„â∫10Ç∏Ç¬
    int areaWidth = width / 2;

    int areaLeft = (width - areaWidth - 60) / 2; // 60: scrollbarïù+åÑä‘
    int areaTop = (height - textAreaHeight) / 2;

    g_textArea.left   = areaLeft;
    g_textArea.top    = areaTop;
    g_textArea.right  = areaLeft + areaWidth;
    g_textArea.bottom = areaTop + textAreaHeight;

    g_scrollBarArea.left   = g_textArea.right + 10;
    g_scrollBarArea.top    = g_textArea.top;
    g_scrollBarArea.right  = g_textArea.right + 40;
    g_scrollBarArea.bottom = g_textArea.bottom;

    g_upButtonRect.left   = g_scrollBarArea.left;
    g_upButtonRect.top    = g_scrollBarArea.top;
    g_upButtonRect.right  = g_scrollBarArea.right;
    g_upButtonRect.bottom = g_scrollBarArea.top + 30;

    g_downButtonRect.left   = g_scrollBarArea.left;
    g_downButtonRect.top    = g_scrollBarArea.bottom - 30;
    g_downButtonRect.right  = g_scrollBarArea.right;
    g_downButtonRect.bottom = g_scrollBarArea.bottom;

    int trackTop = g_upButtonRect.bottom;
    int trackBottom = g_downButtonRect.top;
    int trackHeight = trackBottom - trackTop;

    int sliderHeight = (trackHeight * VISIBLE_LINE_COUNT) / MAX_LINE_COUNT;
    if (sliderHeight < 10) sliderHeight = 10;
    int sliderTop = trackTop + (trackHeight - sliderHeight) * g_scrollPos / (MAX_LINE_COUNT - VISIBLE_LINE_COUNT);

    g_sliderRect.left   = g_scrollBarArea.left;
    g_sliderRect.top    = sliderTop;
    g_sliderRect.right  = g_scrollBarArea.right;
    g_sliderRect.bottom = sliderTop + sliderHeight;
}

void DrawContent(HDC hdc) {
    FillRect(hdc, &g_textArea, (HBRUSH)(COLOR_WINDOW + 1));
    SetBkMode(hdc, TRANSPARENT);

    int lineHeight = 20;
    int startY = g_textArea.top + 10;

    for (int i = 0; i < VISIBLE_LINE_COUNT; ++i) {
        int index = g_scrollPos + i;
        if (index >= (int)g_lines.size()) break;
        TextOut(hdc, g_textArea.left + 10, startY + i * lineHeight, g_lines[index].c_str(), g_lines[index].length());
    }

    FillRect(hdc, &g_upButtonRect, (HBRUSH)(COLOR_BTNFACE + 1));
    DrawText(hdc, L"Å£", -1, &g_upButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    FillRect(hdc, &g_downButtonRect, (HBRUSH)(COLOR_BTNFACE + 1));
    DrawText(hdc, L"Å•", -1, &g_downButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT trackRect = { g_scrollBarArea.left, g_upButtonRect.bottom, g_scrollBarArea.right, g_downButtonRect.top };
    FillRect(hdc, &trackRect, (HBRUSH)(COLOR_SCROLLBAR + 1));

    FillRect(hdc, &g_sliderRect, (HBRUSH)GetStockObject(GRAY_BRUSH));
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
    g_hWnd = CreateWindow(L"MyWindowClass", L"Win32 Scroll Demo", WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
                          NULL, NULL, hInstance, NULL);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
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
        POINTS pts = MAKEPOINTS(lParam);
        POINT pt = { pts.x, pts.y };

        if (PtInRect(&g_upButtonRect, pt)) {
            ScrollText(-SCROLL_STEP);
            g_scrollDirection = -1;
            SetTimer(hWnd, IDT_SCROLL_TIMER, SCROLL_INTERVAL, NULL);
        } else if (PtInRect(&g_downButtonRect, pt)) {
            ScrollText(SCROLL_STEP);
            g_scrollDirection = 1;
            SetTimer(hWnd, IDT_SCROLL_TIMER, SCROLL_INTERVAL, NULL);
        } else if (PtInRect(&g_sliderRect, pt)) {
            g_draggingSlider = true;
            g_dragOffsetY = pt.y - g_sliderRect.top;
            SetCapture(hWnd);
        }
    }
    break;
    case WM_LBUTTONUP:
        if (g_draggingSlider) {
            g_draggingSlider = false;
            ReleaseCapture();
        }
        KillTimer(hWnd, IDT_SCROLL_TIMER);
        g_scrollDirection = 0;
        break;
    case WM_MOUSEMOVE:
        if (g_draggingSlider) {
            POINTS pts = MAKEPOINTS(lParam);
            POINT pt = { pts.x, pts.y };

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

            UpdateLayout(hWnd);
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    case WM_TIMER:
        if (wParam == IDT_SCROLL_TIMER && g_scrollDirection != 0) {
            ScrollText(g_scrollDirection);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}
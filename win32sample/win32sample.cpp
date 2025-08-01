#include "stdafx.h"
#include "win32sample.h"
#include <windows.h>

#define FONT_HEIGHT         20   // フォントの高さ（20ピクセル）
#define LINE_SPACING        8    // 行間スペース（上下で余白）
#define LINE_HEIGHT         (FONT_HEIGHT + LINE_SPACING)

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

BOOL g_showContent = TRUE;       // 表示状態を保持
HWND g_btnToggle = NULL;         // 表示切替ボタン
BOOL g_draggingSlider = FALSE;
int g_dragOffsetY = 0;
BOOL g_draggingText = FALSE;
int g_dragStartY = 0;

HFONT g_hFont = NULL;
UINT_PTR g_timerId = 0;
BOOL g_scrollingUp = FALSE;
BOOL g_scrollingDown = FALSE;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void InitLines() {
	const TCHAR* sampleText[MAX_LINE_COUNT] = {
		TEXT("第1条　本規約はサービス利用条件を定めます。"),
		TEXT("第2条　ユーザーは本規約に同意の上、利用します。"),
		TEXT("第3条　サービス内容は予告なく変更されます。"),
		TEXT("第4条　当社は通知なくサービスを終了できます。"),
		TEXT("第5条　ユーザーは不正利用をしてはなりません。"),
		TEXT("第6条　本サービスの利用は自己責任とします。"),
		TEXT("第7条　当社は誠実に運営を行います。"),
		TEXT("第8条　当社は障害による損害を負いません。"),
		TEXT("第9条　他人の権利を侵害してはなりません。"),
		TEXT("第10条　他の利用者に迷惑をかけないこと。"),
		TEXT("第11条　得た情報の転載は禁止します。"),
		TEXT("第12条　アカウントは自己管理を行います。"),
		TEXT("第13条　パスワード管理は利用者の責任です。"),
		TEXT("第14条　必要に応じて連絡を行う場合があります。"),
		TEXT("第15条　個人情報は適切に管理されます。"),
		TEXT("第16条　全コンテンツは当社に帰属します。"),
		TEXT("第17条　複製・改変は禁止されています。"),
		TEXT("第18条　違反時は利用停止などの措置をとります。"),
		TEXT("第19条　権利の譲渡は禁止されています。"),
		TEXT("第20条　業務を外部に委託することがあります。"),
		TEXT("第21条　取引は利用者の責任で行ってください。"),
		TEXT("第22条　法令を遵守してサービスを利用します。"),
		TEXT("第23条　規約は改定されることがあります。"),
		TEXT("第24条　準拠法は日本法とします。"),
		TEXT("第25条　訴訟は東京地方裁判所を管轄とします。"),
		TEXT("第26条　未成年者は保護者の同意が必要です。"),
		TEXT("第27条　不適切な行為には措置を取ります。"),
		TEXT("第28条　安定運用に努めますが保証しません。"),
		TEXT("第29条　日本語規約が優先されます。"),
		TEXT("第30条　本規約は2025年8月1日より施行します。")
	};

	for (int i = 0; i < MAX_LINE_COUNT; ++i) {
		lstrcpyn(g_lines[i], sampleText[i], MAX_LINE_LENGTH);
	}
}


void SetContentVisible(BOOL visible) {
	int cmd = visible ? SW_SHOW : SW_HIDE;

	ShowWindow(g_btnAgree, cmd);
	ShowWindow(g_btnDisagree, cmd);
	// g_btnToggle は常に表示（消すとトグル不能になる）
	g_showContent = visible;
	InvalidateRect(g_hWnd, NULL, TRUE);
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
	int width = client.right - client.left;
	int height = client.bottom - client.top;

	int areaHeight = VISIBLE_LINE_COUNT * LINE_HEIGHT + 20;
	int areaTop = (height - areaHeight) / 2;

	int areaBaseWidth = width / 2;              // ★ 元の基準幅（初期値300）
	int textLeft = 200;                          // ★ 左端固定
	int textWidth = (int)(areaBaseWidth * 1.5); // ★ 1.8倍に拡大（たとえば540px）
	int spacing = 10;
	int scrollBarWidth = 30;

	// テキストエリア
	g_textArea.left = textLeft;
	g_textArea.top = areaTop;
	g_textArea.right = g_textArea.left + textWidth;
	g_textArea.bottom = g_textArea.top + areaHeight;

	// スクロールバーエリア
	g_scrollBarArea.left = g_textArea.right + spacing;
	g_scrollBarArea.top = g_textArea.top;
	g_scrollBarArea.right = g_scrollBarArea.left + scrollBarWidth;
	g_scrollBarArea.bottom = g_textArea.bottom;

	// 上ボタン
	g_upButtonRect.left = g_scrollBarArea.left;
	g_upButtonRect.top = g_scrollBarArea.top;
	g_upButtonRect.right = g_scrollBarArea.right;
	g_upButtonRect.bottom = g_scrollBarArea.top + 30;

	// 下ボタン
	g_downButtonRect.left = g_scrollBarArea.left;
	g_downButtonRect.top = g_scrollBarArea.bottom - 30;
	g_downButtonRect.right = g_scrollBarArea.right;
	g_downButtonRect.bottom = g_scrollBarArea.bottom;

	// スライダー
	int trackHeight = g_scrollBarArea.bottom - g_scrollBarArea.top - 60;
	int sliderHeight = (trackHeight * VISIBLE_LINE_COUNT) / MAX_LINE_COUNT;
	if (sliderHeight < 20) sliderHeight = 20;
	int maxScroll = MAX_LINE_COUNT - VISIBLE_LINE_COUNT;
	int sliderTop = g_upButtonRect.bottom + ((trackHeight - sliderHeight) * g_scrollPos / maxScroll);

	g_sliderRect.left = g_scrollBarArea.left;
	g_sliderRect.top = sliderTop;
	g_sliderRect.right = g_scrollBarArea.right;
	g_sliderRect.bottom = sliderTop + sliderHeight;

	// ボタン配置
	if (g_btnAgree && g_btnDisagree) {
		MoveWindow(g_btnAgree, textLeft, g_textArea.bottom + 20, 100, 30, TRUE);
		MoveWindow(g_btnDisagree, textLeft + 120, g_textArea.bottom + 20, 100, 30, TRUE);
	}
}



void DrawContent(HDC hdwnd) {
    RECT client;
	int width = 0;
	int height = 0;
	int startY = 0;
	int i = 0;

	if (!g_showContent) return; // 非表示時は描画しない

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
        TextOut(memDC, g_textArea.left + 10, startY + i * LINE_HEIGHT, g_lines[index], lstrlen(g_lines[index]));
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
		CW_USEDEFAULT, CW_USEDEFAULT, 1100, 500, // ★ 十分な幅に拡大
		NULL, NULL, hInstance, NULL);

	LOGFONT lf = {};
	lf.lfHeight = -FONT_HEIGHT;  // フォントサイズを固定（行間とは別）
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
        g_btnAgree = CreateWindow(L"BUTTON", L"同意する", WS_CHILD | WS_VISIBLE | WS_DISABLED,
            160, 280, 100, 30, hWnd, (HMENU)1, g_hInst, NULL);
        g_btnDisagree = CreateWindow(L"BUTTON", L"同意しない", WS_CHILD | WS_VISIBLE,
            280, 280, 100, 30, hWnd, (HMENU)2, g_hInst, NULL);
		g_btnToggle = CreateWindow(L"BUTTON", L"表示切替", WS_CHILD | WS_VISIBLE,
			600, 400, 100, 30, hWnd, (HMENU)3, g_hInst, NULL);
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
            if (abs(dy) >= LINE_HEIGHT) {
                ScrollText(-dy / LINE_HEIGHT);
                g_dragStartY = pt.y;
            }
        }
    }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1:
            MessageBox(hWnd, L"ありがとうございます。", L"同意", MB_OK);
            break;
        case 2:
            MessageBox(hWnd, L"同意されませんでした。", L"非同意", MB_OK);
            break;
		case 3: // 表示切替ボタン
			SetContentVisible(!g_showContent);
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

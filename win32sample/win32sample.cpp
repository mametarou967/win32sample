#include "stdafx.h"
#include "win32sample.h"
#include <windows.h>

#define FONT_HEIGHT         20   // �t�H���g�̍����i20�s�N�Z���j
#define LINE_SPACING        8    // �s�ԃX�y�[�X�i�㉺�ŗ]���j
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

BOOL g_showContent = TRUE;       // �\����Ԃ�ێ�
HWND g_btnToggle = NULL;         // �\���ؑփ{�^��
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
		TEXT("��1���@�{�K��̓T�[�r�X���p�������߂܂��B"),
		TEXT("��2���@���[�U�[�͖{�K��ɓ��ӂ̏�A���p���܂��B"),
		TEXT("��3���@�T�[�r�X���e�͗\���Ȃ��ύX����܂��B"),
		TEXT("��4���@���Ђ͒ʒm�Ȃ��T�[�r�X���I���ł��܂��B"),
		TEXT("��5���@���[�U�[�͕s�����p�����Ă͂Ȃ�܂���B"),
		TEXT("��6���@�{�T�[�r�X�̗��p�͎��ȐӔC�Ƃ��܂��B"),
		TEXT("��7���@���Ђ͐����ɉ^�c���s���܂��B"),
		TEXT("��8���@���Ђ͏�Q�ɂ�鑹�Q�𕉂��܂���B"),
		TEXT("��9���@���l�̌�����N�Q���Ă͂Ȃ�܂���B"),
		TEXT("��10���@���̗��p�҂ɖ��f�������Ȃ����ƁB"),
		TEXT("��11���@�������̓]�ڂ͋֎~���܂��B"),
		TEXT("��12���@�A�J�E���g�͎��ȊǗ����s���܂��B"),
		TEXT("��13���@�p�X���[�h�Ǘ��͗��p�҂̐ӔC�ł��B"),
		TEXT("��14���@�K�v�ɉ����ĘA�����s���ꍇ������܂��B"),
		TEXT("��15���@�l���͓K�؂ɊǗ�����܂��B"),
		TEXT("��16���@�S�R���e���c�͓��ЂɋA�����܂��B"),
		TEXT("��17���@�����E���ς͋֎~����Ă��܂��B"),
		TEXT("��18���@�ᔽ���͗��p��~�Ȃǂ̑[�u���Ƃ�܂��B"),
		TEXT("��19���@�����̏��n�͋֎~����Ă��܂��B"),
		TEXT("��20���@�Ɩ����O���Ɉϑ����邱�Ƃ�����܂��B"),
		TEXT("��21���@����͗��p�҂̐ӔC�ōs���Ă��������B"),
		TEXT("��22���@�@�߂����炵�ăT�[�r�X�𗘗p���܂��B"),
		TEXT("��23���@�K��͉��肳��邱�Ƃ�����܂��B"),
		TEXT("��24���@�����@�͓��{�@�Ƃ��܂��B"),
		TEXT("��25���@�i�ׂ͓����n���ٔ������Ǌ��Ƃ��܂��B"),
		TEXT("��26���@�����N�҂͕ی�҂̓��ӂ��K�v�ł��B"),
		TEXT("��27���@�s�K�؂ȍs�ׂɂ͑[�u�����܂��B"),
		TEXT("��28���@����^�p�ɓw�߂܂����ۏ؂��܂���B"),
		TEXT("��29���@���{��K�񂪗D�悳��܂��B"),
		TEXT("��30���@�{�K���2025�N8��1�����{�s���܂��B")
	};

	for (int i = 0; i < MAX_LINE_COUNT; ++i) {
		lstrcpyn(g_lines[i], sampleText[i], MAX_LINE_LENGTH);
	}
}


void SetContentVisible(BOOL visible) {
	int cmd = visible ? SW_SHOW : SW_HIDE;

	ShowWindow(g_btnAgree, cmd);
	ShowWindow(g_btnDisagree, cmd);
	// g_btnToggle �͏�ɕ\���i�����ƃg�O���s�\�ɂȂ�j
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

	int areaBaseWidth = width / 2;              // �� ���̊���i�����l300�j
	int textLeft = 200;                          // �� ���[�Œ�
	int textWidth = (int)(areaBaseWidth * 1.5); // �� 1.8�{�Ɋg��i���Ƃ���540px�j
	int spacing = 10;
	int scrollBarWidth = 30;

	// �e�L�X�g�G���A
	g_textArea.left = textLeft;
	g_textArea.top = areaTop;
	g_textArea.right = g_textArea.left + textWidth;
	g_textArea.bottom = g_textArea.top + areaHeight;

	// �X�N���[���o�[�G���A
	g_scrollBarArea.left = g_textArea.right + spacing;
	g_scrollBarArea.top = g_textArea.top;
	g_scrollBarArea.right = g_scrollBarArea.left + scrollBarWidth;
	g_scrollBarArea.bottom = g_textArea.bottom;

	// ��{�^��
	g_upButtonRect.left = g_scrollBarArea.left;
	g_upButtonRect.top = g_scrollBarArea.top;
	g_upButtonRect.right = g_scrollBarArea.right;
	g_upButtonRect.bottom = g_scrollBarArea.top + 30;

	// ���{�^��
	g_downButtonRect.left = g_scrollBarArea.left;
	g_downButtonRect.top = g_scrollBarArea.bottom - 30;
	g_downButtonRect.right = g_scrollBarArea.right;
	g_downButtonRect.bottom = g_scrollBarArea.bottom;

	// �X���C�_�[
	int trackHeight = g_scrollBarArea.bottom - g_scrollBarArea.top - 60;
	int sliderHeight = (trackHeight * VISIBLE_LINE_COUNT) / MAX_LINE_COUNT;
	if (sliderHeight < 20) sliderHeight = 20;
	int maxScroll = MAX_LINE_COUNT - VISIBLE_LINE_COUNT;
	int sliderTop = g_upButtonRect.bottom + ((trackHeight - sliderHeight) * g_scrollPos / maxScroll);

	g_sliderRect.left = g_scrollBarArea.left;
	g_sliderRect.top = sliderTop;
	g_sliderRect.right = g_scrollBarArea.right;
	g_sliderRect.bottom = sliderTop + sliderHeight;

	// �{�^���z�u
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

	if (!g_showContent) return; // ��\�����͕`�悵�Ȃ�

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
		CW_USEDEFAULT, CW_USEDEFAULT, 1100, 500, // �� �\���ȕ��Ɋg��
		NULL, NULL, hInstance, NULL);

	LOGFONT lf = {};
	lf.lfHeight = -FONT_HEIGHT;  // �t�H���g�T�C�Y���Œ�i�s�ԂƂ͕ʁj
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
        g_btnAgree = CreateWindow(L"BUTTON", L"���ӂ���", WS_CHILD | WS_VISIBLE | WS_DISABLED,
            160, 280, 100, 30, hWnd, (HMENU)1, g_hInst, NULL);
        g_btnDisagree = CreateWindow(L"BUTTON", L"���ӂ��Ȃ�", WS_CHILD | WS_VISIBLE,
            280, 280, 100, 30, hWnd, (HMENU)2, g_hInst, NULL);
		g_btnToggle = CreateWindow(L"BUTTON", L"�\���ؑ�", WS_CHILD | WS_VISIBLE,
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
            MessageBox(hWnd, L"���肪�Ƃ��������܂��B", L"����", MB_OK);
            break;
        case 2:
            MessageBox(hWnd, L"���ӂ���܂���ł����B", L"�񓯈�", MB_OK);
            break;
		case 3: // �\���ؑփ{�^��
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

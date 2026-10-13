#include "View.h"
#include "IController.h"
#include <sstream>
#include <iomanip>
#include <windowsx.h>

namespace {
    const int ID_BTN_INPUT = 101;
    const wchar_t* CLASS_NAME = L"MainWindowClass";
    const int WIN_W = 480, WIN_H = 420;
}

// ---------- helpers ----------
static HFONT MakeFont(int height, bool bold) {
    return CreateFontW(
        height, 0, 0, 0, bold ? FW_SEMIBOLD : FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
        L"Segoe UI");
}

static void RoundRectFill(HDC hdc, RECT r, int radius, COLORREF fill, COLORREF border) {
    HBRUSH br = CreateSolidBrush(fill);
    HPEN   pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldBr = SelectObject(hdc, br);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, r.left, r.top, r.right, r.bottom, radius, radius);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(br);
    DeleteObject(pen);
}

// ---------- window proc glue ----------
LRESULT CALLBACK MainWindow::WndProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }
    if (self) return self->WndProc(hWnd, msg, wParam, lParam);
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool MainWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WndProcStatic;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // фон рисуем сами
    RegisterClassW(&wc);

    RECT r{ 0, 0, WIN_W, WIN_H };
    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX) & ~WS_THICKFRAME;
    AdjustWindowRect(&r, style, FALSE);

    hwnd = CreateWindowExW(0, CLASS_NAME, L"Конвертер валют  BYN → EUR", style,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        nullptr, nullptr, hInstance, this);

    if (!hwnd) return false;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return true;
}

void MainWindow::CreateFonts() {
    fontTitle   = MakeFont(-24, true);
    fontCaption = MakeFont(-15, false);
    fontValue   = MakeFont(-16, true);
    fontResult  = MakeFont(-24, true);
    fontButton  = MakeFont(-16, true);
}

void MainWindow::CreateBrushes() {
    hbrBackground = CreateSolidBrush(colorBg);
    hbrCard       = CreateSolidBrush(colorCard);
}

void MainWindow::CreateControls(HWND hWnd) {
    const int marginX = 24;

    lblTitle = CreateWindowW(L"STATIC", L"Конвертер BYN \u2192 EUR",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        marginX, 20, WIN_W - marginX * 2, 34, hWnd, nullptr, nullptr, nullptr);
    SendMessageW(lblTitle, WM_SETFONT, (WPARAM)fontTitle, TRUE);

    // геометрия карточки. Высота каждого поля-значения подобрана с запасом
    // относительно высоты его шрифта, чтобы текст не обрезался сверху/снизу.
    const int cardTop = 68;
    const int rowCaptionH = 20;
    const int rowValueH   = 28;   // под fontValue (-16)
    const int resultValueH = 44;  // под fontResult (-24), с запасом

    const int row1CaptionY = cardTop + 18;
    const int row1ValueY   = row1CaptionY + rowCaptionH + 2;
    const int row2CaptionY = row1ValueY + rowValueH + 16;
    const int row2ValueY   = row2CaptionY + rowCaptionH + 2;

    dividerY = row2ValueY + rowValueH + 18;

    const int resultCaptionY = dividerY + 14;
    const int resultValueY   = resultCaptionY + rowCaptionH + 2;
    const int cardBottom     = resultValueY + resultValueH + 18;

    cardRect = { marginX, cardTop, WIN_W - marginX, cardBottom };

    auto makeCaption = [&](const wchar_t* text, int y) {
        HWND h = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
            cardRect.left + 20, y, 260, rowCaptionH, hWnd, nullptr, nullptr, nullptr);
        SendMessageW(h, WM_SETFONT, (WPARAM)fontCaption, TRUE);
        return h;
    };
    auto makeValue = [&](int y, int height, HFONT font) {
        HWND h = CreateWindowW(L"STATIC", L"\u2014",
            WS_CHILD | WS_VISIBLE | SS_RIGHT | SS_CENTERIMAGE,
            cardRect.left + 20, y, cardRect.right - cardRect.left - 40, height,
            hWnd, nullptr, nullptr, nullptr);
        SendMessageW(h, WM_SETFONT, (WPARAM)font, TRUE);
        return h;
    };

    lblCaptionSum  = makeCaption(L"Сумма (BYN)", row1CaptionY);
    lblSumValue    = makeValue(row1ValueY, rowValueH, fontValue);

    lblCaptionRate = makeCaption(L"Курс (BYN за 1 EUR)", row2CaptionY);
    lblRateValue   = makeValue(row2ValueY, rowValueH, fontValue);

    lblCaptionResult = makeCaption(L"Результат", resultCaptionY);
    lblResultValue   = makeValue(resultValueY, resultValueH, fontResult);

    btnInput = CreateWindowW(L"BUTTON", L"Ввод чисел",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        (WIN_W - 200) / 2, cardRect.bottom + 24, 200, 44,
        hWnd, (HMENU)(INT_PTR)ID_BTN_INPUT, nullptr, nullptr);
    SendMessageW(btnInput, WM_SETFONT, (WPARAM)fontButton, TRUE);
}

void MainWindow::UpdateLabels() {
    std::wostringstream sumStr, rateStr, resultStr;
    if (model.HasData()) {
        sumStr    << std::fixed << std::setprecision(2) << model.GetLastSum()  << L" BYN";
        rateStr   << std::fixed << std::setprecision(4) << model.GetLastRate() << L" BYN/EUR";
        resultStr << std::fixed << std::setprecision(2) << model.GetResult()   << L" \u20AC";
    } else {
        sumStr << L"\u2014"; rateStr << L"\u2014"; resultStr << L"\u2014";
    }
    SetWindowTextW(lblSumValue,    sumStr.str().c_str());
    SetWindowTextW(lblRateValue,   rateStr.str().c_str());
    SetWindowTextW(lblResultValue, resultStr.str().c_str());
}

void MainWindow::PaintBackground(HWND hWnd, HDC hdc) {
    RECT client; GetClientRect(hWnd, &client);
    FillRect(hdc, &client, hbrBackground);

    RoundRectFill(hdc, cardRect, 14, colorCard, colorBorder);

    // тонкий разделитель перед строкой результата
    HPEN pen = CreatePen(PS_SOLID, 1, colorBorder);
    HGDIOBJ old = SelectObject(hdc, pen);
    MoveToEx(hdc, cardRect.left + 20, dividerY, nullptr);
    LineTo(hdc, cardRect.right - 20, dividerY);
    SelectObject(hdc, old);
    DeleteObject(pen);
}

void MainWindow::DrawInputButton(LPDRAWITEMSTRUCT dis) {
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;
    COLORREF fill = pressed ? colorAccentDn : colorAccent;

    RECT r = dis->rcItem;
    HDC hdc = dis->hDC;

    RoundRectFill(hdc, r, 10, fill, fill);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    HGDIOBJ old = SelectObject(hdc, fontButton);

    wchar_t text[64];
    GetWindowTextW(dis->hwndItem, text, 64);
    DrawTextW(hdc, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, old);
}

void MainWindow::Cleanup() {
    if (fontTitle)   DeleteObject(fontTitle);
    if (fontCaption) DeleteObject(fontCaption);
    if (fontValue)   DeleteObject(fontValue);
    if (fontResult)  DeleteObject(fontResult);
    if (fontButton)  DeleteObject(fontButton);
    if (hbrBackground) DeleteObject(hbrBackground);
    if (hbrCard)       DeleteObject(hbrCard);
}

LRESULT MainWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateFonts();
        CreateBrushes();
        CreateControls(hWnd);
        return 0;

    case WM_ERASEBKGND:
        return 1; // рисуем фон сами в WM_PAINT, чтобы не было мерцания

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        PaintBackground(hWnd, hdc);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        HWND ctrl = (HWND)lParam;
        SetBkMode(hdcStatic, TRANSPARENT);

        if (ctrl == lblTitle) {
            SetTextColor(hdcStatic, colorTitle);
            return (LRESULT)hbrBackground;
        }
        if (ctrl == lblResultValue) {
            SetTextColor(hdcStatic, colorResult);
            return (LRESULT)hbrCard;
        }
        if (ctrl == lblCaptionSum || ctrl == lblCaptionRate || ctrl == lblCaptionResult) {
            SetTextColor(hdcStatic, colorCaption);
            return (LRESULT)hbrCard;
        }
        if (ctrl == lblSumValue || ctrl == lblRateValue) {
            SetTextColor(hdcStatic, colorValue);
            return (LRESULT)hbrCard;
        }
        return (LRESULT)hbrBackground;
    }

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
        if (dis->CtlID == ID_BTN_INPUT) {
            DrawInputButton(dis);
            return TRUE;
        }
        return FALSE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BTN_INPUT && controller) {
            controller->OnInputButtonClicked(hWnd);
        }
        return 0;

    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

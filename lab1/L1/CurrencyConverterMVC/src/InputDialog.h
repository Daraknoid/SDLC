#pragma once
#include <windows.h>
#include <string>
#include <sstream>
#include <stdexcept>

// ============================================================
//  Модальное окно ввода данных (динамическое создание окна,
//  без .rc-диалогов - только чистый WinAPI), оформленное в
//  одном визуальном стиле с главным окном.
//  Запоминает последние введённые значения и восстанавливает
//  их в полях ввода при повторном открытии.
// ============================================================
class InputDialog {
public:
    static bool Show(HWND owner, double initialSum, double initialRate,
                      double& out_sum, double& out_rate) {
        InputDialog dlg(owner, initialSum, initialRate);
        return dlg.RunModal(out_sum, out_rate);
    }

private:
    InputDialog(HWND owner, double initialSum, double initialRate)
        : ownerWnd(owner), sum(initialSum), rate(initialRate) {}

    HWND ownerWnd;
    HWND hwnd     = nullptr;
    HWND editSum  = nullptr;
    HWND editRate = nullptr;
    HWND btnOk     = nullptr;
    HWND btnCancel = nullptr;
    double sum, rate;
    bool okPressed = false;
    bool closed    = false;

    // стиль
    HFONT fontLabel  = nullptr;
    HFONT fontEdit   = nullptr;
    HFONT fontButton = nullptr;
    HBRUSH hbrBg     = nullptr;
    COLORREF colorBg     = RGB(0xFF, 0xFF, 0xFF);
    COLORREF colorLabel  = RGB(0x4A, 0x4E, 0x58);
    COLORREF colorAccent = RGB(0x2F, 0x6F, 0xED);
    COLORREF colorAccentDn = RGB(0x25, 0x59, 0xC2);
    COLORREF colorCancelBorder = RGB(0xD5, 0xD8, 0xE0);
    COLORREF colorCancelText   = RGB(0x4A, 0x4E, 0x58);

    static const int ID_EDIT_SUM = 1001;
    static const int ID_EDIT_RATE = 1002;

    static HFONT MakeFont(int height, bool bold) {
        return CreateFontW(height, 0, 0, 0, bold ? FW_SEMIBOLD : FW_NORMAL,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
            L"Segoe UI");
    }

    static void RoundRectFill(HDC hdc, RECT r, int radius, COLORREF fill, COLORREF border) {
        HBRUSH br = CreateSolidBrush(fill);
        HPEN pen = CreatePen(PS_SOLID, 1, border);
        HGDIOBJ oldBr = SelectObject(hdc, br);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        RoundRect(hdc, r.left, r.top, r.right, r.bottom, radius, radius);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(br);
        DeleteObject(pen);
    }

    static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        InputDialog* self = nullptr;
        if (msg == WM_NCCREATE) {
            CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = reinterpret_cast<InputDialog*>(cs->lpCreateParams);
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        } else {
            self = reinterpret_cast<InputDialog*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
        }
        if (self) return self->WndProc(hWnd, msg, wParam, lParam);
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    void DrawButton(LPDRAWITEMSTRUCT dis) {
        bool pressed = (dis->itemState & ODS_SELECTED) != 0;
        RECT r = dis->rcItem;
        HDC hdc = dis->hDC;

        if (dis->CtlID == IDOK) {
            RoundRectFill(hdc, r, 8, pressed ? colorAccentDn : colorAccent, pressed ? colorAccentDn : colorAccent);
            SetTextColor(hdc, RGB(255, 255, 255));
        } else {
            RoundRectFill(hdc, r, 8, colorBg, colorCancelBorder);
            SetTextColor(hdc, colorCancelText);
        }
        SetBkMode(hdc, TRANSPARENT);
        HGDIOBJ old = SelectObject(hdc, fontButton);
        wchar_t text[32];
        GetWindowTextW(dis->hwndItem, text, 32);
        DrawTextW(hdc, text, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, old);
    }

    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            fontLabel  = MakeFont(-15, false);
            fontEdit   = MakeFont(-16, false);
            fontButton = MakeFont(-15, true);
            hbrBg      = CreateSolidBrush(colorBg);

            HWND lbl1 = CreateWindowW(L"STATIC", L"Сумма (BYN)", WS_CHILD | WS_VISIBLE,
                24, 24, 260, 20, hWnd, nullptr, nullptr, nullptr);
            SendMessageW(lbl1, WM_SETFONT, (WPARAM)fontLabel, TRUE);

            editSum = CreateWindowExW(0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                24, 46, 292, 30, hWnd, (HMENU)ID_EDIT_SUM, nullptr, nullptr);
            SendMessageW(editSum, WM_SETFONT, (WPARAM)fontEdit, TRUE);

            HWND lbl2 = CreateWindowW(L"STATIC", L"Курс (BYN за 1 EUR)", WS_CHILD | WS_VISIBLE,
                24, 92, 260, 20, hWnd, nullptr, nullptr, nullptr);
            SendMessageW(lbl2, WM_SETFONT, (WPARAM)fontLabel, TRUE);

            editRate = CreateWindowExW(0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                24, 114, 292, 30, hWnd, (HMENU)ID_EDIT_RATE, nullptr, nullptr);
            SendMessageW(editRate, WM_SETFONT, (WPARAM)fontEdit, TRUE);

            if (sum > 0.0)  SetWindowTextW(editSum,  DoubleToWStr(sum).c_str());
            if (rate > 0.0) SetWindowTextW(editRate, DoubleToWStr(rate).c_str());

            btnCancel = CreateWindowW(L"BUTTON", L"Отмена",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                24, 168, 138, 38, hWnd, (HMENU)IDCANCEL, nullptr, nullptr);

            btnOk = CreateWindowW(L"BUTTON", L"OK",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | BS_DEFPUSHBUTTON,
                178, 168, 138, 38, hWnd, (HMENU)IDOK, nullptr, nullptr);

            SetFocus(editSum);
            return 0;
        }
        case WM_ERASEBKGND: {
            RECT r; GetClientRect(hWnd, &r);
            FillRect((HDC)wParam, &r, hbrBg);
            return 1;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, colorLabel);
            return (LRESULT)hbrBg;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlID == IDOK || dis->CtlID == IDCANCEL) {
                DrawButton(dis);
                return TRUE;
            }
            return FALSE;
        }
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == IDOK) {
                if (TryReadValues()) {
                    okPressed = true;
                    closed = true;
                    DestroyWindow(hWnd);
                } else {
                    MessageBoxW(hWnd,
                        L"Введите корректные положительные числа в оба поля.",
                        L"Ошибка ввода", MB_OK | MB_ICONERROR);
                }
            } else if (id == IDCANCEL) {
                okPressed = false;
                closed = true;
                DestroyWindow(hWnd);
            }
            return 0;
        }
        case WM_CLOSE:
            okPressed = false;
            closed = true;
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            if (fontLabel)  DeleteObject(fontLabel);
            if (fontEdit)   DeleteObject(fontEdit);
            if (fontButton) DeleteObject(fontButton);
            if (hbrBg)      DeleteObject(hbrBg);
            return 0;
        }
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }

    bool TryReadValues() {
        wchar_t bufSum[64];
        wchar_t bufRate[64];
        GetWindowTextW(editSum, bufSum, 64);
        GetWindowTextW(editRate, bufRate, 64);
        try {
            double s = ParseDouble(bufSum);
            double r = ParseDouble(bufRate);
            if (s <= 0.0 || r <= 0.0) return false;
            sum = s;
            rate = r;
            return true;
        } catch (...) {
            return false;
        }
    }

    static double ParseDouble(const std::wstring& text) {
        std::wstring trimmed = text;
        for (auto& c : trimmed) if (c == L',') c = L'.';
        if (trimmed.empty()) throw std::invalid_argument("empty");
        size_t idx = 0;
        double val = std::stod(trimmed, &idx);
        if (idx != trimmed.size()) throw std::invalid_argument("trailing chars");
        return val;
    }

    static std::wstring DoubleToWStr(double v) {
        std::wostringstream oss;
        oss << v;
        return oss.str();
    }

    bool RunModal(double& out_sum, double& out_rate) {
        static bool classRegistered = false;
        const wchar_t* className = L"InputDialogWndClass";
        if (!classRegistered) {
            WNDCLASSW wc = {};
            wc.lpfnWndProc   = WndProcStatic;
            wc.hInstance     = GetModuleHandle(nullptr);
            wc.lpszClassName = className;
            wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
            RegisterClassW(&wc);
            classRegistered = true;
        }

        EnableWindow(ownerWnd, FALSE);

        RECT ownerRect;
        GetWindowRect(ownerWnd, &ownerRect);
        const int width = 344, height = 244;
        RECT r{ 0, 0, width, height };
        AdjustWindowRectEx(&r, WS_POPUP | WS_CAPTION | WS_SYSMENU, FALSE, WS_EX_DLGMODALFRAME);
        const int x = ownerRect.left + (ownerRect.right - ownerRect.left - (r.right - r.left)) / 2;
        const int y = ownerRect.top  + (ownerRect.bottom - ownerRect.top - (r.bottom - r.top)) / 2;

        hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, className, L"Ввод данных",
            WS_POPUP | WS_CAPTION | WS_SYSMENU,
            x, y, r.right - r.left, r.bottom - r.top, ownerWnd, nullptr,
            GetModuleHandle(nullptr), this);

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        MSG msg;
        while (!closed && GetMessageW(&msg, nullptr, 0, 0)) {
            if (!IsDialogMessageW(hwnd, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

        EnableWindow(ownerWnd, TRUE);
        SetForegroundWindow(ownerWnd);

        if (okPressed) {
            out_sum  = sum;
            out_rate = rate;
        }
        return okPressed;
    }
};

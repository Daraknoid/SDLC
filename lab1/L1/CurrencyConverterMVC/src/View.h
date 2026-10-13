#pragma once
#include <windows.h>
#include "Model.h"

class IController;

// ============================================================
//  VIEW: главное окно приложения (стилизованный UI: карточка,
//  акцентные цвета, шрифт Segoe UI, кастомная кнопка).
//  Реализует IModelObserver -> подписывается на модель и
//  обновляет метки, когда модель сама сообщает об изменении.
// ============================================================
class MainWindow : public IModelObserver {
public:
    explicit MainWindow(CurrencyModel& model) : model(model) {
        model.AddObserver(this);
    }

    void SetController(IController* c) { controller = c; }

    bool Create(HINSTANCE hInstance, int nCmdShow);

    HWND GetHwnd() const { return hwnd; }

    // IModelObserver
    void OnModelChanged() override { UpdateLabels(); }

private:
    static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void CreateFonts();
    void CreateBrushes();
    void CreateControls(HWND hWnd);
    void UpdateLabels();
    void PaintBackground(HWND hWnd, HDC hdc);
    void DrawInputButton(LPDRAWITEMSTRUCT dis);
    void Cleanup();

    CurrencyModel& model;
    IController* controller = nullptr;

    HWND hwnd = nullptr;

    // заголовок
    HWND lblTitle = nullptr;

    // подписи (серые) и значения (тёмные) внутри карточки
    HWND lblCaptionSum = nullptr, lblSumValue = nullptr;
    HWND lblCaptionRate = nullptr, lblRateValue = nullptr;
    HWND lblCaptionResult = nullptr, lblResultValue = nullptr;

    HWND btnInput = nullptr;

    // геометрия карточки (используется и в WM_PAINT, и при
    // расстановке контролов)
    RECT cardRect{};
    int dividerY = 0;

    // шрифты
    HFONT fontTitle   = nullptr;
    HFONT fontCaption = nullptr;
    HFONT fontValue   = nullptr;
    HFONT fontResult  = nullptr;
    HFONT fontButton  = nullptr;

    // кисти/цвета
    HBRUSH hbrBackground = nullptr; // светлый фон окна
    HBRUSH hbrCard       = nullptr; // белый фон карточки
    COLORREF colorBg       = RGB(0xF3, 0xF4, 0xF8);
    COLORREF colorCard     = RGB(0xFF, 0xFF, 0xFF);
    COLORREF colorBorder   = RGB(0xE2, 0xE4, 0xEC);
    COLORREF colorTitle    = RGB(0x22, 0x24, 0x2A);
    COLORREF colorCaption  = RGB(0x8A, 0x8F, 0x9C);
    COLORREF colorValue    = RGB(0x2A, 0x2C, 0x33);
    COLORREF colorResult   = RGB(0x15, 0x8A, 0x5C);
    COLORREF colorAccent   = RGB(0x2F, 0x6F, 0xED);
    COLORREF colorAccentDn = RGB(0x25, 0x59, 0xC2);
};

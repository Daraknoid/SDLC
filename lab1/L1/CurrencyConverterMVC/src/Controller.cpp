#include "Controller.h"
#include "InputDialog.h"
#include <stdexcept>

void AppController::OnInputButtonClicked(HWND parentWnd) {
    double sum  = model.HasData() ? model.GetLastSum()  : 0.0;
    double rate = model.HasData() ? model.GetLastRate() : 0.0;

    double newSum = 0.0, newRate = 0.0;
    if (InputDialog::Show(parentWnd, sum, rate, newSum, newRate)) {
        try {
            model.SetData(newSum, newRate);   // модель сама уведомит View
        } catch (const std::exception& ex) {
            MessageBoxA(parentWnd, ex.what(), "Ошибка", MB_OK | MB_ICONERROR);
        }
    }
}

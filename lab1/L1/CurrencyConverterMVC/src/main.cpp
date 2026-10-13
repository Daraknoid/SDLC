#include <windows.h>
#include "Model.h"
#include "View.h"
#include "Controller.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    CurrencyModel  model;               // Активная модель
    MainWindow     view(model);         // Представление (подписано на модель)
    AppController  controller(model);   // Контроллер

    view.SetController(&controller);

    if (!view.Create(hInstance, nCmdShow)) {
        return 1;
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

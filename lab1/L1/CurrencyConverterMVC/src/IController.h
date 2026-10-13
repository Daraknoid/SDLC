#pragma once
#include <windows.h>

// View знает только этот интерфейс и не зависит от конкретной
// реализации контроллера (слабая связанность MVC-компонентов).
class IController {
public:
    virtual void OnInputButtonClicked(HWND parentWnd) = 0;
    virtual ~IController() {}
};

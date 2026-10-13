#pragma once
#include "IController.h"
#include "Model.h"

// ============================================================
//  CONTROLLER: реагирует на действия пользователя (клик по
//  кнопке "Ввод чисел"), открывает окно ввода и передаёт данные
//  в модель. Обновлением View контроллер не занимается - это
//  делает сама модель (активная модель), рассылая уведомления.
// ============================================================
class AppController : public IController {
public:
    explicit AppController(CurrencyModel& model) : model(model) {}

    void OnInputButtonClicked(HWND parentWnd) override;

private:
    CurrencyModel& model;
};

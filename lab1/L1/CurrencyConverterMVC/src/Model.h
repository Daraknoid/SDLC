#pragma once
#include <vector>
#include <stdexcept>

// ============================================================
//  Интерфейс наблюдателя. View реализует этот интерфейс, чтобы
//  Model могла оповещать его об изменении данных.
// ============================================================
class IModelObserver {
public:
    virtual void OnModelChanged() = 0;
    virtual ~IModelObserver() {}
};

// ============================================================
//  АКТИВНАЯ МОДЕЛЬ (Active Model)
//  Модель сама содержит бизнес-логику (конвертацию валют) и
//  САМА оповещает подписчиков (View) о своём изменении.
//  Controller лишь передаёт данные от пользователя в модель,
//  но не занимается обновлением View напрямую - это делает
//  сама модель через рассылку уведомлений (Observer pattern).
// ============================================================
class CurrencyModel {
public:
    void AddObserver(IModelObserver* observer) {
        observers.push_back(observer);
    }

    // Установить новые исходные данные и выполнить конвертацию.
    // При некорректных данных бросает std::invalid_argument.
    void SetData(double sumByn, double rateEur) {
        if (sumByn <= 0.0 || rateEur <= 0.0) {
            throw std::invalid_argument(
                "Сумма и курс должны быть положительными числами");
        }

        lastSum  = sumByn;
        lastRate = rateEur;
        result   = sumByn / rateEur;   // BYN -> EUR
        hasData  = true;

        NotifyObservers();   // <-- активная модель сама уведомляет View
    }

    bool   HasData()     const { return hasData; }
    double GetLastSum()  const { return lastSum; }
    double GetLastRate() const { return lastRate; }
    double GetResult()   const { return result; }

private:
    void NotifyObservers() {
        for (auto* observer : observers) {
            if (observer) observer->OnModelChanged();
        }
    }

    std::vector<IModelObserver*> observers;

    bool   hasData  = false;
    double lastSum  = 0.0;
    double lastRate = 0.0;
    double result   = 0.0;
};

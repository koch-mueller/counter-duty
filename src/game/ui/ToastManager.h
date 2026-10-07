#pragma once

#include <deque>
#include <string>

// Darstellungsart einer kurz eingeblendeten HUD-Nachricht
enum class ToastType
{
    Info,
    Success,
    Warning,
    Error
};

// Text, Typ und verbleibende Anzeigezeit eines Toasts
struct ToastMessage
{
    std::string text;

    ToastType type = ToastType::Info;

    float timeRemaining = 0.0f;
};

// Verwaltet zeitlich begrenzte HUD-Meldungen
class ToastManager
{
public:
    ToastManager();

    void push(const std::string& text, ToastType type, float duration);

    void update(float deltaTime);

    void clear();

    bool hasCurrentToast() const;

    const ToastMessage& currentToast() const;

private:
    void showNextToast();

    std::deque<ToastMessage> m_queue;

    ToastMessage m_currentToast;

    bool m_hasCurrentToast;
};

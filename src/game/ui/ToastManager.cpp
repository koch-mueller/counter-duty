#include "./ToastManager.h"

ToastManager::ToastManager()
    : m_hasCurrentToast(false)
{}

void ToastManager::push(const std::string& text, ToastType type, float duration)
{
    if (text.empty() || duration <= 0.0f)
    {
        return;
    }

    // Gleiche Meldungen direkt hintereinander werden unterdrückt, damit Ereignisse nicht durch frameweise wiederholte push-Aufrufe die Queue füllen
    if (m_hasCurrentToast && m_currentToast.text == text)
    {
        return;
    }

    if (!m_queue.empty() && m_queue.back().text == text)
    {
        return;
    }

    ToastMessage toast;

    toast.text = text;
    toast.type = type;
    toast.timeRemaining = duration;

    m_queue.push_back(toast);

    if (!m_hasCurrentToast)
    {
        showNextToast();
    }
}

void ToastManager::update(float deltaTime)
{
    if (!m_hasCurrentToast || deltaTime <= 0.0f)
    {
        return;
    }

    m_currentToast.timeRemaining -= deltaTime;

    if (m_currentToast.timeRemaining > 0.0f)
    {
        return;
    }

    m_hasCurrentToast = false;

    showNextToast();
}

void ToastManager::clear()
{
    m_queue.clear();

    m_currentToast = ToastMessage();

    m_hasCurrentToast = false;
}

bool ToastManager::hasCurrentToast() const
{
    return m_hasCurrentToast;
}

const ToastMessage& ToastManager::currentToast() const
{
    return m_currentToast;
}

// Toasts werden strikt nacheinander angezeigt; nur eine Meldung ist gleichzeitig aktiv
void ToastManager::showNextToast()
{
    if (m_queue.empty())
    {
        m_currentToast = ToastMessage();

        m_hasCurrentToast = false;

        return;
    }

    m_currentToast = m_queue.front();

    m_queue.pop_front();

    m_hasCurrentToast = true;
}

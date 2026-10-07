#include "PauseMenu.h"

namespace
{
    constexpr std::size_t kMenuItemCount = 4;

    constexpr std::size_t kVolumeItemIndex = 1;
}

PauseMenu::PauseMenu()
    : m_open(false),
      m_selectedIndex(0),
      m_pendingAction(PauseMenuAction::None)
{
}

void PauseMenu::open()
{
    m_open = true;

    m_selectedIndex = 0;

    m_pendingAction = PauseMenuAction::None;
}

void PauseMenu::close()
{
    m_open = false;

    m_pendingAction = PauseMenuAction::None;
}

bool PauseMenu::isOpen() const
{
    return m_open;
}

// Navigation und bestätigte Aktion werden getrennt gespeichert; Application konsumiert die Aktion anschließend genau einmal
void PauseMenu::update(bool moveUpPressed, bool moveDownPressed, bool confirmPressed)
{
    if (!m_open)
    {
        return;
    }

    if (moveUpPressed)
    {
        if (m_selectedIndex == 0)
        {
            m_selectedIndex = kMenuItemCount - 1;
        }
        else
        {
            --m_selectedIndex;
        }
    }
    else if (moveDownPressed)
    {
        m_selectedIndex = (m_selectedIndex + 1) % kMenuItemCount;
    }

    if (!confirmPressed)
    {
        return;
    }

    switch (m_selectedIndex)
    {
        case 0:
            m_pendingAction = PauseMenuAction::Resume;
            break;

        case 1:
            break;

        case 2:
            m_pendingAction = PauseMenuAction::MainMenu;
            break;

        case 3:
            m_pendingAction = PauseMenuAction::Quit;
            break;
    }
}

std::size_t PauseMenu::selectedIndex() const
{
    return m_selectedIndex;
}

bool PauseMenu::isVolumeSelected() const
{
    return m_selectedIndex == kVolumeItemIndex;
}

PauseMenuAction PauseMenu::consumeAction()
{
    PauseMenuAction action = m_pendingAction;

    m_pendingAction = PauseMenuAction::None;

    return action;
}

#include "MainMenu.h"

MainMenu::MainMenu()
    : m_selectedItem(MainMenuItem::Start)
{
}

// Die Auswahl läuft zyklisch über Start, Lautstärke und Beenden
void MainMenu::selectPrevious()
{
    switch (m_selectedItem)
    {
        case MainMenuItem::Start:
            m_selectedItem = MainMenuItem::Exit;
            break;

        case MainMenuItem::Volume:
            m_selectedItem = MainMenuItem::Start;
            break;

        case MainMenuItem::Exit:
            m_selectedItem = MainMenuItem::Volume;
            break;
    }
}

void MainMenu::selectNext()
{
    switch (m_selectedItem)
    {
        case MainMenuItem::Start:
            m_selectedItem = MainMenuItem::Volume;
            break;

        case MainMenuItem::Volume:
            m_selectedItem = MainMenuItem::Exit;
            break;

        case MainMenuItem::Exit:
            m_selectedItem = MainMenuItem::Start;
            break;
    }
}

MainMenuItem MainMenu::selectedItem() const
{
    return m_selectedItem;
}

void MainMenu::reset()
{
    m_selectedItem = MainMenuItem::Start;
}

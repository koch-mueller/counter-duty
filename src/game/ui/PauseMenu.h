#pragma once

#include <cstddef>

// Ergebnis einer bestätigten Aktion im Pausenmenü
enum class PauseMenuAction
{
    None,
    Resume,
    MainMenu,
    Quit
};

// Verwaltet Zustand, Navigation und Aktionen des Pausenmenüs
class PauseMenu
{
public:
    PauseMenu();

    void open();

    void close();

    bool isOpen() const;

    void update(bool moveUpPressed, bool moveDownPressed, bool confirmPressed);

    std::size_t selectedIndex() const;

    bool isVolumeSelected() const;

    PauseMenuAction consumeAction();

private:
    bool m_open;

    std::size_t m_selectedIndex;

    PauseMenuAction m_pendingAction;
};

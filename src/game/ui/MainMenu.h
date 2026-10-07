#pragma once

// Auswählbare Einträge des Hauptmenüs
enum class MainMenuItem
{
    Start,
    Volume,
    Exit
};

// Verwaltet Auswahl und Navigation im Hauptmenü
class MainMenu
{
public:
    MainMenu();

    void selectPrevious();
    void selectNext();

    MainMenuItem selectedItem() const;

    void reset();

private:
    MainMenuItem m_selectedItem;
};

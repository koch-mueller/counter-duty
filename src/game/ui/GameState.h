#pragma once

// Zentraler Zustand für Hauptmenü, laufendes Spiel und Pause
// Oberster Anwendungszustand fuer Intro, Hauptmenue, Gameplay und Pause
enum class GameState
{
    Intro,
    MainMenu,
    Playing,
    Paused
};

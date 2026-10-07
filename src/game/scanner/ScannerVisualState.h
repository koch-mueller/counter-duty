#pragma once

// Sichtbarer Zustand des Scanners fuer neutrales, gueltiges und fehlerhaftes Feedback
enum class ScannerVisualState
{
    Idle,
    Ready,
    Success,
    Error
};

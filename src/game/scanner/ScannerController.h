#pragma once

#include "./ScanResult.h"
#include "./ScannerStation.h"

#include <string>

class AudioPlayer;
class OrderManager;
class Product;
class ToastManager;

// Koordiniert Scanversuch, Auftragsfortschritt, Scannerfeedback, Audio und Toasts
class ScannerController
{
public:
    ScannerController(ScannerStation& scannerStation,
                      OrderManager& orderManager,
                      AudioPlayer& audioPlayer,
                      ToastManager& toastManager,
                      const std::string& scanSoundPath);

    void update(Product* heldProduct, float deltaTime);
    
    void requestScan(Product* heldProduct);
    
    bool isReady() const;

    const std::string& hintText() const;

private:
    ScanCheckResult determineResult(const Product* heldProduct) const;

    void tryScan(Product* heldProduct);

    void showFeedback(const std::string& text, ScannerVisualState visualState);

    void updateFeedbackTimer(float deltaTime);

    std::string createHintText(ScanCheckResult result) const;

    std::string createErrorText(ScanCheckResult result) const;

    void updateOutput();

    ScannerStation& m_scannerStation;
    OrderManager& m_orderManager;
    AudioPlayer& m_audioPlayer;
    ToastManager& m_toastManager;

    std::string m_scanSoundPath;

    ScanCheckResult m_currentResult;

    std::string m_hintText;
    std::string m_feedbackText;

    float m_feedbackTimeRemaining;

    ScannerVisualState m_feedbackVisualState;
};

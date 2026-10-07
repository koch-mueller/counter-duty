
#include "ScannerController.h"

#include "../audio/AudioPlayer.h"
#include "../orders/OrderManager.h"
#include "../products/Product.h"
#include "../ui/ToastManager.h"

#include <iostream>

namespace
{
    constexpr float kFeedbackDuration = 1.0f;
}

ScannerController::ScannerController(ScannerStation& scannerStation,
                                     OrderManager& orderManager,
                                     AudioPlayer& audioPlayer,
                                     ToastManager& toastManager,
                                     const std::string& scanSoundPath)
    : m_scannerStation(scannerStation),
      m_orderManager(orderManager),
      m_audioPlayer(audioPlayer),
      m_toastManager(toastManager),
      m_scanSoundPath(scanSoundPath),
      m_currentResult(ScanCheckResult::NoHeldProduct),
      m_feedbackTimeRemaining(0.0f),
      m_feedbackVisualState(ScannerVisualState::Idle)
{
}

void ScannerController::update(Product* heldProduct, float deltaTime)
{
    updateFeedbackTimer(deltaTime);

    m_currentResult = determineResult(heldProduct);

    updateOutput();
}

void ScannerController::requestScan(Product* heldProduct)
{
    if (heldProduct == nullptr)
    {
        return;
    }

    m_currentResult = determineResult(heldProduct);

    tryScan(heldProduct);

    m_currentResult = determineResult(heldProduct);

    updateOutput();
}

bool ScannerController::isReady() const
{
    return m_currentResult == ScanCheckResult::Ready;
}

const std::string& ScannerController::hintText() const
{
    return m_hintText;
}

ScanCheckResult ScannerController::determineResult(const Product* heldProduct) const
{
    if (heldProduct == nullptr)
    {
        return ScanCheckResult::NoHeldProduct;
    }

    if (!m_orderManager.hasActiveOrder())
    {
        return ScanCheckResult::NoActiveOrder;
    }

    // Die Prüfungen sind bewusst geordnet: erst Auftrag/Produktstatus, danach die räumlichen Scannerbedingungen
    if (heldProduct->isScanned())
    {
        return ScanCheckResult::AlreadyScanned;
    }

    ProductType productType = heldProduct->definition().type();

    if (!m_orderManager.isProductStillRequired(productType))
    {
        return ScanCheckResult::ProductNotRequired;
    }

    if (!m_scannerStation.containsBarcode(heldProduct->barcodeWorldPosition()))
    {
        return ScanCheckResult::BarcodeOutsideZone;
    }

    if (!m_scannerStation.isBarcodeAngleValid(heldProduct->barcodeWorldNormal()))
    {
        return ScanCheckResult::WrongBarcodeAngle;
    }

    return ScanCheckResult::Ready;
}

void ScannerController::tryScan(Product* heldProduct)
{
    if (heldProduct == nullptr)
    {
        return;
    }

    if (m_currentResult != ScanCheckResult::Ready)
    {
        std::string errorText = createErrorText(m_currentResult);

        if (!errorText.empty())
        {
            showFeedback(errorText, ScannerVisualState::Error);
        }

        return;
    }

    const Order* activeOrder = m_orderManager.activeOrder();

    if (activeOrder == nullptr)
    {
        return;
    }

    ProductType productType = heldProduct->definition().type();

    // Erst wenn der OrderManager den Scan akzeptiert, wird das konkrete Produkt dauerhaft als gescannt markiert
    bool scanRegistered = m_orderManager.registerScannedProduct(productType);

    if (!scanRegistered)
    {
        showFeedback("Produkt konnte nicht gescannt werden", ScannerVisualState::Error);

        return;
    }

    heldProduct->markScanned(activeOrder->id());

    showFeedback("Produkt erfolgreich gescannt", ScannerVisualState::Success);

    if (!m_audioPlayer.playSound(m_scanSoundPath))
    {
        std::cout << "Scanner-Sound konnte nicht "
                  << "abgespielt werden" << std::endl;
    }
}

void ScannerController::showFeedback(const std::string& text,
                                     ScannerVisualState visualState)
{
    m_feedbackText = text;

    m_feedbackTimeRemaining = kFeedbackDuration;

    m_feedbackVisualState = visualState;

    ToastType toastType = ToastType::Error;

    if (visualState == ScannerVisualState::Success)
    {
        toastType = ToastType::Success;
    }

    m_toastManager.push(text, toastType, 2.0f);
}

void ScannerController::updateFeedbackTimer(float deltaTime)
{
    if (m_feedbackTimeRemaining <= 0.0f || deltaTime <= 0.0f)
    {
        return;
    }

    m_feedbackTimeRemaining -= deltaTime;

    if (m_feedbackTimeRemaining > 0.0f)
    {
        return;
    }

    m_feedbackTimeRemaining = 0.0f;

    m_feedbackText.clear();
}

std::string ScannerController::createHintText(ScanCheckResult result) const
{
    switch (result)
    {
        case ScanCheckResult::NoHeldProduct:
            return "";

        case ScanCheckResult::NoActiveOrder:
            return "Es gibt keine aktive Bestellung";

        case ScanCheckResult::AlreadyScanned:
            return "Dieses Produkt wurde bereits gescannt";

        case ScanCheckResult::ProductNotRequired:
            return "Dieses Produkt wird nicht benoetigt";

        case ScanCheckResult::BarcodeOutsideZone:
            return "Der Barcode ist nicht in der Scannerzone";

        case ScanCheckResult::WrongBarcodeAngle:
            return "Sie halten das Produkt nicht im richtigen Winkel";

        case ScanCheckResult::Ready:
            return "";
    }

    return "";
}

std::string ScannerController::createErrorText(ScanCheckResult result) const
{
    switch (result)
    {
        case ScanCheckResult::NoActiveOrder:
            return "Keine aktive Bestellung";

        case ScanCheckResult::AlreadyScanned:
            return "Produkt bereits gescannt";

        case ScanCheckResult::ProductNotRequired:
            return "Produkt wird nicht benoetigt";

        case ScanCheckResult::BarcodeOutsideZone:
            return "Barcode nicht im Scannerbereich";

        case ScanCheckResult::WrongBarcodeAngle:
            return "Barcode falsch ausgerichtet";

        case ScanCheckResult::NoHeldProduct:
        case ScanCheckResult::Ready:
            return "";
    }

    return "";
}

void ScannerController::updateOutput()
{
    // Kurzzeitiges Erfolgs-/Fehlerfeedback hat Vorrang vor den normalen Ausrichtungshinweisen
    if (m_feedbackTimeRemaining > 0.0f)
    {
        m_hintText.clear();

        m_scannerStation.setVisualState(m_feedbackVisualState);

        return;
    }

    m_hintText = createHintText(m_currentResult);

    if (m_currentResult == ScanCheckResult::Ready)
    {
        m_scannerStation.setVisualState(ScannerVisualState::Ready);

        return;
    }

    m_scannerStation.setVisualState(ScannerVisualState::Idle);
}

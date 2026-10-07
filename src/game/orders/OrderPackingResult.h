#pragma once

#include "../products/ProductDefinition.h"

#include <vector>

// Auswertung einer einzelnen Auftragsposition
struct OrderLinePackingStatus
{
    int requiredAmount = 0;
    int packedAmount = 0;
    int scannedOutsideAmount = 0;
    int notScannedAmount = 0;
};

// Art eines Problems, das das Versiegeln des Kartons verhindert
enum class BoxWarningType
{
    Unscanned,
    WrongProduct,
    ExcessProduct
};

// Beschreibt ein konkretes Problem mit dem aktuellen Kartoninhalt
struct BoxWarning
{
    BoxWarningType type = BoxWarningType::Unscanned;

    const ProductDefinition* definition = nullptr;

    int amount = 0;
};

// Gesamtergebnis der Prüfung, ob ein Auftrag korrekt verpackt wurde
struct OrderPackingResult
{
    std::vector<OrderLinePackingStatus> lineStatuses;

    std::vector<BoxWarning> warnings;

    bool isComplete = false;
};

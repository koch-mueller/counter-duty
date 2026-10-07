#pragma once

// Ergebnis der Scannerpruefung vor dem eigentlichen Auftragsscan
enum class ScanCheckResult
{
    NoHeldProduct,
    NoActiveOrder,
    AlreadyScanned,
    ProductNotRequired,
    BarcodeOutsideZone,
    WrongBarcodeAngle,
    Ready
};

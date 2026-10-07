#pragma once

// Gameplay-Zustand eines Produkts zwischen Regal, Tragen, Physik und versiegeltem Karton
enum class ProductState
{
    OnShelf,
    Held,
    Dynamic,
    Sleeping,
    InSealedBox,
    Delivered
};

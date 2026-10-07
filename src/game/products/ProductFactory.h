#pragma once

#include "ProductDefinition.h"

#include <memory>

class Product;
class ProductCatalog;

// Erzeugt Produktinstanzen aus den Definitionen des Produktkatalogs
class ProductFactory
{
public:
    explicit ProductFactory(const ProductCatalog& catalog);

    std::unique_ptr<Product> create(ProductType type, const Vector& position) const;

private:
    const ProductCatalog& m_catalog;
};

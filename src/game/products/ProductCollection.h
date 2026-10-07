#pragma once

#include "Product.h"
#include "ProductFactory.h"

#include <memory>
#include <unordered_set>
#include <vector>

// Besitzt die zur Laufzeit erzeugten Produkte und verwaltet deren Lebensdauer
class ProductCollection
{
public:
    explicit ProductCollection(const ProductCatalog& catalog);

    Product* create(ProductType type, const Vector& position = Vector());

    void remove(Product* product);

    void remove(const std::unordered_set<Product*>& products);

    void clear();

    const std::vector<Product*>& products() const;

private:
    void removeProductPointers(const std::unordered_set<Product*>& products);

    void removeOwnedProducts(const std::unordered_set<Product*>& products);

    ProductFactory m_factory;

    std::vector<std::unique_ptr<Product>> m_ownedProducts;

    // Diese Liste ist nur eine nicht-besitzende Ansicht für Systeme, die rohe Zeiger benötigen
    std::vector<Product*> m_products;
};

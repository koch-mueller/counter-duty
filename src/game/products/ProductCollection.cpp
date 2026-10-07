#include "ProductCollection.h"

ProductCollection::ProductCollection(const ProductCatalog& catalog)
    : m_factory(catalog)
{
}

Product* ProductCollection::create(ProductType type, const Vector& position)
{
    std::unique_ptr<Product> product = m_factory.create(type, position);

    if (product == nullptr)
    {
        return nullptr;
    }

    // unique_ptr besitzt das Produkt, die zweite Liste enthält nur stabile, nicht-besitzende Zeiger für die Gameplay-Systeme
    Product* productPointer = product.get();

    m_ownedProducts.push_back(std::move(product));

    m_products.push_back(productPointer);

    return productPointer;
}

void ProductCollection::remove(Product* product)
{
    if (product == nullptr)
    {
        return;
    }

    std::unordered_set<Product*> productsToRemove;

    productsToRemove.insert(product);

    remove(productsToRemove);
}

void ProductCollection::remove(const std::unordered_set<Product*>& products)
{
    if (products.empty())
    {
        return;
    }

    // Zuerst werden die nicht-besitzenden Zeiger entfernt, danach die eigentlichen Objekte
    removeProductPointers(products);

    removeOwnedProducts(products);
}

void ProductCollection::clear()
{
    m_products.clear();

    m_ownedProducts.clear();
}

const std::vector<Product*>& ProductCollection::products() const
{
    return m_products;
}

void ProductCollection::removeProductPointers(const std::unordered_set<Product*>& products)
{
    auto productIterator = m_products.begin();

    while (productIterator != m_products.end())
    {
        Product* product = *productIterator;

        bool shouldRemove = products.find(product) != products.end();

        if (!shouldRemove)
        {
            ++productIterator;
            continue;
        }

        productIterator = m_products.erase(productIterator);
    }
}

void ProductCollection::removeOwnedProducts(const std::unordered_set<Product*>& products)
{
    auto productIterator = m_ownedProducts.begin();

    while (productIterator != m_ownedProducts.end())
    {
        Product* product = productIterator->get();

        bool shouldRemove = products.find(product) != products.end();

        if (!shouldRemove)
        {
            ++productIterator;
            continue;
        }

        productIterator = m_ownedProducts.erase(productIterator);
    }
}

#include "ProductFactory.h"

#include "Product.h"
#include "ProductCatalog.h"

ProductFactory::ProductFactory(const ProductCatalog& catalog)
    : m_catalog(catalog)
{
}

std::unique_ptr<Product> ProductFactory::create(ProductType type, const Vector& position) const
{
    const ProductDefinition* definition = m_catalog.find(type);

    if (definition == nullptr)
    {
        return nullptr;
    }

    return std::make_unique<Product>(*definition, position);
}

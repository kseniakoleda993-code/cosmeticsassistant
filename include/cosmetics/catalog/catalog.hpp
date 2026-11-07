#pragma once

#include "cosmetics/catalog/budget.hpp"
#include "cosmetics/catalog/position.hpp"
#include "cosmetics/catalog/product.hpp"

namespace cosmetics {

struct Offer {
    Product product;
    std::string photo;
};

class Catalog {
public:
    virtual ~Catalog() = default;
    virtual Result<Offer> find(Position position, Budget budget) const = 0;
};

class SampleCatalog final : public Catalog {
public:
    Result<Offer> find(Position position, Budget budget) const override;
};

}

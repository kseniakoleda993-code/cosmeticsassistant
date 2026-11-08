#pragma once

#include "cosmetics/catalog/product.hpp"

#include <vector>

namespace cosmetics {

class Cart {
public:
    Cart add(const Product& product) const;
    std::int64_t total() const;
    const std::vector<Product>& items() const { return products; }

private:
    std::vector<Product> products;
};

}

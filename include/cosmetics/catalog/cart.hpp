#pragma once

#include "cosmetics/catalog/product.hpp"

#include <vector>

namespace cosmetics {

class Cart {
public:
    Cart add(const Product& product) const;
    Cart remove(std::size_t index) const;
    Cart clear() const;
    std::int64_t revision() const { return version; }
    std::int64_t total() const;
    const std::vector<Product>& items() const { return products; }

private:
    std::vector<Product> products;
    std::int64_t version = 0;
    Cart next_version() const;
};

}

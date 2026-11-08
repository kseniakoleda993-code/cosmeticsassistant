#include "cosmetics/catalog/cart.hpp"

#include <limits>
#include <numeric>
#include <stdexcept>

namespace cosmetics {

Cart Cart::add(const Product& product) const {
    Cart copy = *this;
    copy.products.push_back(product);
    return copy;
}

std::int64_t Cart::total() const {
    return std::accumulate(products.begin(), products.end(), std::int64_t{0}, [](std::int64_t sum, const Product& product) {
        if (product.price() > std::numeric_limits<std::int64_t>::max() - sum) {
            throw std::overflow_error("Сумма корзины слишком большая");
        }
        return sum + product.price();
    });
}

}

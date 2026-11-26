#include "cosmetics/catalog/cart.hpp"

#include <limits>
#include <numeric>
#include <stdexcept>

namespace cosmetics {

Cart Cart::next_version() const {
    if (version == std::numeric_limits<std::int64_t>::max()) {
        throw std::overflow_error("Слишком много изменений корзины");
    }
    Cart copy = *this;
    ++copy.version;
    return copy;
}

Cart Cart::add(const Product& product) const {
    Cart copy = next_version();
    copy.products.push_back(product);
    return copy;
}

Cart Cart::remove(std::size_t index) const {
    if (index >= products.size()) {
        throw std::out_of_range("Нет такой позиции в корзине");
    }
    Cart copy = next_version();
    copy.products.erase(copy.products.begin() + index);
    return copy;
}

Cart Cart::clear() const {
    if (products.empty()) {
        return *this;
    }
    Cart copy = next_version();
    copy.products.clear();
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

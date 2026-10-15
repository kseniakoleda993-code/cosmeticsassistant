#include "cosmetics/catalog/product.hpp"

#include <stdexcept>
#include <utility>

namespace cosmetics {

std::string format_price(std::int64_t price) {
    if (price < 0) {
        throw std::invalid_argument("Цена не может быть отрицательной");
    }
    auto whole = std::to_string(price / 100);
    auto fraction = std::to_string(price % 100);
    if (fraction.size() == 1) {
        fraction = "0" + fraction;
    }
    return whole + "." + fraction;
}

Product::Product(std::string name, std::int64_t price) : name_(std::move(name)), price_(price) {
    if (name_.empty()) {
        throw std::invalid_argument("Название товара не может быть пустым");
    }
    if (price_ < 0) {
        throw std::invalid_argument("Цена не может быть отрицательной");
    }
}

const std::string& Product::name() const noexcept {
    return name_;
}

std::int64_t Product::price() const noexcept {
    return price_;
}

}

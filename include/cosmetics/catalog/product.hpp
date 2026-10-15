#pragma once

#include <cstdint>
#include <string>

namespace cosmetics {

std::string format_price(std::int64_t price);

class Product {
public:
    Product(std::string name, std::int64_t price);
    const std::string& name() const noexcept;
    std::int64_t price() const noexcept;

private:
    std::string name_;
    std::int64_t price_;
};

}

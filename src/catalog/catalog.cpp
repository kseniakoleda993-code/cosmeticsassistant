#include "cosmetics/catalog/catalog.hpp"

#include <algorithm>

namespace cosmetics {

Result<Offer> SampleCatalog::find(Position position, Budget budget) const {
    auto it = std::find(positions.begin(), positions.end(), position);
    if (it == positions.end() || (budget != Budget::Average && budget != Budget::AboveAverage)) {
        return Result<Offer>::failure("Неизвестная позиция или бюджет");
    }
    std::string name(to_text(position));
    bool expensive = budget == Budget::AboveAverage;
    name += expensive ? " — вариант подороже" : " — средний бюджет";
    std::int64_t price = (it - positions.begin() + 1) * 100 + (expensive ? 2000 : 1000);
    return Result<Offer>::success({Product(name, price), {}});
}

}

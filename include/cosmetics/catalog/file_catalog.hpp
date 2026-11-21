#pragma once

#include "cosmetics/catalog/catalog.hpp"

#include <filesystem>
#include <map>

namespace cosmetics {

class FileCatalog final : public Catalog {
public:
    static Result<FileCatalog> load(const std::filesystem::path& file);
    Result<Offer> find(Position position, Budget budget) const override;

private:
    std::map<std::pair<Position, Budget>, Offer> offers;
};

}

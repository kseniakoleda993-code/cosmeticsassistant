#include "cosmetics/catalog/category_positions.hpp"

#include <stdexcept>

namespace cosmetics {

const std::vector<Position>& positions_for(Category category) {
    static const std::vector<Position> hair{
        Position::Shampoo, Position::Conditioner, Position::HairMask,
        Position::HairOil, Position::HeatProtection, Position::DryShampoo
    };
    static const std::vector<Position> body{
        Position::ShowerGel, Position::BodyScrub, Position::BodyCream
    };
    static const std::vector<Position> skin{
        Position::FaceScrub, Position::FaceWash, Position::Toner, Position::EnzymePowder
    };
    static const std::vector<Position> makeup{
        Position::Foundation, Position::Blush, Position::Mascara,
        Position::Lipstick, Position::BrowGel, Position::Concealer
    };
    switch (category) {
        case Category::Hair: return hair;
        case Category::Body: return body;
        case Category::Skin: return skin;
        case Category::Makeup: return makeup;
    }
    throw std::invalid_argument("Неизвестный раздел каталога");
}

}

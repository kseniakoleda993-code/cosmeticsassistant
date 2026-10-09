#pragma once

#include "cosmetics/core/result.hpp"

#include <array>
#include <string_view>

namespace cosmetics {

enum class Position {
    Shampoo, Conditioner, HairMask, HairOil, HeatProtection, DryShampoo,
    ShowerGel, BodyScrub, BodyCream,
    FaceScrub, FaceWash, Toner, EnzymePowder,
    Foundation, Blush, Mascara, Lipstick, BrowGel, Concealer
};

inline constexpr std::array<Position, 19> positions{
    Position::Shampoo, Position::Conditioner, Position::HairMask, Position::HairOil,
    Position::HeatProtection, Position::DryShampoo,
    Position::ShowerGel, Position::BodyScrub, Position::BodyCream,
    Position::FaceScrub, Position::FaceWash, Position::Toner, Position::EnzymePowder,
    Position::Foundation, Position::Blush, Position::Mascara, Position::Lipstick,
    Position::BrowGel, Position::Concealer
};

std::string_view to_text(Position position);
Result<Position> position_from_text(std::string_view text);

}

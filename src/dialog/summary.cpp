#include "cosmetics/dialog/summary.hpp"

namespace cosmetics {

std::string cart_summary(const Cart& cart) {
    return "Отлично! Суммарная стоимость продуктов:\n" + format_price(cart.total()) + " BYN\nБыл рад помочь! Если есть какие-либо вопросы, задавай";
}

}

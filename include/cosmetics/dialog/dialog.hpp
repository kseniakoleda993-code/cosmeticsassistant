#pragma once

#include "cosmetics/dialog/step.hpp"

namespace cosmetics {

class Dialog {
public:
    explicit Dialog(const Catalog& catalog);
    const Step& step(Stage stage) const;
    Transition handle(const User& user, std::string_view action) const;

private:
    std::array<std::unique_ptr<Step>, 6> steps;
};

}

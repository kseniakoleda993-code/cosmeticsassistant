#pragma once

#include "cosmetics/core/result.hpp"
#include "cosmetics/json/value.hpp"

namespace cosmetics::json {

template <typename T>
Result<const T*> field(const Object& object, const std::string& key) {
    const auto* value = object.find(key);
    if (!value) {
        return Result<const T*>::failure("Нет поля JSON: " + key);
    }
    const auto* typed = dynamic_cast<const T*>(value);
    if (!typed) {
        return Result<const T*>::failure("Неверный тип поля JSON: " + key);
    }
    return Result<const T*>::success(typed);
}

}

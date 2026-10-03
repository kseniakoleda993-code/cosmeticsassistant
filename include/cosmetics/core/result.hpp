#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace cosmetics {

template <typename T>
class Result {
public:
    static Result success(T value) {
        return Result(std::move(value));
    }

    static Result failure(std::string error) {
        return Result(Error{std::move(error)});
    }

    bool has_value() const noexcept {
        return data_.index() == 0;
    }

    explicit operator bool() const noexcept {
        return has_value();
    }

    T& value() {
        check_value();
        return std::get<0>(data_);
    }

    const T& value() const {
        check_value();
        return std::get<0>(data_);
    }

    const std::string& error() const {
        if (has_value()) {
            throw std::logic_error("У успешного результата нет ошибки");
        }
        return std::get<1>(data_).message;
    }

private:
    struct Error {
        std::string message;
    };

    explicit Result(T value) : data_(std::in_place_index<0>, std::move(value)) {}
    explicit Result(Error error) : data_(std::in_place_index<1>, std::move(error)) {}

    void check_value() const {
        if (!has_value()) {
            throw std::logic_error("Нельзя получить значение результата с ошибкой");
        }
    }

    std::variant<T, Error> data_;
};

}

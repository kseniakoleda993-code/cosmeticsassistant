#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cosmetics::json {

enum class Type { String, Number, Boolean, Null, Array, Object };

class Value {
public:
    virtual ~Value() = default;
    virtual Type type() const noexcept = 0;
    virtual std::string text() const = 0;
};

using Ptr = std::unique_ptr<Value>;

class String final : public Value {
public:
    explicit String(std::string value) : value(std::move(value)) {}
    Type type() const noexcept override { return Type::String; }
    std::string text() const override;
    std::string value;
};

class Number final : public Value {
public:
    explicit Number(std::int64_t value) : value(value) {}
    Type type() const noexcept override { return Type::Number; }
    std::string text() const override { return std::to_string(value); }
    std::int64_t value;
};

class Boolean final : public Value {
public:
    explicit Boolean(bool value) : value(value) {}
    Type type() const noexcept override { return Type::Boolean; }
    std::string text() const override { return value ? "true" : "false"; }
    bool value;
};

class Null final : public Value {
public:
    Type type() const noexcept override { return Type::Null; }
    std::string text() const override { return "null"; }
};

class Array final : public Value {
public:
    Type type() const noexcept override { return Type::Array; }
    std::string text() const override;
    const Value* at(std::size_t index) const;
    std::vector<Ptr> values;
};

class Object final : public Value {
public:
    Type type() const noexcept override { return Type::Object; }
    std::string text() const override;
    const Value* find(const std::string& key) const;
    std::map<std::string, Ptr> fields;
};

}

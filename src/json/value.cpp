#include "cosmetics/json/value.hpp"

namespace cosmetics::json {

std::string String::text() const {
    const char* hex = "0123456789abcdef";
    std::string result = "\"";
    for (unsigned char ch : value) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 32) {
                    result += "\\u00";
                    result += hex[ch >> 4];
                    result += hex[ch & 15];
                } else {
                    result += static_cast<char>(ch);
                }
        }
    }
    return result + '"';
}

std::string Array::text() const {
    std::string result = "[";
    for (const auto& value : values) {
        if (result.size() > 1) {
            result += ',';
        }
        result += value->text();
    }
    return result + ']';
}

std::string Object::text() const {
    std::string result = "{";
    for (const auto& field : fields) {
        if (result.size() > 1) {
            result += ',';
        }
        result += String(field.first).text() + ':' + field.second->text();
    }
    return result + '}';
}

const Value* Array::at(std::size_t index) const {
    return index < values.size() ? values[index].get() : nullptr;
}

const Value* Object::find(const std::string& key) const {
    auto it = fields.find(key);
    return it == fields.end() ? nullptr : it->second.get();
}

}

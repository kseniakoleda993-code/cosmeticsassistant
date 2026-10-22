#include "cosmetics/json/parse.hpp"

#include <charconv>
#include <stdexcept>

namespace cosmetics::json {
namespace {

bool digit(char ch) {
    return ch >= '0' && ch <= '9';
}

void append_utf8(std::string& text, unsigned int code) {
    if (code < 0x80) {
        text += static_cast<char>(code);
        return;
    }
    if (code < 0x800) {
        text += static_cast<char>(0xc0 | (code >> 6));
    } else if (code < 0x10000) {
        text += static_cast<char>(0xe0 | (code >> 12));
        text += static_cast<char>(0x80 | ((code >> 6) & 63));
    } else {
        text += static_cast<char>(0xf0 | (code >> 18));
        text += static_cast<char>(0x80 | ((code >> 12) & 63));
        text += static_cast<char>(0x80 | ((code >> 6) & 63));
    }
    text += static_cast<char>(0x80 | (code & 63));
}

class Parser {
public:
    explicit Parser(std::string_view input) : input(input) {}

    Ptr read() {
        skip_space();
        Ptr result = read_value();
        skip_space();
        if (pos != input.size()) {
            fail();
        }
        return result;
    }

private:
    std::string_view input;
    std::size_t pos = 0;

    [[noreturn]] void fail() const {
        throw std::invalid_argument("Некорректный JSON, байт " + std::to_string(pos));
    }

    char peek() const {
        return pos < input.size() ? input[pos] : '\0';
    }

    bool take(char ch) {
        if (pos == input.size() || input[pos] != ch) {
            return false;
        }
        ++pos;
        return true;
    }

    void skip_space() {
        while (peek() == ' ' || peek() == '\r' || peek() == '\n' || peek() == '\t') {
            ++pos;
        }
    }

    unsigned int hex4() {
        unsigned int code = 0;
        for (int i = 0; i < 4; ++i) {
            char ch = peek();
            unsigned int value;
            if (digit(ch)) {
                value = ch - '0';
            } else if (ch >= 'a' && ch <= 'f') {
                value = ch - 'a' + 10;
            } else if (ch >= 'A' && ch <= 'F') {
                value = ch - 'A' + 10;
            } else {
                fail();
            }
            ++pos;
            code = code * 16 + value;
        }
        return code;
    }

    unsigned int read_unicode() {
        unsigned int code = hex4();
        if (code >= 0xd800 && code <= 0xdbff) {
            if (!take('\\') || !take('u')) {
                fail();
            }
            unsigned int low = hex4();
            if (low < 0xdc00 || low > 0xdfff) {
                fail();
            }
            return 0x10000 + ((code - 0xd800) << 10) + (low - 0xdc00);
        }
        if (code >= 0xdc00 && code <= 0xdfff) {
            fail();
        }
        return code;
    }

    std::string read_string() {
        if (!take('"')) {
            fail();
        }
        std::string text;
        while (pos < input.size()) {
            unsigned char ch = input[pos++];
            if (ch == '"') {
                return text;
            }
            if (ch < 32) {
                fail();
            }
            if (ch != '\\') {
                text += static_cast<char>(ch);
                continue;
            }
            if (pos == input.size()) {
                fail();
            }
            switch (input[pos++]) {
                case '"': text += '"'; break;
                case '\\': text += '\\'; break;
                case '/': text += '/'; break;
                case 'b': text += '\b'; break;
                case 'f': text += '\f'; break;
                case 'n': text += '\n'; break;
                case 'r': text += '\r'; break;
                case 't': text += '\t'; break;
                case 'u': append_utf8(text, read_unicode()); break;
                default: fail();
            }
        }
        fail();
    }

    Ptr read_number() {
        std::size_t start = pos;
        take('-');
        if (!digit(peek())) {
            fail();
        }
        if (take('0')) {
            if (digit(peek())) {
                fail();
            }
        } else {
            while (digit(peek())) {
                ++pos;
            }
        }
        std::int64_t value = 0;
        auto result = std::from_chars(input.data() + start, input.data() + pos, value);
        if (result.ec != std::errc{}) {
            fail();
        }
        return std::make_unique<Number>(value);
    }

    Ptr read_value() {
        if (peek() == '"') {
            return std::make_unique<String>(read_string());
        }
        return read_number();
    }
};

}

Result<Ptr> parse(std::string_view input) {
    try {
        return Result<Ptr>::success(Parser(input).read());
    } catch (const std::invalid_argument& error) {
        return Result<Ptr>::failure(error.what());
    }
}

}

#include "cosmetics/catalog/file_catalog.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>

namespace cosmetics {
namespace {

std::array<std::string, 5> split_row(const std::string& line) {
    std::array<std::string, 5> fields;
    std::size_t start = 0;
    for (std::size_t i = 0; i < fields.size() - 1; ++i) {
        auto end = line.find('\t', start);
        if (end == std::string::npos) {
            return {};
        }
        fields[i] = line.substr(start, end - start);
        start = end + 1;
    }
    fields.back() = line.substr(start);
    return fields;
}

bool valid_photo(const std::string& name) {
    if (name.size() <= 4 || name.substr(name.size() - 4) != ".jpg") {
        return false;
    }
    return std::all_of(name.begin(), name.end() - 4, [](char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-';
    });
}

}

Result<FileCatalog> FileCatalog::load(const std::filesystem::path& file) {
    using Outcome = Result<FileCatalog>;
    std::ifstream input(file);
    if (!input) {
        return Outcome::failure("Не удалось открыть каталог " + file.string());
    }
    FileCatalog catalog;
    std::string line;
    unsigned int line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        auto fields = split_row(line);
        auto position = position_from_text(fields[0]);
        auto budget = budget_from_text(fields[1]);
        std::int64_t price = 0;
        const auto& price_text = fields[3];
        const auto* end = price_text.data() + price_text.size();
        auto parsed = std::from_chars(price_text.data(), end, price);
        if (!position || !budget || fields[2].empty() || parsed.ec != std::errc{} || parsed.ptr != end || price <= 0 || !valid_photo(fields[4])) {
            return Outcome::failure("Неверная строка каталога: " + std::to_string(line_number));
        }
        auto photo = file.parent_path() / "photos" / fields[4];
        std::ifstream image(photo, std::ios::binary);
        unsigned char marker[3] = {};
        image.read(reinterpret_cast<char*>(marker), sizeof marker);
        if (!image || marker[0] != 0xff || marker[1] != 0xd8 || marker[2] != 0xff) {
            return Outcome::failure("Каталог ссылается на отсутствующее или неверное JPEG фото: " + fields[4]);
        }
        auto key = std::make_pair(position.value(), budget.value());
        auto entry = catalog.offers.emplace(key, Offer{Product(fields[2], price), photo.string()});
        if (!entry.second) {
            return Outcome::failure("Повтор позиции и бюджета в строке: " + std::to_string(line_number));
        }
    }
    if (!input.eof() || input.bad()) {
        return Outcome::failure("Ошибка чтения каталога");
    }
    for (auto position : positions) {
        auto cheap = catalog.find(position, Budget::Average);
        auto expensive = catalog.find(position, Budget::AboveAverage);
        if (!cheap || !expensive || cheap.value().product.price() >= expensive.value().product.price()) {
            return Outcome::failure("Нужны два предложения с возрастающей ценой для позиции: " + std::string(to_text(position)));
        }
    }
    return Outcome::success(std::move(catalog));
}

Result<Offer> FileCatalog::find(Position position, Budget budget) const {
    auto it = offers.find({position, budget});
    if (it == offers.end()) {
        return Result<Offer>::failure("В каталоге нет предложения");
    }
    return Result<Offer>::success(it->second);
}

}

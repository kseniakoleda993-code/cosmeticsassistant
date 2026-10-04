#include "cosmetics/config/token.hpp"
#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <optional>
#include <unistd.h>

namespace {

class Environment {
public:
    Environment() {
        if (const auto* old = std::getenv("TELEGRAM_BOT_TOKEN")) {
            old_ = old;
        }
        set(nullptr);
    }

    ~Environment() {
        if (old_) {
            setenv("TELEGRAM_BOT_TOKEN", old_->c_str(), 1);
        } else {
            unsetenv("TELEGRAM_BOT_TOKEN");
        }
    }

    void set(const char* value) {
        const int result = value ? setenv("TELEGRAM_BOT_TOKEN", value, 1) : unsetenv("TELEGRAM_BOT_TOKEN");
        if (result != 0) {
            throw std::runtime_error("Не удалось изменить окружение теста");
        }
    }

private:
    std::optional<std::string> old_;
};

class TokenFile {
public:
    explicit TokenFile(const std::string& contents) {
        std::ofstream out(path(), std::ios::binary);
        out << contents;
        out.close();
        if (!out) {
            throw std::runtime_error("Не удалось записать настройки теста");
        }
    }

    ~TokenFile() {
        std::error_code error;
        std::filesystem::remove(path(), error);
    }

    static std::filesystem::path path() {
        return std::filesystem::temp_directory_path() / ("cosmetics-token-" + std::to_string(getpid()) + ".txt");
    }
};

}

TEST(Token, EnvironmentPrecedesFile) {
    Environment env;
    env.set("123:environment-secret");
    TokenFile file("456:file-secret\n");
    auto result = cosmetics::load_bot_token(file.path());
    ASSERT_TRUE(result && result.value() == "123:environment-secret");
}

TEST(Token, FileLineEndings) {
    Environment env;
    for (const auto& suffix : {"", "\n", "\r\n"}) {
        TokenFile file(std::string("456:file-secret") + suffix);
        auto result = cosmetics::load_bot_token(file.path());
        ASSERT_TRUE(result && result.value() == "456:file-secret");
    }
}

TEST(Token, EmptyEnvironmentFallsBack) {
    Environment env;
    env.set("");
    TokenFile file("456:file-secret");
    ASSERT_TRUE(cosmetics::load_bot_token(file.path()));
}

TEST(Token, MissingSources) {
    Environment env;
    auto result = cosmetics::load_bot_token("nonexistent-test-directory/token");
    ASSERT_TRUE(!result);
    ASSERT_TRUE(!result.error().empty());
}

TEST(Token, InvalidTokenDoesNotLeak) {
    Environment env;
    const std::string secret = "secret-token with-space";
    env.set(secret.c_str());
    auto result = cosmetics::load_bot_token();
    ASSERT_TRUE(!result);
    ASSERT_TRUE(result.error().find(secret) == std::string::npos);
    env.set(nullptr);
    for (const auto& contents : {"", "\n", "secret-token with-space", "secret-token\nextra"}) {
        TokenFile file(contents);
        auto invalid = cosmetics::load_bot_token(file.path());
        ASSERT_TRUE(!invalid);
        ASSERT_TRUE(invalid.error().find("secret-token") == std::string::npos);
    }
}

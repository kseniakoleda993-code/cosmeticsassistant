cmake_minimum_required(VERSION 3.16)
project(cosmeticsassistant VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(COSMETICS_COVERAGE "Измерять покрытие кода Clang" OFF)
if(COSMETICS_COVERAGE AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Для покрытия нужен Clang")
endif()

function(cosmetics_coverage target)
    if(COSMETICS_COVERAGE)
        target_compile_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
        target_link_options(${target} PRIVATE -fprofile-instr-generate)
    endif()
endfunction()

find_package(CURL 7.56 REQUIRED)

add_library(cosmetics STATIC
    src/config/token.cpp
    src/transport/fake_transport.cpp
    src/transport/https_transport.cpp
    src/http/response.cpp
    src/catalog/category.cpp
    src/catalog/position.cpp
    src/catalog/category_positions.cpp
    src/catalog/budget.cpp
    src/catalog/product.cpp
    src/catalog/catalog.cpp
    src/catalog/file_catalog.cpp
    src/catalog/cart.cpp
    src/dialog/summary.cpp
    src/dialog/step.cpp
    src/dialog/dialog.cpp
    src/json/value.cpp
    src/json/parse.cpp
    src/telegram/update.cpp
    src/telegram/request.cpp
    src/telegram/client.cpp
    src/telegram/bot.cpp
)
cosmetics_coverage(cosmetics)
target_link_libraries(cosmetics PUBLIC CURL::libcurl)
target_include_directories(cosmetics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(cosmetics PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(cosmetics_bot src/main.cpp)
target_link_libraries(cosmetics_bot PRIVATE cosmetics)
cosmetics_coverage(cosmetics_bot)

add_executable(telegram_smoke EXCLUDE_FROM_ALL tests/telegram_smoke.cpp)
target_link_libraries(telegram_smoke PRIVATE cosmetics)
cosmetics_coverage(telegram_smoke)

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(googletest
        URL https://codeload.github.com/google/googletest/tar.gz/refs/tags/v1.17.0
        URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
    )
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
    add_executable(cosmetics_tests
        tests/framework_tests.cpp
        tests/result_tests.cpp
        tests/token_tests.cpp
        tests/transport_tests.cpp
        tests/http_tests.cpp
        tests/category_tests.cpp
        tests/position_tests.cpp
        tests/category_positions_tests.cpp
        tests/budget_tests.cpp
        tests/product_tests.cpp
        tests/catalog_tests.cpp
        tests/cart_tests.cpp
        tests/user_tests.cpp
        tests/dialog_tests.cpp
        tests/store_tests.cpp
        tests/bot_tests.cpp
        tests/json_tests.cpp
        tests/telegram_tests.cpp
    )
    target_compile_definitions(cosmetics_tests PRIVATE COSMETICS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
    target_link_libraries(cosmetics_tests PRIVATE cosmetics GTest::gtest_main)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(cosmetics_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    cosmetics_coverage(cosmetics_tests)
    gtest_discover_tests(cosmetics_tests)
    add_test(NAME bot_missing_token
        COMMAND ${CMAKE_COMMAND} -DBOT=$<TARGET_FILE:cosmetics_bot> -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_startup.cmake
    )
    add_executable(framework_failure_probe tests/framework_failure_probe.cpp)
    target_link_libraries(framework_failure_probe PRIVATE GTest::gtest_main)
    add_test(NAME framework_failure_exit
        COMMAND ${CMAKE_COMMAND} -DPROBE=$<TARGET_FILE:framework_failure_probe>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_failure.cmake
    )
endif()

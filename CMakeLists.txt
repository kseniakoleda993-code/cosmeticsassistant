cmake_minimum_required(VERSION 3.16)
project(cosmeticsassistant VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cosmetics STATIC
    src/config/token.cpp
    src/transport/fake_transport.cpp
    src/http/response.cpp
    src/catalog/category.cpp
    src/catalog/position.cpp
)
target_include_directories(cosmetics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(cosmetics PRIVATE -Wall -Wextra -Wpedantic)
endif()

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
    function(add_cosmetics_test name)
        add_executable(${name} tests/${name}.cpp)
        target_link_libraries(${name} PRIVATE cosmetics GTest::gtest_main)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(${name} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        gtest_discover_tests(${name})
    endfunction()

    add_cosmetics_test(framework_tests)
    add_cosmetics_test(result_tests)
    add_cosmetics_test(token_tests)
    add_cosmetics_test(transport_tests)
    add_cosmetics_test(http_tests)
    add_cosmetics_test(category_tests)
    add_cosmetics_test(position_tests)
    add_executable(framework_failure_probe tests/framework_failure_probe.cpp)
    target_link_libraries(framework_failure_probe PRIVATE GTest::gtest_main)
    add_test(NAME framework_failure_exit
        COMMAND ${CMAKE_COMMAND} -DPROBE=$<TARGET_FILE:framework_failure_probe>
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_failure.cmake
    )
endif()

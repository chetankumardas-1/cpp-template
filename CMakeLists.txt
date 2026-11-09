
cmake_minimum_required(VERSION 3.20)

project(CppTemplate
    VERSION 1.0
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Thread support
find_package(Threads REQUIRED)

# Main application
add_executable(app
    src/main.cpp
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE
    Threads::Threads
)

# Compiler warnings
target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
)

# Enable sanitizers in Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug"
   AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

    target_compile_options(app PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )

    target_link_options(app PRIVATE
        -fsanitize=address,undefined
    )

endif()

# Unit testing
include(CTest)

if(BUILD_TESTING)

    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    # Test executable
    add_executable(unit_tests
        tests/calculator_test.cpp
        tests/lru_cache_test.cpp
    )

    target_include_directories(unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    target_link_libraries(unit_tests PRIVATE
        GTest::gtest_main
        Threads::Threads
    )

    # Compiler warnings for tests
    target_compile_options(unit_tests PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
    )

    # Sanitizers for tests
    if(CMAKE_BUILD_TYPE STREQUAL "Debug"
       AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")

        target_compile_options(unit_tests PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )

        target_link_options(unit_tests PRIVATE
            -fsanitize=address,undefined
        )

    endif()

    # Register GoogleTest cases with CTest
    include(GoogleTest)
    gtest_discover_tests(unit_tests)

endif()

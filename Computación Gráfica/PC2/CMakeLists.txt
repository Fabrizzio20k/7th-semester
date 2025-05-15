cmake_minimum_required(VERSION 3.14)
project(mi_proyecto)

# Requiere C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Descargar GoogleTest con FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Ejecutable de test
add_executable(tests
    exercise08/solution.cpp
)

# Enlace con gtest y tu código
target_link_libraries(tests
    GTest::gtest_main
)

# Configurar tests
include(GoogleTest)
enable_testing()
gtest_discover_tests(tests)

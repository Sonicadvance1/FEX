cmake_minimum_required(VERSION 3.10)
project(FEX)

option(BUILD_TESTS "Build unit tests to ensure sanity" TRUE)
option(ENABLE_CLANG_FORMAT "Run clang format over the source" FALSE)
option(ENABLE_LTO "Enable LTO with compilation" TRUE)
option(ENABLE_XRAY "Enable building with LLVM X-Ray" FALSE)
option(ENABLE_LLD "Enable linking with LLD" FALSE)
option(ENABLE_ASAN "Enables Clang ASAN" FALSE)
option(ENABLE_TSAN "Enables Clang TSAN" FALSE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (ENABLE_LTO)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

if (ENABLE_XRAY)
  add_compile_options(-fxray-instrument)
  link_libraries(-fxray-instrument)
endif()

if (ENABLE_LLD)
  link_libraries(-fuse-ld=lld)
endif()

if (ENABLE_ASAN)
  add_compile_options(-fno-omit-frame-pointer -fsanitize=address)
  link_libraries(-fno-omit-frame-pointer -fsanitize=address)
endif()

if (ENABLE_TSAN)
  add_compile_options(-fno-omit-frame-pointer -fsanitize=thread)
  link_libraries(-fno-omit-frame-pointer -fsanitize=thread)
endif()

set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer")
set (CMAKE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_LINKER_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer")

set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
set (CMAKE_LINKER_FLAGS_RELEASE "${CMAKE_LINKER_FLAGS_RELEASE} -fomit-frame-pointer")

add_definitions(-Wno-trigraphs)

add_subdirectory(External/SonicUtils/)
include_directories(External/SonicUtils/include/SonicUtils)

add_subdirectory(External/cpp-optparse/)
include_directories(External/cpp-optparse/)

add_subdirectory(External/imgui/)
include_directories(External/imgui/)

add_subdirectory(External/json-maker/)
include_directories(External/json-maker/)

add_subdirectory(External/tiny-json/)
include_directories(External/tiny-json/)

include_directories(External/xbyak/)

include_directories(Source/)
include_directories("${CMAKE_BINARY_DIR}/Source/")

add_subdirectory(External/FEXCore)

find_package(LLVM CONFIG QUIET)
if(LLVM_FOUND AND TARGET LLVM)
  message(STATUS "LLVM found!")
  include_directories(${LLVM_INCLUDE_DIRS})
endif()

include(CheckCXXCompilerFlag)

# Add in diagnostic colours if the option is available.
# Ninja code generator will kill colours if this isn't here
check_cxx_compiler_flag(-fdiagnostics-color=always GCC_COLOR)
check_cxx_compiler_flag(-fcolor-diagnostics CLANG_COLOR)

if (GCC_COLOR)
  add_compile_options(-fdiagnostics-color=always)
endif()
if (CLANG_COLOR)
  add_compile_options(-fcolor-diagnostics)
endif()

check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

add_compile_options(-Wall)

add_subdirectory(Source/)

if (BUILD_TESTS)
  enable_testing()
  message(STATUS "Unit tests are enabled")
  add_subdirectory(unittests/)
endif()

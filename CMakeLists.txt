cmake_minimum_required(VERSION 3.16)
project(ELFSecurityAnalyzer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(elf-analyzer
    src/main.cpp
    src/elf_parser.cpp
    src/elf_report.cpp
)

target_compile_options(elf-analyzer PRIVATE -Wall -Wextra)

option(ENABLE_SANITIZERS "Enable ASan and UBSan" OFF)
if(ENABLE_SANITIZERS)
    target_compile_options(elf-analyzer PRIVATE -fsanitize=address,undefined)
    target_link_options(elf-analyzer PRIVATE -fsanitize=address,undefined)
endif()

enable_testing()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(
    NAME elf_header_tests
    COMMAND ${CMAKE_COMMAND} -E env
        "ELF_ANALYZER=$<TARGET_FILE:elf-analyzer>"
        ${Python3_EXECUTABLE} -m unittest discover
        -s ${CMAKE_SOURCE_DIR}/tests
        -p test_*.py -v
)
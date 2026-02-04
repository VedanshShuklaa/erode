
cmake_minimum_required(VERSION 3.16)

project(
    erode
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(erode
    main.cpp

    lexer/lexer.cpp
    parser/parser.cpp
)

target_include_directories(erode 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lexer
        ${CMAKE_CURRENT_SOURCE_DIR}/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/token
        ${CMAKE_CURRENT_SOURCE_DIR}/ast
)

target_compile_options(erode
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

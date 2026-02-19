cmake_minimum_required(VERSION 3.16)

project(erode VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM_DIR = ${LLVM_DIR}")
message(STATUS "LLVM_INCLUDE_DIRS = ${LLVM_INCLUDE_DIRS}")

set(LLVM_COMPONENTS
    core
    support
    executionengine
    irreader
    native
    orcjit
)

llvm_map_components_to_libnames(LLVM_LIBS ${LLVM_COMPONENTS})

add_executable(erode
    main.cpp
    lexer/lexer.cpp
    parser/parser.cpp
    semantics/semantic_analyzer.cpp
    codegen/codegen.cpp
)

target_include_directories(erode 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lexer
        ${CMAKE_CURRENT_SOURCE_DIR}/parser
        ${CMAKE_CURRENT_SOURCE_DIR}/token
        ${CMAKE_CURRENT_SOURCE_DIR}/ast
        ${CMAKE_CURRENT_SOURCE_DIR}/semantics
        ${CMAKE_CURRENT_SOURCE_DIR}/codegen
)

if(LLVM_INCLUDE_DIRS)
    target_include_directories(erode PRIVATE ${LLVM_INCLUDE_DIRS})
endif()

if(LLVM_DEFINITIONS)
    target_compile_definitions(erode PRIVATE ${LLVM_DEFINITIONS})
endif()

if(DEFINED LLVM_CXX_FLAGS)
    string(STRIP "${LLVM_CXX_FLAGS}" LLVM_CXX_FLAGS)
    separate_arguments(LLVM_CXX_FLAGS)
    target_compile_options(erode PRIVATE ${LLVM_CXX_FLAGS})
endif()

target_link_libraries(erode PRIVATE ${LLVM_LIBS})

target_compile_options(erode
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

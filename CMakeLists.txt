cmake_minimum_required(VERSION 3.30)
project(complier_anchor)

set(CMAKE_CXX_STANDARD 17)

add_executable(complier_anchor main.cpp
        symbol_table.cpp
        symbol_table.h
        scanner.cpp
        scanner.h
        token.cpp
        token.h
        parser.cpp
        parser.h
        ast_nodes.cpp
        ast_nodes.h
        quadruple.h
        ir_generator.cpp
        ir_generator.h
        tinyfiledialogs.c
        tinyfiledialogs.h
        optimizer.cpp
        optimizer.h
        code_generator.cpp
        code_generator.h
)

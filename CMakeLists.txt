cmake_minimum_required(VERSION 4.0)
project(PapuCode)

set(CMAKE_CXX_STANDARD 20)

add_executable(PapuCode
        main.cpp
        lexico.cpp
        sintaxis.cpp
        semantica.cpp
        semantica.h)

cmake_minimum_required(VERSION 3.15)

project(lsdir)

set(CMAKE_CXX_STANDARD 17)

add_executable(
            lsdir
            lsdir.cpp
            argparser.hpp
        )


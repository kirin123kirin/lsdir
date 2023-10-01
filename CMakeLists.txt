cmake_minimum_required(VERSION 3.15)

project(lsdir)

add_executable(
            lsdir
            lsdir.cpp
            argparser.hpp
        )
add_executable(
            shortcut
            shortcut.cpp
            liblnk.hpp
        )

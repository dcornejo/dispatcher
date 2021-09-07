cmake_minimum_required(VERSION 3.15)
project(dispatch C)

set(CMAKE_C_STANDARD 99)

add_executable(dispatch main.c clixon_dispatcher.c clixon_dispatcher.h)

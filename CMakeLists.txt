cmake_minimum_required(VERSION 3.31)
project(HW1_MATAM_SPRING25)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(HW1_MATAM_SPRING25
        BlockChain.h
        Transaction.h
        Utilities.cpp
        Utilities.h
        Transaction.cpp
        BlockChain.cpp)

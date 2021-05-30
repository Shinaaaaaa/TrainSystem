cmake_minimum_required(VERSION 3.16)
project(TrainStation)

set(CMAKE_CXX_STANDARD 14)

add_executable(TrainStation
        Alloc.hpp
        BPT.hpp
        date.hpp
        exception.hpp
        main.cpp
        Map.hpp
        Order.cpp
        Order.h
        program.cpp
        program.h
        String.hpp
        Ticket.cpp
        Ticket.h
        Train.cpp
        Train.h
        Users.cpp
        Users.h
        Vector.hpp
        )
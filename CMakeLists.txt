CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(TCPServer)

ADD_COMPILE_OPTIONS(-std=c++11)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

SET(SRC 
    app/main.cpp
    # net/net.cpp
    # proc/process_cycle.cpp
    # signal/signal.cpp
    )

ADD_EXECUTABLE(TCPServer ${SRC})
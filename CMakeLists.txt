CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(TCPServer)

ADD_COMPILE_OPTIONS(-std=c++11)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

# ===================== source =====================
SET(SRC 
    app/main.cpp
    # net/net.cpp
    proc/Process.cpp
    proc/process_loop.cpp
    signal/signal_handle.cpp
    signal/SignalHandler.cpp
    signal/SignalMask.cpp
    signal/SignalSet.cpp
    )

# ===================== build =====================
ADD_EXECUTABLE(TCPServer ${SRC})
TARGET_LINK_LIBRARIES(TCPServer tools_config tools_log tools_base pthread)

# ===================== install =====================
# TCPServer -- install path = /usr/local/bin
INSTALL(TARGETS TCPServer RUNTIME DESTINATION bin)
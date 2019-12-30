CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(TCPServer)

ADD_COMPILE_OPTIONS(-std=c++11)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

# ===================== source =====================
SET(SRC 
    app/main.cpp
    app/signal_handle.cpp
    net/TCPConnection.cpp
    net/TCPServer.cpp
    proc/Process.cpp
    signal/SignalHandler.cpp
    signal/SignalMask.cpp
    signal/SignalSet.cpp
  )

# ===================== build =====================
ADD_EXECUTABLE(serverd ${SRC})
TARGET_LINK_LIBRARIES(serverd tools_poller tools_socket tools_config tools_log tools_base pthread)

# ===================== install =====================
# TCPServer -- install path = /usr/local/bin
INSTALL(TARGETS serverd RUNTIME DESTINATION bin)
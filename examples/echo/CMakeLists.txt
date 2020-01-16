ADD_EXECUTABLE(EchoClient EchoClient.cpp)
TARGET_LINK_LIBRARIES(EchoClient tools_cxx)

ADD_EXECUTABLE(EchoServer EchoServer.cpp)
TARGET_LINK_LIBRARIES(EchoServer serverd tools_cxx pthread)
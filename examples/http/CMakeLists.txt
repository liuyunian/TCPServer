SET(HTTP_SRC
  HttpContext.cpp
  HttpServer.cpp
)

ADD_EXECUTABLE(HttpServer ${HTTP_SRC})
TARGET_LINK_LIBRARIES(HttpServer serverd tools_poller tools_socket tools_config tools_log tools_base pthread)
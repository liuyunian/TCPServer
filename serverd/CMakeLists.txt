# ===================== source =====================
SET(SERVERD_SRC 
  Acceptor.cpp
  MasterProcess.cpp
  Process.cpp
  SignalHandler.cpp
  SignalMask.cpp
  SignalSet.cpp
  TCPConnection.cpp
  TCPServer.cpp
  WorkerProcess.cpp
)

SET(SERVERD_HDR
  Buffer.h
  Callbacks.h
  TCPConnection.h
  TCPServer.h
)

# ========================= build ==========================
ADD_LIBRARY(serverd STATIC ${SERVERD_SRC})
TARGET_LINK_LIBRARIES(serverd tools_cxx pthread)

# ======================== install =========================
# libserverd.a -- install path = /usr/local/lib
INSTALL(TARGETS serverd ARCHIVE DESTINATION lib)

# *.h -- install path = /usr/local/include/serverd
INSTALL(FILES ${SERVERD_HDR} DESTINATION include/serverd)

# ========================= tests ==========================
ADD_SUBDIRECTORY(tests)
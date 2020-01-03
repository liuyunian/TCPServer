#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

#include <memory>

#include <stdint.h>

#include <tools/base/noncopyable.h>
#include <tools/socket/ConnSocket.h>
#include <tools/poller/Channel.h>

#include "net/Callbacks.h"
#include "net/Buffer.h"

class Poller;

class TCPConnection : public std::enable_shared_from_this<TCPConnection>,
                      noncopyable {
public:
  TCPConnection(Poller *poller, ConnSocket &socket);

  ~TCPConnection() = default;

  void set_connection_callback(const ConnectionCallback &ccb){
    m_connCallback = ccb;
  }

  void set_message_callback(const MessageCallback &mcb){
    m_messageCallback = mcb;
  }

  void set_close_callback(const CloseCallback &ccb){
    m_closeCallback = ccb;
  }

  void set_writeComplete_callback(const WriteCompleteCallback &wccb){
    m_writeCompleteCallback = wccb;
  }

  void set_highWaterMask_callback(const HighWaterMarkCallback &hwmcb){
    m_highWaterMarkCallback = hwmcb;
  }

  void connect_established();

  void send(std::string &&msg);
  void send(const void *msg, ssize_t len);

  void shutdown();

private:
  void handle_read();
  void handle_write();
  void handle_close();
  void handle_error();

  enum State {
    Connecting,
    Connected,
    Disconnecting,
    Disconnected
  };

private:
  State m_state;
  ConnSocket m_socket;
  Channel m_channel;

  size_t m_highWaterMark;

  Buffer m_inputBuffer;
  Buffer m_outputBuffer;

  ConnectionCallback m_connCallback;
  MessageCallback m_messageCallback;
  CloseCallback m_closeCallback;
  WriteCompleteCallback m_writeCompleteCallback;
  HighWaterMarkCallback m_highWaterMarkCallback;
};

#endif // TCPCONNECTION_H_
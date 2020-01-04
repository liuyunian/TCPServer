#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include <memory>
#include <set>

#include <tools/base/noncopyable.h>
#include <tools/poller/Poller.h>
#include <tools/poller/Channel.h>
#include <tools/socket/ServerSocket.h>

#include "serverd/Callbacks.h"

class InetAddress;

class Acceptor : noncopyable {
public:
  explicit Acceptor(const InetAddress &localAddr);

  ~Acceptor() = default;

  void start();

  void set_connection_callback(const ConnectionCallback &ccb){
    m_connCallback = ccb;
  }

  void set_message_callback(const MessageCallback &mcb){
    m_messageCallback = mcb;
  }

  void set_writeComplete_callback(const WriteCompleteCallback &wccb){
    m_writeCompleteCallback = wccb;
  }

private:
  void new_connection();

  void remove_connection(const TCPConnectionPtr &tcpc);

private:
  std::unique_ptr<Poller> m_poller;
  ServerSocket m_serverSocket;
  Channel m_listenChannel;
  int m_idlefd;                     // 预留的文件描述符

  std::set<TCPConnectionPtr> m_connPool;

  ConnectionCallback m_connCallback;
  MessageCallback m_messageCallback;
  WriteCompleteCallback m_writeCompleteCallback;
};

#endif // ACCEPTOR_H_
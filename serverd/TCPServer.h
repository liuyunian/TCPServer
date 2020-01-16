#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <string>
#include <map>

#include <tools/base/noncopyable.h>
#include <tools/config/ConfigFile.h>

#include "serverd/Callbacks.h"

class Acceptor;
class MasterProcess;
class InetAddress;

class TCPServer : noncopyable {
public:
  TCPServer(const std::string &name, char **argv, const InetAddress &localAddr);

  ~TCPServer(); // force out-line dtor, for std::unique_ptr members.

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
  void worker_loop();

private:
  const std::string m_name;

  std::unique_ptr<Acceptor> m_acceptor; // avoid revealing Acceptor

  std::unique_ptr<MasterProcess> m_master;

  ConnectionCallback m_connCallback;
  MessageCallback m_messageCallback;
  WriteCompleteCallback m_writeCompleteCallback;
};

#endif // TCPSERVER_H_
#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#include <string>
#include <map>

#include <tools/base/noncopyable.h>

#include "serverd/Callbacks.h"

class Acceptor;
class InetAddress;

class TCPServer : noncopyable {
public:
  TCPServer(const std::string &name, char **argv, const InetAddress &localAddr);

  ~TCPServer(); // force out-line dtor, for std::unique_ptr members.

  void start();

  void setNumberOfWorker(int num){
    m_workerNum = num;
  }

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
  void master_loop(ProcessPtr master);

  void worker_loop(ProcessPtr worker);

  /**
   * @brief SIGCHLD信号处理函数
  */
  void get_status();

  /**
   * @brief SIGHUP信号处理函数
   * @TODO 
  */
  void handle_sighup();

private:
  const std::string m_name;
  char **m_argv;

  std::unique_ptr<Acceptor> m_acceptor; // avoid revealing Acceptor

  ProcessPtr m_master;
  int m_workerNum;
  std::map<pid_t, ProcessPtr> m_workers;

  ConnectionCallback m_connCallback;
  MessageCallback m_messageCallback;
  WriteCompleteCallback m_writeCompleteCallback;
};

#endif // TCPSERVER_H_
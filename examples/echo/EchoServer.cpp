#include <iostream>
#include <memory>

#include <unistd.h>   // getopt setsid close sleep
#include <fcntl.h>    // open dup2
#include <sys/stat.h> // umask

#include <tools/log/log.h>
#include <tools/socket/InetAddress.h>

#include <serverd/Buffer.h>
#include <serverd/Callbacks.h>
#include <serverd/TCPServer.h>
#include <serverd/TCPConnection.h>

class EchoServer : noncopyable {
public:
  EchoServer(char **argv, const InetAddress &addr) : m_server("echo", argv, addr){
    m_server.set_connection_callback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
    m_server.set_message_callback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
  }

  ~EchoServer() = default;

  void start(){
    LOG_INFO("EchoServer is running");
    m_server.start();
  }

  void onConnection(const TCPConnectionPtr& tcpc){
    LOG_INFO("onConnection(): pid = %d", getpid());
  }

  void onMessage(const TCPConnectionPtr& tcpc, Buffer *buf){
    LOG_INFO("onMessage(): pid = %d, received %d bytes from connection", getpid(), buf->readable_bytes());
    tcpc->send(buf->retrieve_all_as_string());
  }

private:
  TCPServer m_server;
};

#define PORT 9000

int main(int argc, char* argv[]){
  InetAddress addr(PORT);
  EchoServer server(argv, addr);

  server.start();

  return 0;
}
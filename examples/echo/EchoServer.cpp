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

static void 
onConnection(const TCPConnectionPtr& tcpc){
  LOG_INFO("onConnection(): pid = %d", getpid());
}

static void 
onMessage(const TCPConnectionPtr& tcpc, Buffer *buf){
  LOG_INFO("onMessage(): pid = %d, received %d bytes from connection", getpid(), buf->readable_bytes());
  tcpc->send(buf->retrieve_all_as_string());
}

#define PORT 9000

int main(int argc, char* argv[]){
  InetAddress addr(PORT);
  TCPServer server("echo", argv, addr);
  server.set_connection_callback(onConnection);
  server.set_message_callback(onMessage);
  server.start();

  return 0;
}
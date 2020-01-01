#include <fcntl.h>  // open
#include <errno.h>  // errno

#include <tools/log/log.h>
#include <tools/base/Exception.h>
#include <tools/socket/SocketsOps.h>
#include <tools/socket/InetAddress.h>
#include <tools/socket/ConnSocket.h>

#include "net/TCPServer.h"
#include "net/TCPConnection.h"

static const int kTimeoutMs = 10000;

TCPServer::TCPServer(const InetAddress &localAddr) : 
  m_poller(Poller::new_default_Poller()),
  m_serverSocket(localAddr),
  m_listenChannel(m_poller.get(), m_serverSocket.get_sockfd()),
  m_idlefd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
  m_serverSocket.set_reuse_address(true); // 地址可以重复使用
  m_serverSocket.set_reuse_port(true);    // 端口可以重复使用
  m_serverSocket.listen();

  m_listenChannel.set_read_callback(std::bind(&TCPServer::new_connection, this));
  m_listenChannel.enable_reading();
}

void TCPServer::start(){
  for(;;){
    Poller::ChannelList activeChannels = m_poller->poll(kTimeoutMs);
    for(auto &channelPtr : activeChannels){
      channelPtr->handle_event();
    }
  }
}

void TCPServer::new_connection(){
  try{
    ConnSocket connSocket = m_serverSocket.accept_nonblocking();
    TCPConnectionPtr tcpc(new TCPConnection(m_poller.get(), connSocket));
    m_connPool.insert(tcpc);
    tcpc->set_connection_callback(m_connCallback);
    tcpc->set_message_callback(m_messageCallback);
    tcpc->set_close_callback(std::bind(&TCPServer::remove_connection, this, std::placeholders::_1));
    tcpc->connect_established();
  }
  catch(const Exception &e){
    if(errno == EMFILE){
      sockets::close(m_idlefd);
      {
        ConnSocket connSocket = m_serverSocket.accept_nonblocking();
      }
      
      m_idlefd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
    else{
      LOG_SYSERR("Failed to accept in TCPServer::onConnection()");
    }
  }
}

void TCPServer::remove_connection(const TCPConnectionPtr &tcpc){
  m_connPool.erase(tcpc);
}
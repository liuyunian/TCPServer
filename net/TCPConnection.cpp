#include <assert.h> // assert

#include <tools/log/log.h>
#include <tools/poller/Poller.h>

#include "net/TCPConnection.h"

TCPConnection::TCPConnection(Poller *poller, ConnSocket &socket) : 
  m_state(Connecting),
  m_socket(std::move(socket)),
  m_channel(poller, m_socket.get_sockfd())
{
  m_channel.set_read_callback(std::bind(&TCPConnection::handle_read, this));
  m_channel.set_close_callback(std::bind(&TCPConnection::handle_close, this));
  m_channel.set_error_callback(std::bind(&TCPConnection::handle_error, this));
}

void TCPConnection::connect_established(){
  assert(m_state == Connecting);
  m_state = Connected;
  m_channel.enable_reading();

  m_connCallback(shared_from_this());
}

void TCPConnection::handle_read(){
  char buf[65536];
  ssize_t len = m_socket.read(buf, sizeof(buf));
  if(len > 0){
    m_messageCallback(shared_from_this(), buf, len);
  }
  else if(len == 0){
    LOG_INFO("client disconnection");
    handle_close();
  }
  else{
    LOG_SYSERR("read error in TcpConnection::handle_read");
  }
}

void TCPConnection::handle_close(){
  assert(m_state == Connected || m_state == Disconnecting);
  m_state == Disconnected;

  m_channel.remove();

  m_closeCallback(shared_from_this());
}

void TCPConnection::handle_error(){
  LOG_ERR("TCPConnection::handle_error");
}


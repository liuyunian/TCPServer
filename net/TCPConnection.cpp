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
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = m_inputBuffer.writable_bytes();
  vec[0].iov_base = m_inputBuffer.writable_index();
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t len = m_socket.readv(vec, iovcnt);
  if(len < 0){
    LOG_SYSERR("read error in TcpConnection::handle_read");
    handle_error();
  }
  else if(len == 0){
    handle_close();
  }
  else{
    if(static_cast<size_t>(len) <= writable){
      m_inputBuffer.adjust_writer_index(len);
    }
    else{
      m_inputBuffer.adjust_writer_index(writable);
      m_inputBuffer.append(extrabuf, len - writable);
    }

    m_messageCallback(shared_from_this(), &m_inputBuffer);
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


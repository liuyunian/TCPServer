#include <assert.h> // assert

#include <tools/log/log.h>
#include <tools/poller/Poller.h>

#include "serverd/TCPConnection.h"

TCPConnection::TCPConnection(Poller *poller, ConnSocket &socket) : 
  m_state(Connecting),
  m_socket(std::move(socket)),
  m_channel(poller, m_socket.get_sockfd()),
  m_highWaterMark(64*1024*1024)
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

void TCPConnection::send(const std::string &msg){
  send(msg.c_str(), static_cast<ssize_t>(msg.size()));
}

void TCPConnection::send(const void *msg, ssize_t len){
  if(m_state == Disconnected){
    LOG_WARN("disconnected, give up writing");
    return;
  }

  ssize_t nwrote = 0;
  ssize_t remaining = len;
  if(!m_channel.is_writing() && m_outputBuffer.readable_bytes() == 0){
    nwrote = m_socket.write(msg, len);
    if(nwrote < 0){
      nwrote = 0;
      if(errno != EWOULDBLOCK){
        LOG_SYSERR("Failed to send msg in TCPConnection::send()");
      }
    }
    else{
      remaining -= nwrote;
      if(remaining == 0 && m_writeCompleteCallback){
        m_writeCompleteCallback(shared_from_this());
      }
    }
  }

  assert(remaining <= len);
  if(remaining > 0){
    size_t oldLen = m_outputBuffer.readable_bytes();

    if(oldLen+remaining >= m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback){
      m_highWaterMarkCallback(shared_from_this(), oldLen+remaining);
    }

    m_outputBuffer.append(static_cast<const char*>(msg)+nwrote, remaining);
    if (!m_channel.is_writing())
    {
      m_channel.enable_writing();
    }
  }
}

void TCPConnection::shutdown(){
  if(m_state == Connected){
    m_state = Disconnecting;
    if(!m_channel.is_writing()){
      m_socket.shutdown_write();
    }
  }
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

void TCPConnection::handle_write(){
  if(m_channel.is_writing()){
    ssize_t n = m_socket.write(m_outputBuffer.readable_index(), m_outputBuffer.readable_bytes());
    if(n < 0){
      LOG_SYSERR("TCPConnection::handle_write()");
    }
    else{
      m_outputBuffer.retrieve(n);
      if(m_outputBuffer.readable_bytes() == 0){
        m_channel.disable_writing();

        if(m_writeCompleteCallback){
          m_writeCompleteCallback(shared_from_this());
        }

        if(m_state == Disconnecting){
          m_socket.shutdown_write();
        }
      }
    }
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


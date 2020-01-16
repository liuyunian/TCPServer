#include <functional>

#include <tools/log/log.h>
#include <tools/base/Singleton.h>
#include <tools/base/Exception.h>
#include <tools/config/ConfigFile.h>

#include "serverd/TCPServer.h"
#include "serverd/Acceptor.h"
#include "serverd/MasterProcess.h"

TCPServer::TCPServer(const std::string &name, char **argv, const InetAddress &localAddr) : 
  m_name(name),
  m_acceptor(new Acceptor(localAddr)),
  m_master(new MasterProcess(std::string(name).append("(m)"), argv))
{
  ConfigFile &cf = Singleton<ConfigFile>::instance();
  try{
    cf.load("../serverd.conf"); // FIXME: 绝对路径
  }
  catch(const Exception &e){
    LOG_FATAL("%s", e.what());
  }
}

TCPServer::~TCPServer(){}

void TCPServer::start(){
  m_master->set_workerLoop(std::bind(&TCPServer::worker_loop, this));
  m_master->start();
}

void TCPServer::worker_loop(){
  m_acceptor->set_connection_callback(m_connCallback);
  m_acceptor->set_message_callback(m_messageCallback);
  m_acceptor->set_writeComplete_callback(m_writeCompleteCallback);
  m_acceptor->start();
}
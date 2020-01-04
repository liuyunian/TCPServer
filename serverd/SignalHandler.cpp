#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/signalfd.h>

#include <tools/log/log.h>

#include "serverd/SignalHandler.h"

SignalHandler::SignalHandler(SignalMask &mask) : 
  m_sigfd(-1),
  m_mask(mask)
{}

SignalHandler::~SignalHandler(){
  if(m_sigfd != -1){
    ::close(m_sigfd);
  }
}

void SignalHandler::register_signal(int signo, SignalCallback cb){
  if(m_signalCallbacks.find(signo) != m_signalCallbacks.end()){
    return;
  }

  if(!m_mask.contain(signo)){
    m_mask.add(signo);
    m_mask.update();
  }

  m_signalCallbacks.insert({signo, cb});
  update();
}

void SignalHandler::unregister_signal(int signo){
  auto iter = m_signalCallbacks.find(signo);
  if(iter == m_signalCallbacks.end()){
    return;
  }
  m_signalCallbacks.erase(iter);

  if(m_mask.contain(signo)){
    m_mask.remove(signo);
    m_mask.update();
  }
  
  update();
}

void SignalHandler::wait(){
  for(;;){
    struct signalfd_siginfo fdsi;
    ssize_t n = ::read(m_sigfd, &fdsi, sizeof(struct signalfd_siginfo));
    if(n != sizeof(struct signalfd_siginfo)){
      LOG_SYSFATAL("read error in SignalHandler::wait()");
    }

    if(m_signalCallbacks[fdsi.ssi_signo]){
      m_signalCallbacks[fdsi.ssi_signo]();
    }
  }
}

void SignalHandler::update(){
  sigset_t mask = m_mask.get();
  m_sigfd = signalfd(m_sigfd, &mask, SFD_CLOEXEC);
  if(m_sigfd < 0){
    LOG_SYSFATAL("signalfd error in SignalHandler::update()");
  }
}
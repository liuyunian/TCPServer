#include <string.h>

#include <tools/base/Singleton.h>
#include <tools/log/log.h>

#include "signal/SignalHandler.h"

SignalHandler::SignalHandler(SignalMask &mask) : 
  m_mask(mask)
{}

void SignalHandler::register_signal(int signo, SignalSet set, SignalCallback cb){
  if(!m_mask.contain(signo)){
      m_mask.add(signo);
      m_mask.update();
  }

  update(signo, set, cb);
}

void SignalHandler::unregister_signal(int signo){
  update(signo, SignalSet::get_empty_sigset(), SIG_IGN);
}

void SignalHandler::update(int signo, SignalSet set, SignalCallback cb){
  struct sigaction sa;
  sa.sa_handler = cb;
  sa.sa_flags = 0;
  sa.sa_mask = set.get();

  if(::sigaction(signo, &sa, nullptr) < 0){
      LOG_SYSFATAL("Failed to update handler for %s in SignalHandler::update()", strsignal(signo));
  }
}
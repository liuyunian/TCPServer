#include <signal.h>

#include <tools/log/log.h>

#include "serverd/SignalMask.h"

sigset_t get_process_sigset(){
  sigset_t set;
  if(::sigprocmask(0, nullptr, &set) < 0){
    LOG_SYSFATAL("Failed to get process sigset in get_process_sigset()");
  }

  return set;
}

SignalMask::SignalMask() : 
  SignalSet(get_process_sigset())
{}

void SignalMask::update(){
  if(::sigprocmask(SIG_SETMASK, &m_sigset, nullptr) < 0){
    LOG_SYSFATAL("Failed to update process sigset in SignalMask::update()");
  }
}


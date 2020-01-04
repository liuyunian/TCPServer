#include <assert.h> // assert
#include <string.h> // memset

#include <tools/log/log.h>

#include "serverd/SignalSet.h"

SignalSet::SignalSet(sigset_t set) : 
  m_sigset(set)
{}

void SignalSet::add(int signo){
  if(::sigaddset(&m_sigset, signo) < 0){
    LOG_SYSFATAL("Failed to add signal to sigset in SignalSet::add()");
  }
}

void SignalSet::remove(int signo){
  if(::sigdelset(&m_sigset, signo) < 0){
    LOG_SYSFATAL("Failed to remove signal from sigset in SignalSet::remove()");
  }
}

void SignalSet::clear(){
  if(::sigemptyset(&m_sigset) < 0){
    LOG_SYSFATAL("Failed to clear sigset in SignalSet::clear()");
  }
}

void SignalSet::fill(){
  if(::sigfillset(&m_sigset) < 0){
    LOG_SYSFATAL("Failed to fill sigset in SignalSet::fill()");
  }
}

bool SignalSet::contain(int signo){
  if(::sigismember(&m_sigset, signo) == 0){
    return false;
  }

  return true;
}

SignalSet SignalSet::get_empty_sigset(){
  sigset_t set;
  SignalSet rtnSet(set);
  rtnSet.clear();
  return rtnSet;
}

SignalSet SignalSet::get_fill_sigset(){
  sigset_t set;
  SignalSet rtnSet(set);
  rtnSet.fill();
  return rtnSet;
}
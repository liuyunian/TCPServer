#include <signal.h>
#include <tools/log/log.h>

#include <serverd/SignalMask.h>
#include <serverd/SignalHandler.h>

void handle_sighup(){
  LOG_INFO("recv signal: SIGHUP");
}

void handle_sigint(){
  LOG_INFO("recv signal: SIGINT");
}

int main(){
  SignalMask mask;

  SignalHandler handler(mask);
  handler.register_signal(SIGHUP, handle_sighup);
  handler.register_signal(SIGINT, handle_sigint);
  handler.wait();

  return 0;
}
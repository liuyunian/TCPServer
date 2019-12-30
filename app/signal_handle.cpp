#include <stdio.h>
#include <assert.h>
#include <sys/wait.h>   // waitpid

#include <tools/log/log.h>

#include "signal/SignalHandler.h"

/**
 * SIGCHLD信号处理函数
*/
void Signal::get_status(int signo){
  assert(signo == SIGCHLD);

  int status;
  pid_t pid = waitpid(-1, &status, WNOHANG);
  if(pid < 0){
    LOG_SYSERR("Failed to get child process status in Signal::get_status()");
    return;
  }
  else if(pid == 0){
    return;
  }

  // WTERMSIG()获取终止进程的信号值，用于判断子进程是不是被信号终止的
  // WEXITSTATUS()获取子进程传递给exit或者_exit参数的低八位，用于判断进程终止的原因
  if(WTERMSIG(status)){
    LOG_INFO("worker process pid = %d exited on signal %d", pid, WTERMSIG(status));
  }
  else{
    LOG_INFO("worker process pid = %d exited with code %d\n", pid, WEXITSTATUS(status)); 
  }
}
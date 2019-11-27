#include <unistd.h> // fork sleep
#include <signal.h> // signal相关

#include <tools/log/log.h>
#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>

#include "proc/Process.h"
#include "signal/SignalHandler.h"

void master_process_loop(char **argv){
  // [1] 屏蔽信号，不希望在fork子进程时被信号打断
  SignalMask mask;
  mask.clear();
  mask.add(SIGCHLD);     // 子进程状态改变
  mask.add(SIGALRM);     // 定时器超时
  mask.add(SIGIO);       // 异步I/O
  mask.add(SIGINT);      // 终端中断符
  mask.add(SIGHUP);      // 连接断开
  mask.add(SIGUSR1);     // 用户定义信号
  mask.add(SIGUSR2);     // 用户定义信号
  mask.add(SIGWINCH);    // 终端窗口大小改变
  mask.add(SIGTERM);     // 终止
  mask.add(SIGQUIT);     // 终端退出符
  mask.update();

  // [2] fork worker进程
  std::string procName(::basename(argv[0]));
  procName.append("(w)");

  ConfigFile &cf = Singleton<ConfigFile>::instance();
  int workProcessNum = cf.get<int>("WorkProcess");
  for(int i = 0; i < workProcessNum; ++ i){
      Process(procName, argv, worker_process_loop);
  }

  // [3] 解除屏蔽的信号
  mask.clear();
  mask.update();

  // [4] 注册信号处理函数
  SignalHandler handler(mask);
  handler.register_signal(SIGCHLD, SignalSet::get_fill_sigset(), Signal::get_status);

  // [5] 阻塞在这里，等待一个信号，只有收到信号才会被唤醒
  sigset_t set = mask.get();
  for(;;){
    sigsuspend(&set);
  }
}

void worker_process_loop(){
  for(;;){
      LOG_INFO("worker process sleep 100ms");
      ::usleep(100 * 1000);
  }
}
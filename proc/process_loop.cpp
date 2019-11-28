#include <vector>

#include <unistd.h>   // setsid close sleep
#include <fcntl.h>    // open dup2
#include <sys/stat.h> // umask

#include <tools/log/log.h>
#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>

#include "proc/Process.h"
#include "signal/SignalHandler.h"

void process::daemon_process_loop(ProcessPtr master){
  // [1] 设置守护进程
  if(::setsid() < 0){
    LOG_SYSFATAL("Failed to setsid in process::daemon_process_loop()");
  }

  ::umask(0);

  int fd = ::open("/dev/null", O_RDWR);
  if(fd < 0){
    LOG_SYSFATAL("Failed to open \"/dev/null\" in process::daemon_process_loop()");
  }

  if(::dup2(fd, STDIN_FILENO) < 0){
    LOG_SYSFATAL("Failed to dup2 STDIN in process::daemon_process_loop()");
  }

  if(::dup2(fd, STDOUT_FILENO) < 0){
    LOG_SYSFATAL("Failed to dup2 STDOUT&STDERR in process::daemon_process_loop()");
  }

  if(fd > STDERR_FILENO){
    if(::close(fd) < 0){
      LOG_SYSFATAL("Failed to close \"/dev/null\" in process::daemon_process_loop()");
    }
  }

  // [2] 创建worker进程
  master->set_mask({SIGCHLD,     // 子进程状态改变
                    SIGALRM,     // 定时器超时
                    SIGIO,       // 异步I/O
                    SIGINT,      // 终端中断符
                    SIGHUP,      // 连接断开
                    SIGUSR1,     // 用户定义信号
                    SIGUSR2,     // 用户定义信号
                    SIGWINCH,    // 终端窗口大小改变
                    SIGTERM,     // 终止
                    SIGQUIT      // 终端退出符
                  });            // 屏蔽信号，不希望在fork子进程时被信号打断
  
  ConfigFile &cf = Singleton<ConfigFile>::instance();
  int workProcessNum = cf.get<int>("WorkProcess");
  std::vector<ProcessPtr> workers;
  for(int i = 0; i < workProcessNum; ++ i){
    auto worker = std::make_shared<Process>("serverd(w)", master->get_argv(), process::worker_process_loop);
    worker->start();

    workers.push_back(worker);
  }

  // [3] 解除屏蔽的信号
  master->clear_mask();

  // [4] 注册信号处理函数
  SignalHandler handler(master->get_mask());
  handler.register_signal(SIGCHLD, SignalSet::get_fill_sigset(), Signal::get_status);

  // [5] 阻塞等待信号醒
  sigset_t set = master->get_mask().get();
  for(;;){
    sigsuspend(&set);
  }
}

void process::worker_process_loop(ProcessPtr worker){
  for(;;){
      LOG_INFO("worker process sleep 100ms");
      ::usleep(100 * 1000);
  }
}
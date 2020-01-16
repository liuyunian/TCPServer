#include <unistd.h>       // fork setsid close read write
#include <fcntl.h>        // open dup2
#include <assert.h>       // assert
#include <sys/stat.h>     // umask
#include <sys/wait.h>     // waitpid
#include <sys/eventfd.h>  // eventfd

#include <tools/base/Singleton.h>
#include <tools/base/Exception.h>
#include <tools/config/ConfigFile.h>
#include <tools/log/log.h>
#include <tools/log/LogFile.h>

#include "serverd/MasterProcess.h"
#include "serverd/WorkerProcess.h"
#include "serverd/SignalHandler.h"

static int 
create_eventfd(){
  int evtfd = ::eventfd(0, 0);
  if(evtfd < 0){
    LOG_SYSFATAL("eventfd error");
  }

  return evtfd;
}

MasterProcess::MasterProcess(const std::string &name, char **argv) : 
  Process(name, argv),
  m_eventfd(create_eventfd()){}

void MasterProcess::start(){
  ConfigFile &cf = Singleton<ConfigFile>::instance();
  bool daemon = cf.get<bool>("Daemon");
  if(daemon){
    create_daemon();
    set_log();
  }

  set_title();

  create_worker();

  SignalHandler handler(get_mask());
  handler.register_signal(SIGCHLD, std::bind(&MasterProcess::handle_sigchld, this));
  handler.register_signal(SIGHUP, std::bind(&MasterProcess::handle_sighup, this));
  for(;;){
    handler.wait();
  }
}

void MasterProcess::create_daemon(){
  pid_t pid = ::fork();
  if(pid == -1){
    LOG_SYSFATAL("fork error in Process::start(bool)");
  }
  else if(pid == 0){
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
  }
  else{
    exit(0);
  }
}

void MasterProcess::create_worker(){
  set_mask({SIGCHLD,     // 子进程状态改变
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
  int workerNum = cf.get<int>("WorkerProcess");
  assert(workerNum >= 1);

  std::string name(get_name());
  name = name.substr(0, name.find('(')).append("(w)");
  WorkerProcess *worker;
  for(int i = 0; i < workerNum; ++ i){
    try{
      worker = new WorkerProcess(name, get_argv(), m_eventfd, m_workerLoop);
    }
    catch(const Exception &e){
      LOG_FATAL("Failed to create WorkerProcess");
    }

    pid_t pid = static_cast<pid_t>(eventfd_wait());
    m_workers.insert({pid, worker});
  }

  clear_mask();
}

void MasterProcess::handle_sigchld(){
  int status;
  pid_t pid = waitpid(-1, &status, WNOHANG);
  if(pid < 0){
    LOG_SYSERR("waitpid error");
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

  auto iter = m_workers.find(pid);
  assert(iter != m_workers.end());
  delete iter->second;
  m_workers.erase(iter);

  // @TODO 再次创建一个worker
}

void MasterProcess::handle_sighup(){
  LOG_INFO("recv signal: SIGHUP");

  // @TODO 支持热更新
}

uint64_t MasterProcess::eventfd_wait(){
  uint64_t one;
  ssize_t n = ::read(m_eventfd, &one, sizeof one);
  if(n != sizeof one){
    LOG_SYSERR("TCPServer::eventfd_wait() reads %d bytes instead of 8", n);
  }

  return one;
}
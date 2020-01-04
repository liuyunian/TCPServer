#include <functional>

#include <assert.h>   // assert
#include <unistd.h>   // getopt setsid close sleep
#include <fcntl.h>    // open dup2
#include <sys/stat.h> // umask
#include <sys/wait.h> // waitpid

#include <tools/log/log.h>

#include "serverd/Process.h"
#include "serverd/Acceptor.h"
#include "serverd/TCPServer.h"
#include "serverd/SignalHandler.h"

TCPServer::TCPServer(const std::string &name, char **argv, const InetAddress &localAddr) : 
  m_name(name),
  m_argv(argv),
  m_acceptor(new Acceptor(localAddr)),
  m_master(nullptr),
  m_workerNum(1)
{}

TCPServer::~TCPServer(){}

void TCPServer::start(){
  std::string name(m_name);
  m_master = std::make_shared<Process>(name.append("(m)"), m_argv, std::bind(&TCPServer::master_loop, this, std::placeholders::_1));
  m_master->start(true);
}

void TCPServer::master_loop(ProcessPtr master){
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

  // [2] 关联日志文件
  std::unique_ptr<LogFile> logFile(new LogFile(master->get_name())); // 默认的flushInterval = 3s, checkEveryN = 1024
  Log::set_output(std::bind(&LogFile::append, logFile.get(), std::placeholders::_1, std::placeholders::_2));
  Log::set_flush(std::bind(&LogFile::flush, logFile.get()));

  // [3] 创建worker进程
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

  std::string name(m_name);
  for(int i = 0; i < m_workerNum; ++ i){
    auto worker = std::make_shared<Process>(name.append("(w)"), m_argv, std::bind(&TCPServer::worker_loop, this, std::placeholders::_1));
    m_workers.insert({worker->get_pid(), worker});
    worker->start();
  }

  // [4] 解除屏蔽的信号
  master->clear_mask();

  // [5] 注册信号处理函数并阻塞等待信号唤醒
  SignalHandler handler(master->get_mask());
  handler.register_signal(SIGCHLD, std::bind(&TCPServer::get_status, this));
  handler.register_signal(SIGHUP, std::bind(&TCPServer::handle_sighup, this));
  for(;;){
    handler.wait();
  }
}

void TCPServer::worker_loop(ProcessPtr worker){
  // [1] 屏蔽信号
  /**
   * SIGPIPE信号发生的时机：
   * 如果客户端没有按照四次挥手关闭TCP连接，那么服务器如果调用write或者send发送数据时，会收到一个RST报文段，相应的两个函数错误返回
   * 如果再次调用write或者send发送数据，那么就会收到一个内核发来的SIGPIPE信号，该信号的默认处理动作是终止进程
  */
  worker->set_mask(SIGPIPE);

  // [2] 关联日志文件
  std::unique_ptr<LogFile> logFile(new LogFile(worker->get_name())); // 默认的flushInterval = 3s, checkEveryN = 1024
  Log::set_output(std::bind(&LogFile::append, logFile.get(), std::placeholders::_1, std::placeholders::_2));
  Log::set_flush(std::bind(&LogFile::flush, logFile.get()));

  // [3] 运行Acceptor
  m_acceptor->set_connection_callback(m_connCallback);
  m_acceptor->set_message_callback(m_messageCallback);
  m_acceptor->set_writeComplete_callback(m_writeCompleteCallback);
  m_acceptor->start();
}

void TCPServer::get_status(){
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

  size_t n = m_workers.erase(pid);
  assert(n == 1);

  // @TODO 再次创建一个worker
}

void TCPServer::handle_sighup(){
  LOG_INFO("recv signal: SIGHUP");

  // @TODO 支持热更新
}
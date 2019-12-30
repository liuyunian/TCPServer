#include <iostream>
#include <memory>

#include <unistd.h>   // getopt setsid close sleep
#include <fcntl.h>    // open dup2
#include <sys/stat.h> // umask

#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>
#include <tools/log/log.h>
#include <tools/log/LogFile.h>
#include <tools/socket/InetAddress.h>

#include "proc/Process.h"
#include "signal/SignalHandler.h"
#include "net/TCPServer.h"
#include "net/Callbacks.h"

static void 
onConnection(const TCPConnectionPtr& conn){
  LOG_INFO("onConnection(): pid = %d", getpid());
}

static void 
onMessage(const TCPConnectionPtr& conn,  const char* data, ssize_t len){
  LOG_INFO("onMessage(): pid = %d, received %d bytes from connection", getpid(), len);
}

static void 
worker_process_loop(ProcessPtr worker){
  // [1] 日志个性化设置
  worker->get_logFile()->setCheckEveryN(1);

  // [2] 屏蔽信号
  /**
   * 屏蔽SIGPIPE信号
   * SIGPIPE信号发生的实际：如果客户端没有按照四次挥手关闭TCP连接，那么服务器如果调用write或者send发送数据时，会收到一个RST报文段，相应的两个函数错误返回
   * 如果再次调用write或者send发送数据，那么就会收到一个内核发来的SIGPIPE信号，该信号的默认处理动作是终止进程
   * 为了服务器能够长时间稳定原型，这里忽略该信号
  */
  worker->set_mask(SIGPIPE);

  ConfigFile &cf = Singleton<ConfigFile>::instance();
  const int port = cf.get<int>("Port");
  InetAddress addr(port);
  TCPServer server(addr);
  server.set_connection_callback(onConnection);
  server.set_message_callback(onMessage);

  server.start();
}

static void 
master_process_loop(ProcessPtr master){
  // [1] 日志个性化设置
  master->get_logFile()->setCheckEveryN(1);

  // [2] 设置守护进程
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
  
  ConfigFile &cf = Singleton<ConfigFile>::instance();
  int workProcessNum = cf.get<int>("WorkProcess");
  std::vector<ProcessPtr> workers;
  for(int i = 0; i < workProcessNum; ++ i){
    auto worker = std::make_shared<Process>("serverd(w)", master->get_argv(), worker_process_loop);
    worker->start();

    workers.push_back(worker);
  }

  // [4] 解除屏蔽的信号
  master->clear_mask();

  // [5] 注册信号处理函数
  SignalHandler handler(master->get_mask());
  handler.register_signal(SIGCHLD, SignalSet::get_fill_sigset(), Signal::get_status);

  // [6] 阻塞等待信号醒
  sigset_t set = master->get_mask().get();
  for(;;){
    sigsuspend(&set);
  }
}

static void 
printUsage(std::ostream &os, const std::string programName){
  os << "Usage: " << programName << " [OPTIONS...]\n"
    << "OPTIONS:\n"
    << "    -f <FILE>   Set Path to configuration file with absolute path\n"
    << "    -h          Display this help message\n"
    << "    -V          Display version information"
    << std::endl;
}

int main(int argc, char* argv[]){
  int opt;
  char *configFilePath = const_cast<char*>("serverd.conf");
  while((opt = getopt(argc, argv, "hf:V")) != -1) {
    switch(opt) {
    case 'h':
      printUsage(std::cout, argv[0]);
      return 0;
    case 'f':
      configFilePath = optarg;
      break;
    case 'V':
      std::cout << "Don't have version information" << std::endl;
      return 0;
    default:
      printUsage(std::cerr, argv[0]);
      return 1;
    }
  }

  ConfigFile &cf = Singleton<ConfigFile>::instance(); // 采用单例模式创建配置文件解析对象
  if(!cf.load(configFilePath)){
    LOG_FATAL("Failed to load configFile: %s", configFilePath);
  }

  auto master = std::make_shared<Process>("serverd(m)", argv, master_process_loop);
  master->start(true);

  return 0;
}
#ifndef MASTERPROCESS
#define MASTERPROCESS

#include <string>

#include "serverd/Callbacks.h"
#include "serverd/Process.h"

class WorkerProcess;

class MasterProcess : public Process
{
public:
  MasterProcess(const std::string &name, char **argv);
  ~MasterProcess(){}

  void start();

  void set_workerLoop(const WorkerLoop &loop){
    m_workerLoop = loop;
  }

private:
  void create_daemon();

  void create_worker();

  uint64_t eventfd_wait();

  /**
   * @brief SIGCHLD信号处理函数
  */
  void handle_sigchld();

  /**
   * @brief SIGHUP信号处理函数
   * @TODO 
  */
  void handle_sighup();

private:
  int m_eventfd;
  std::map<pid_t, WorkerProcess*> m_workers;
  WorkerLoop m_workerLoop;
};

#endif // MASTERPROCESS
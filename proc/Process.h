#ifndef PROCESS_H_
#define PROCESS_H_

#include <vector>
#include <memory>
#include <functional>

#include <signal.h>

#include <tools/base/noncopyable.h>
#include <tools/log/LogFile.h>

#include "signal/SignalMask.h"

class Process;
typedef std::shared_ptr<Process> ProcessPtr;

namespace process {
  void daemon_process_loop(ProcessPtr);

  void worker_process_loop(ProcessPtr);
};

class Process : noncopyable,
                public std::enable_shared_from_this<Process>
{
public:
  typedef std::function<void(ProcessPtr)> ProcessLoop;

  Process(const std::string &name, char **argv, const ProcessLoop &loop);
  ~Process() = default;

  void start(int checkEveryN = 1024, bool parentExit = false);

  void set_mask(const std::initializer_list<int>& signals);

  SignalMask& get_mask(){
    return m_mask;
  }

  void clear_mask();

  char** get_argv(){
    return m_argv;
  }

private:
  void set_title();

private:
  std::string m_name;
  pid_t m_pid;

  char **m_argv;                      // 命令行参数
  SignalMask m_mask;                  // 信号屏蔽字
  std::vector<std::string> m_env;                   
  std::unique_ptr<LogFile> m_logFile; // 日志文件

  ProcessLoop m_loop;
};

#endif // PROCESS_H_
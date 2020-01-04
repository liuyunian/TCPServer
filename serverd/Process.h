#ifndef PROCESS_H_
#define PROCESS_H_

#include <vector>
#include <memory>
#include <functional>

#include <signal.h>

#include <tools/base/noncopyable.h>
#include <tools/log/LogFile.h>

#include "serverd/Callbacks.h"
#include "serverd/SignalMask.h"

class Process : noncopyable,
                public std::enable_shared_from_this<Process>
{
public:
  typedef std::function<void(ProcessPtr)> ProcessLoop;

  Process(const std::string &name, char **argv, const ProcessLoop &loop);
  ~Process() = default;

  void start(bool parentExit = false);

  std::string get_name() const {
    return m_name;
  }

  pid_t get_pid() const {
    return m_pid;
  }

  void set_mask(int signal);

  void set_mask(const std::initializer_list<int>& signals);

  SignalMask& get_mask(){
    return m_mask;
  }

  void clear_mask();

private:
  void set_title();

private:
  std::string m_name;
  pid_t m_pid;

  char **m_argv;                  // 命令行参数
  std::vector<std::string> m_env; // 环境变量
  SignalMask m_mask;              // 信号屏蔽字

  ProcessLoop m_loop;
};

#endif // PROCESS_H_
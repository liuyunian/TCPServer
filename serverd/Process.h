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

class Process : noncopyable {
protected:
  Process(const std::string &name, char **argv);
  virtual ~Process();

  std::string get_name() const {
    return m_name;
  }

  char** get_argv() const {
    return m_argv;
  }

  void set_mask(int signal);

  void set_mask(const std::initializer_list<int>& signals);

  SignalMask& get_mask(){
    return m_mask;
  }

  void clear_mask();

  void set_title();

  void set_log();

private:
  std::string m_name;

  char **m_argv;                  // 命令行参数
  std::vector<std::string> m_env; // 环境变量
  SignalMask m_mask;              // 信号屏蔽字

  std::unique_ptr<LogFile> m_logFile;
};

#endif // PROCESS_H_
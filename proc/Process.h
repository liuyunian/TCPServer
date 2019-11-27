#ifndef PROCESS_H_
#define PROCESS_H_

#include <vector>
#include <memory>
#include <functional>

#include <signal.h>

#include <tools/base/noncopyable.h>
#include <tools/log/LogFile.h>

#include "signal/SignalMask.h"

class Process : noncopyable {
public:
    typedef std::function<void()> ProcessCallback;

    Process(const std::string &name, char **argv, const ProcessCallback &callback, bool parentExit = false);
    ~Process() = default;

private:
    void loop();

    void set_title();

private:
    std::string m_name;
    pid_t m_pid;

    char **m_argv;                      // 命令行参数
    SignalMask m_mask;                  // 信号屏蔽字
    std::vector<std::string> m_env;                   
    std::unique_ptr<LogFile> m_logFile; // 日志文件

    ProcessCallback m_callback;
};

void master_process_loop(char **argv);

void worker_process_loop();

#endif // PROCESS_H_
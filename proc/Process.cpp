#include <string.h> // memset
#include <unistd.h> // fork
#include <stdlib.h> // environ
#include <stdio.h>

#include <tools/log/log.h>
#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>

#include "proc/Process.h"

Process::Process(const std::string &name, char **argv, sigset_t sigset, const ProcessCallback &callback, bool parentExit) : 
    m_name(name),
    m_argv(argv),
    m_sigset(sigset),
    m_callback(std::move(callback))
{
    m_pid = ::fork();
    if(m_pid == -1){
        LOG_SYSFATAL("Failed to create child process");
    }
    else if(m_pid == 0){
        loop();
    }
    else{
        if(parentExit){
            exit(0);
        }
    }
}

void Process::loop(){
    // [1] 设置信号屏蔽
    if(::sigprocmask(SIG_SETMASK, &m_sigset, NULL) < 0){
        LOG_SYSFATAL("Failed to set sigset in Process::loop()");
    }

    // [2] 关联日志文件
    ConfigFile &cf = Singleton<ConfigFile>::instance();
    std::string logFilePath = cf.get<std::string>("LogFile");
    logFilePath.append(m_name);
    m_logFile.reset(new LogFile(logFilePath.c_str(), 200*1000));
    Log::set_output(std::bind(&LogFile::append, m_logFile.get(), std::placeholders::_1, std::placeholders::_2));
    Log::set_flush(std::bind(&LogFile::flush, m_logFile.get()));

    // [3] 设置标题
    set_title();

    // [4] 进入工作循环
    m_callback();
}

void Process::set_title(){
    // 将环境变量复制到新创建的空间
    char *oldEnv = environ[0];
    size_t len;
    for(int i = 0; environ[i]; ++ i){
        m_env.push_back(std::string(environ[i]));

        len = strlen(environ[i]) + 1;
        memset(environ[i], 0, len);
        environ[i] = const_cast<char*>(m_env.back().c_str());
    }

    // 清除命令行参数
    len = 0;
    for(int i = 0; m_argv[i]; ++ i){
        len += strlen(m_argv[i]) + 1;
    }

    memset(m_argv[0], 0, len);
    m_argv[1] = nullptr;

    // 拷贝标题
    strncpy(m_argv[0], m_name.c_str(), m_name.size()); 
}
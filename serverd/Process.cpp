#include <string.h> // memset
#include <unistd.h> // fork
#include <stdlib.h> // environ
#include <stdio.h>

#include <tools/log/log.h>

#include "serverd/Process.h"

Process::Process(const std::string &name, char **argv, const ProcessLoop &loop) : 
    m_name(name),
    m_argv(argv),
    m_loop(std::move(loop))
{}

void Process::start(bool parentExit){
  m_pid = ::fork();
  if(m_pid == -1){
    LOG_SYSFATAL("fork error in Process::start(bool)");
  }
  else if(m_pid == 0){
    // [1] 设置不屏蔽任何信号
    clear_mask();

    // [2] 设置标题
    set_title();

    // [3] 进入工作循环
    m_loop(shared_from_this());
  }
  else{
    if(parentExit){
      exit(0);
    }
  }
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

void Process::set_mask(int signal){
  m_mask.add(signal);
  m_mask.update();
}

void Process::set_mask(const std::initializer_list<int> &signals){
  for(const int sig : signals){
    m_mask.add(sig);
  }

  m_mask.update();
}

void Process::clear_mask(){
  m_mask.clear();
  m_mask.update();
} 
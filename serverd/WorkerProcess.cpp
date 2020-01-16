#include <unistd.h>   // fork getpid

#include <tools/base/Exception.h>
#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>
#include <tools/log/log.h>
#include <tools/log/LogFile.h>

#include "serverd/WorkerProcess.h"

WorkerProcess::WorkerProcess(const std::string &name, char **argv, int eventfd, const WorkerLoop &loop) : 
  Process(name, argv)
{
  pid_t pid = ::fork();
  if(pid < 0){
    throw Exception("fork error");
  }
  else if(pid == 0){
    eventfd_wakeup(eventfd, static_cast<uint64_t>(::getpid()));

    clear_mask();

    set_title();

    ConfigFile &cf = Singleton<ConfigFile>::instance();
    if(cf.get<bool>("Daemon")){
      set_log();
    }

    loop();
  }
}

void WorkerProcess::eventfd_wakeup(int eventfd, uint64_t data){
  ssize_t n = ::write(eventfd, &data, sizeof data);
  if (n != sizeof data){
    throw Exception("wakeup error");
  }
}
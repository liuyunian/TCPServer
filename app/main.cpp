#include <iostream>
#include <memory>

#include <unistd.h> // getopt

#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>
#include <tools/log/log.h>
#include <tools/log/LogFile.h>

#include "proc/Process.h"

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
  char *configFilePath = const_cast<char*>("server.conf");
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

  auto master = std::make_shared<Process>("serverd(m)", argv, process::daemon_process_loop);
  master->start(1, true);

  return 0;
}
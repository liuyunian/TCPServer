#include <iostream>
#include <memory>

#include <unistd.h> // getopt

#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>
#include <tools/log/log.h>
#include <tools/log/LogFile.h>

#include "proc/Process.h"

#define CONFIG_FILE_PATH "server.conf"               // 默认的配置文件路径

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
    char *configFilePath = const_cast<char*>(CONFIG_FILE_PATH);
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

    std::string logFilePath = cf.get<std::string>("LogFile");
    logFilePath.append(::basename(argv[0]));
    LogFile lf(logFilePath.c_str(), 200*1000);
    Log::set_output(std::bind(&LogFile::append, &lf, std::placeholders::_1, std::placeholders::_2));
    Log::set_flush(std::bind(&LogFile::flush, &lf));

    master_process_loop(argv);

    return 0;
}
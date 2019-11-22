#include <iostream>
#include <memory>

#include <unistd.h> // getopt

#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>
#include <tools/log/log.h>
#include <tools/log/LogFile.h>

#define CONFIG_FILE_PATH "server.conf"               // 默认的配置文件路径

std::unique_ptr<LogFile> g_logFile;

void output_func(const char *msg, int len){
    g_logFile->append(msg, len);
}

void flush_func(){
    g_logFile->flush();
}

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

    std::string logFile = cf.get<std::string>("LogFile");
    logFile.append(::basename(argv[0]));

    g_logFile.reset(new LogFile(logFile.c_str(), 200*1000));
    Log::set_output(output_func);
    Log::set_flush(flush_func);

    LOG_INFO("LogFile = %s", cf.get<std::string>("LogFile").c_str());
    LOG_INFO("WorkProcess = %d", cf.get<int>("WorkProcess"));
    LOG_INFO("Daemon = %b", cf.get<bool>("Daemon"));

    return 0;
}
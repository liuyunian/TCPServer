#include <unistd.h> // fork sleep
#include <signal.h> // signal相关

#include <tools/log/log.h>
#include <tools/base/Singleton.h>
#include <tools/config/ConfigFile.h>

#include "proc/Process.h"

void master_process_loop(char **argv){
    // [1] 屏蔽信号，不希望在fork子进程时被信号打断
    sigset_t masterSigSet;
    ::sigemptyset(&masterSigSet);

    ::sigaddset(&masterSigSet, SIGCHLD);     // 子进程状态改变
    ::sigaddset(&masterSigSet, SIGALRM);     // 定时器超时
    ::sigaddset(&masterSigSet, SIGIO);       // 异步I/O
    ::sigaddset(&masterSigSet, SIGINT);      // 终端中断符
    ::sigaddset(&masterSigSet, SIGHUP);      // 连接断开
    ::sigaddset(&masterSigSet, SIGUSR1);     // 用户定义信号
    ::sigaddset(&masterSigSet, SIGUSR2);     // 用户定义信号
    ::sigaddset(&masterSigSet, SIGWINCH);    // 终端窗口大小改变
    ::sigaddset(&masterSigSet, SIGTERM);     // 终止
    ::sigaddset(&masterSigSet, SIGQUIT);     // 终端退出符

    if(::sigprocmask(SIG_SETMASK, &masterSigSet, NULL) < 0){
        LOG_SYSFATAL("Failed to shield signals in master_process_loop()");
    }

    // [2] fork worker进程
    std::string procName(::basename(argv[0]));
    procName.append("(w)");

    sigset_t workerSigSet;
    ::sigemptyset(&workerSigSet);

    ConfigFile &cf = Singleton<ConfigFile>::instance();
    int workProcessNum = cf.get<int>("WorkProcess");
    for(int i = 0; i < workProcessNum; ++ i){
        Process(procName, argv, workerSigSet, worker_process_loop);
    }

    // [3] 解除屏蔽的信号
    ::sigemptyset(&masterSigSet);
    if(::sigprocmask(SIG_SETMASK, &masterSigSet, NULL) < 0){
        LOG_SYSFATAL("Failed to unshield signals in master_process_loop()");
    }

    // [4] 进入工作循环
    for(;;){
        LOG_INFO("master process sleep 10ms");
        ::usleep(10 * 1000); 
    }
}

void worker_process_loop(){
    // 进入工作循环
    for(;;){
        LOG_INFO("worker process sleep 10ms");
        ::usleep(10 * 1000);
    }
}
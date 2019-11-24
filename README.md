# TCPServer
C++实现的TCP服务器框架  
采用master-worker + epoll + ThreadPool实现高并发

## 目录结构
* _include: 用于存放共用的头文件
* app: main.cpp所在的目录
* net: 存放与网络通信相关的代码
* proc: 存放与进程处理相关的代码
* signal: 存放与信号处理相关的代码

## 使用
### 依赖
1. 测试OS: Ubuntu 18.04 desktop
2. 编译器: gcc 7.4.0
3. 构建工具: cmake 3.10.2
4. 依赖库: [tools-cxx](https://github.com/liuyunian/tools-cxx)

### 编译
1. 构建: make
2. 安装: make install
3. 卸载: make uninstall
4. 清除: make clean

### 测试
* 编译完成未安装情况下: ./_build/TCPServer -h
* 已安装情况下: TCPServer -h

## 修改日志
**2019-11-16 目录结构和构建脚本**  
目录结构如上所示  
项目的编译工具为cmake

**2019-11-20 配置文件**  
采用tools-cxx的config库来解析项目中的配置文件[server.conf](/server.conf)

**2019-11-22 日志**  
采用tools-cxx的log多线程日志库  
日志信息写入日志文件中，日志文件的路径可在配置文件中指定，默认存储在项目根目录下的logs目录中

**2019-11-24 master-worker**  
master主进程创建worker子进程，worker进程数可以通过配置文件设置    
每个进程独占一个日志文件，日志文件名中有进程ID  
提供了修改进程标题(ps命令CMD栏显示的内容)的接口

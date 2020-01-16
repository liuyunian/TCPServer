#include <any>

#include <stdlib.h>   // getenv
#include <sys/stat.h> // stat
#include <fcntl.h>    // open
#include <unistd.h>   // close
#include <sys/mman.h> // mmap munmap

#include <tools/base/noncopyable.h>
#include <tools/base/Exception.h>
#include <tools/log/log.h>
#include <serverd/Buffer.h>
#include <serverd/Callbacks.h>
#include <serverd/TCPServer.h>
#include <serverd/TCPConnection.h>

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#define LISTEN_PORT 8080

class HttpServer : noncopyable {
public:
  HttpServer(char **argv, const InetAddress &listenAddr) : 
    m_server("httpd", argv, listenAddr)
  {
    m_server.set_connection_callback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    m_server.set_message_callback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
  }

  ~HttpServer() = default;

  void start(){
    LOG_INFO("HttpServer is running");
    m_server.start();
  }

  void onConnection(const TCPConnectionPtr &tcpc){
    LOG_INFO("onConnection(): pid = %d", getpid());
    tcpc->set_context(HttpContext());
  }

  void onMessage(const TCPConnectionPtr &tcpc, Buffer *buf){
    LOG_INFO("onMessage(): pid = %d, received %d bytes from connection", getpid(), buf->readable_bytes());
    HttpContext *context = std::any_cast<HttpContext>(tcpc->get_mutable_context());
    try{
      context->parse_request(buf->retrieve_all_as_string());
    }
    catch(const Exception &e){    
      LOG_ERR("%s", e.what());
      
      tcpc->send("HTTP/1.1 400 Bad Request\r\n\r\n");
      tcpc->shutdown();
    }

    try{
      onRequest(tcpc, context->get_request(), context->get_response());
    }
    catch(const Exception &e){
      tcpc->send("HTTP/1.1 500 Internal Server Error \r\n\r\n");
      tcpc->shutdown();
    }
    
    context->reset();
  }

private:
  void onRequest(const TCPConnectionPtr &tcpc, const HttpRequest &req, HttpResponse &res){
    const std::string &connection = req.get_header("Connection");
    bool close = connection == "close" || (req.get_version() == HttpRequest::Http10 && connection != "Keep-Alive");
    res.set_closeConnection(close);

    if(req.get_method() == HttpRequest::Get){
      process_get_request(req.get_path(), res);
      tcpc->send(res.to_string());
      if(res.get_closeConnection()){
        tcpc->shutdown();
      }
    }
    else {
      tcpc->send("HTTP/1.1 400 Bad Request\r\n\r\n");
      tcpc->shutdown();
    }
  }

  void process_get_request(std::string path, HttpResponse &res){
    if(path == "/"){
      path = "index.html";
    }

    std::string curPath(::getenv("PWD"));
    const char *staticResourcePath = curPath.append("/http/").append(path).c_str();
    struct stat sbuf;
    if(stat(staticResourcePath, &sbuf) < 0){
      res.set_stateCode(HttpResponse::NotFound);
      res.set_stateMessage("Not Found");
      res.set_closeConnection(true);
    }
    else{
      int srcfd = open(staticResourcePath, O_RDONLY, 0);
      if(srcfd < 0){
        throw Exception("open error");
      }

      res.set_stateCode(HttpResponse::Ok);
      res.set_stateMessage("OK");
      res.set_contentType(path.substr(path.find('.')));
      res.set_contentLength(sbuf.st_size);
      res.add_header("Server", "HttpServer");

      char *content = static_cast<char*>(::mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0));
      res.set_body(std::string(content, content+sbuf.st_size));
      ::munmap(content,sbuf.st_size);
      ::close(srcfd);
    }
  }

private:
  TCPServer m_server;
};

int main(int argc, char *argv[]){
  InetAddress listenAddr(LISTEN_PORT);
  HttpServer server(argv, listenAddr);
  server.start();

  return 0;
}
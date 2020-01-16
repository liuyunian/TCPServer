#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <string>
#include <sstream>
#include <map>

#include <tools/base/copyable.h>

static std::map<std::string, std::string> kMime = {
  {".html",   "text/html"},
  {".avi",    "video/x-msvideo"},
  {".bmp",    "image/bmp"},
  {".c",      "text/plain"},
  {".doc",    "application/msword"},
  {".gif",    "image/gif"},
  {".gz",     "application/x-gzip"},
  {".htm",    "text/html"},
  {".ico",    "image/x-icon"},
  {".jpg",    "image/jpeg"},
  {".png",    "image/png"},
  {".txt",    "text/plain"},
  {".mp3",    "audio/mp3"},
  {"default", "text/html"}
};

class HttpResponse : copyable {
public:
  enum StateCode {
    Unknow,
    Ok = 200,
    MovedPermanently = 301,
    BadRequest = 400,
    NotFound = 404,
    InternalServerError = 500
  };

  explicit HttpResponse() : m_stateCode(Unknow), m_closeConnection(false){}
  ~HttpResponse() = default;

  void set_stateCode(StateCode code){
    m_stateCode = code;
  }

  void set_stateMessage(const std::string &msg){
    m_stateMessage = msg;
  }

  void set_closeConnection(bool on){
    m_closeConnection = on;
  }

  bool get_closeConnection() const {
    return m_closeConnection;
  }

  void set_contentType(const std::string &contentType){
    add_header("Content-Type", kMime[contentType]);
  }

  void set_contentLength(size_t len){
    add_header("Content-Length", std::to_string(len));
  }

  void add_header(const std::string &key, const std::string &value){
    m_headers[key] = value;
  }

  void set_body(const std::string &body){
    m_body = body;
  }

  const std::string to_string(){
    std::ostringstream oss;
    oss << "HTTP/1.1 " << m_stateCode << " " << m_stateMessage << "\r\n";
    if(m_closeConnection){
      oss << "Connection: close\r\n";
    }
    else{
      oss << "Connection: Keep-Alive\r\n";
    }

    for (const auto& header : m_headers){
      oss << header.first << ": " << header.second << "\r\n";
    }

    oss << "\r\n" << m_body;

    return oss.str();
  }

  void swap(HttpResponse &that){
    m_headers.swap(that.m_headers);
    std::swap(m_stateCode, that.m_stateCode);
    std::swap(m_closeConnection, that.m_closeConnection);
    m_body.swap(that.m_body);
  }

private:
  std::map<std::string, std::string> m_headers;
  StateCode m_stateCode;
  std::string m_stateMessage;
  bool m_closeConnection;
  std::string m_body;
};

#endif // HTTPRESPONSE_H_
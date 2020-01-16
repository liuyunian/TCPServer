#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <string>
#include <map>

#include <assert.h>

#include <tools/base/copyable.h>
#include <tools/base/Exception.h>

class HttpRequest : copyable {
public:
  enum Method {
    Invalid,
    Get,
    Post,
  };

  enum Version {
    Unknow,
    Http10,
    Http11
  };

  HttpRequest() : m_method(Invalid), m_version(Unknow){}
  ~HttpRequest() = default;

  Method get_method() const {
    return m_method;
  }

  void set_method(const std::string &method){
    assert(m_method == Invalid);

    if(method == "GET"){
      m_method = Get;
    }
    else if(method == "POST"){
      m_method = Post;
    }
  }

  Version get_version() const {
    return m_version;
  }

  void set_version(const std::string &version){
    assert(m_version == Unknow);

    if(version == "HTTP/1.1"){
      m_version = Http11;
    }
    else if(version == "HTTP/1.0"){
      m_version = Http10;
    }
  }

  const std::string& get_path() const {
    return m_path;
  }

  void set_path(const std::string &path){
    m_path.assign(path);
  }

  void add_header(const std::string &key, const std::string &value){
    m_headers.insert({key, value});
  }

  std::string get_header(const std::string &key) const {
    auto iter = m_headers.find(key);
    if(iter == m_headers.end()){
      throw Exception("get_header error");
    }

    return iter->second;
  }

  const std::map<std::string, std::string>& headers() const {
    return m_headers;
  }

  void swap(HttpRequest &that){
    std::swap(m_method, that.m_method);
    std::swap(m_version, that.m_version);
    m_path.swap(that.m_path);
    m_headers.swap(that.m_headers);
  }

private:
  Method m_method;
  std::string m_path;
  Version m_version;
  std::map<std::string, std::string> m_headers;
};

#endif // HTTPREQUEST_H_
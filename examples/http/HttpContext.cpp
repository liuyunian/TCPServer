#include <tools/base/Exception.h>
#include <tools/log/log.h>

#include "HttpContext.h"

void HttpContext::parse_request(const std::string &buf){
  int pos = 0; // 记录解析的位置
  std::string content;

  /*
   * parse http request line
    GET / HTTP/1.1
  */
  m_req.set_method(get_content(buf, pos, " "));
  m_req.set_path(get_content(buf, pos, " "));
  m_req.set_version(get_content(buf, pos, "\r\n"));

  /*
   * parse http header line
    Host: localhost:10000
    Connection: keep-alive
    Cache-Control: max-age=0
    Accept-Language: zh-CN,zh;q=0.9
  */
  std::string key, value;
  while(buf[pos] != '\r' && buf[pos+1] != '\n'){
    key = get_content(buf, pos, ": ");
    value = get_content(buf, pos, "\r\n");
    m_req.add_header(key, value);
	}
}

std::string HttpContext::get_content(const std::string &buf, int &pos, const std::string &sepa){
  int index = buf.find(sepa, pos);
  if(index == std::string::npos){
    throw Exception("parse errno");
  }

  std::string content(buf.substr(pos, index-pos));
  pos = index + sepa.size();
  return content;
}
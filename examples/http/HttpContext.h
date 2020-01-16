#ifndef HTTPCONTEXT_H_
#define HTTPCONTEXT_H_

#include <tools/base/copyable.h>

#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpContext : copyable {
public:
  HttpContext(){}
  ~HttpContext() = default;

  const HttpRequest& get_request() const {
    return m_req;
  }

  HttpResponse& get_response(){
    return m_res;
  }

  void reset(){
    HttpRequest dummyReq;
    m_req.swap(dummyReq);

    HttpResponse dummyRes;
    m_res.swap(dummyRes);
  }

  void parse_request(const std::string &buf);

private:
  std::string get_content(const std::string &buf, int &pos, const std::string &sepa);

private:
  HttpRequest m_req;
  HttpResponse m_res;
};

#endif // HTTPCONTEXT_H_
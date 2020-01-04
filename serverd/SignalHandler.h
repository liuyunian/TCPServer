#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_

#include <functional>

#include <tools/base/noncopyable.h>

#include "serverd/SignalMask.h"

class SignalHandler : noncopyable {
public:
  SignalHandler(SignalMask &set);
  ~SignalHandler();

  typedef std::function<void()> SignalCallback;
  void register_signal(int signo, SignalCallback cb);

  void unregister_signal(int signo);

  void wait();

private:
  void update();

private:
  int m_sigfd;
  SignalMask &m_mask;
  std::map<int, SignalCallback> m_signalCallbacks;
};

#endif // SIGNALHANDLER_H_
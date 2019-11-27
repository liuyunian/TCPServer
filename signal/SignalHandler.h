#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_

#include "signal/SignalMask.h"

#include <tools/base/noncopyable.h>

namespace Signal {

  void get_status(int signo);
  
}; // namespace Signal

class SignalHandler : noncopyable {
public:
  SignalHandler(SignalMask &set);
  ~SignalHandler() = default;

  typedef void(*SignalCallback)(int);
  void register_signal(int signo, SignalSet set, SignalCallback cb);

  void unregister_signal(int signo);

private:
  void update(int signo, SignalSet set, SignalCallback cb);

private:
  SignalMask &m_mask;
};

#endif // SIGNALHANDLER_H_
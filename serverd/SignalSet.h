#ifndef SIGNALSET_H_
#define SIGNALSET_H_

#include <map>
#include <functional>

#include <signal.h>

#include <tools/base/copyable.h>

// 信号集
class SignalSet : copyable {
public:
  SignalSet(sigset_t set);

  ~SignalSet() = default;

  sigset_t get() const {
    return m_sigset;
  }

  void add(int signo);

  void remove(int signo);

  void clear();

  void fill();

  bool contain(int signo);

  static SignalSet get_empty_sigset();

  static SignalSet get_fill_sigset();

protected:
  sigset_t m_sigset;
};

#endif // SIGNALSET_H_
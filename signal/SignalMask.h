#ifndef SIGNALMASK_H_
#define SIGNALMASK_H_

#include <tools/base/noncopyable.h>

#include "signal/SignalSet.h"

class SignalMask : noncopyable, 
                  public SignalSet
{
public:
  SignalMask();
  ~SignalMask() = default;

  void update();
};

#endif // SIGNALMASK_H_
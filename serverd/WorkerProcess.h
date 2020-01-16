#ifndef WORKERPROCESS
#define WORKERPROCESS

#include "serverd/Process.h"
#include "serverd/Callbacks.h"

class WorkerProcess : public Process
{
public:
  WorkerProcess(const std::string &name, char **argv, int eventfd, const WorkerLoop &loop);
  ~WorkerProcess() = default;

private:
  void eventfd_wakeup(int eventfd, uint64_t data);
};

#endif // WORKERPROCESS
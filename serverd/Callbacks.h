#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include <memory>
#include <functional>

class Buffer;
class TCPConnection;
typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;
typedef std::function<void(const TCPConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TCPConnectionPtr&, Buffer*)> MessageCallback;
typedef std::function<void(const TCPConnectionPtr&)> CloseCallback;
typedef std::function<void(const TCPConnectionPtr&)> WriteCompleteCallback;         // 低水位标回调函数
typedef std::function<void(const TCPConnectionPtr&, size_t)> HighWaterMarkCallback; // 高水位标回调函数

typedef std::function<void()> WorkerLoop;

#endif // CALLBACKS_H_
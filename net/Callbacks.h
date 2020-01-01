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

#endif // CALLBACKS_H_
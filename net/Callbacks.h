#ifndef CALLBACKS_H_
#define CALLBACKS_H_

#include <memory>
#include <functional>

class TCPConnection;
typedef std::shared_ptr<TCPConnection> TCPConnectionPtr;
typedef std::function<void(const TCPConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TCPConnectionPtr&, const char*, ssize_t)> MessageCallback;
typedef std::function<void(const TCPConnectionPtr&)> CloseCallback;

#endif // CALLBACKS_H_
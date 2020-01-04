#include <string.h>     // strlen

#include <tools/socket/Endian.h>
#include <tools/socket/ConnSocket.h>
#include <tools/socket/SocketsOps.h>
#include <tools/socket/InetAddress.h>

#define PORT 9000
#define BUFFER_SZ 1024

int main(){
  int clientfd = sockets::create_socket();
  InetAddress addr(PORT, InetAddress::IPv4, true);
  sockets::connect(clientfd, addr.get_sockaddr());

  char buf[BUFFER_SZ] = {0};
  while(fgets(buf, BUFFER_SZ, stdin) != nullptr){
    sockets::write(clientfd, buf, strlen(buf));
    memset(buf, 0, BUFFER_SZ);

    sockets::read(clientfd, buf, sizeof(buf));
    fputs(buf, stdout);
    memset(buf, 0, BUFFER_SZ);
  }

  sockets::close(clientfd);

  return 0;
}
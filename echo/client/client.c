#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>

int BUFLEN = 1024;

int connectTCP(char* ip, int port);
int connectsock(char* ip, int port, const char* transport);
void TCPecho(int s);

int main(int argc, char* argv[]) {
  char* ip = "127.0.0.1";
  int port = 5000;

  switch(argc) {
    case 1: break;
    case 3: port = atoi(argv[2]);
    case 2: ip = argv[1];
    default: break;
  }
  int s = connectTCP(ip, port);
  TCPecho(s);
  close(s);
  return 0;
}

int connectTCP(char* ip, int port) {
  return connectsock(ip, port, "tcp");
}

int connectsock(char* ip, int port, const char* transport) {
  int type;
  if(strcmp(transport, "udp") == 0) type = SOCK_DGRAM;
  else type = SOCK_STREAM;

  int s = socket(PF_INET, type, 0);
  if(s < 0) printf("创建套接字失败\n");
  else printf("创建套接字成功\n");

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(ip);
  sin.sin_port = htons(port);

  // UDP也可以使用connect函数
  if(connect(s, (struct sockaddr*)&sin, sizeof(sin))<0) {
    printf("服务端连接失败，请检查你的网络\n");
    exit(1);
  }
  else printf("服务端连接成功\n");
  return s;
}

void TCPecho(int s) {
  char msg[BUFLEN];
  while(1) {
    // 发送数据
    memset(&msg, 0, BUFLEN);
    scanf("%s", msg);
    write(s, msg, strlen(msg));
    if(strcmp(msg, "exit") == 0) break;

    // 接收数据
    memset(&msg, 0, BUFLEN);
    if(read(s, msg, BUFLEN) < 0) printf("Read error.\n");
    else printf("echoed: %s\n", msg);
  }
  printf("关闭客户端\n");
}
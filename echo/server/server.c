#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

int BUFLEN = 1024;

typedef struct client_info {
  int ssock;
  struct sockaddr_in cin;
} client;

int createMainSock(int port, int qlen) {
  int s = socket(PF_INET, SOCK_STREAM, 0);
  if(s < 0) printf("创建套接字失败\n");
  else printf("创建套接字成功\n");

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  if(bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) printf("bind失败\n");

  if(listen(s, qlen) < 0) printf("监听失败\n");
  else printf("在%d端口上进行监听\n", ntohs(sin.sin_port));

  return s;
}

client createSlaveSock(int s) {
  struct sockaddr_in cin;
  socklen_t cin_size = sizeof(cin);
  printf("主进程等待客户端的连接\n");
  int ssock = accept(s, (struct sockaddr*)&cin, &cin_size);

  if(ssock < 0) printf("连接失败\n");
  else printf("%s:%d连接成功\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));

  client cinfo = {ssock, cin};

  return cinfo;
}

void TCPechoed(int ssock) {
  char buf[BUFLEN];
  while(strcmp(buf, "exit") != 0) {
    memset(&buf, 0, BUFLEN);
    if(read(ssock, buf, BUFLEN) < 0) printf("Read error.\n");
    else if(strcmp(buf, "exit") != 0) {
      printf("echo: %s\n", buf);
      write(ssock, buf, sizeof(buf));
    }
  }
}

int main() {
  int port = 5000, qlen = 4;

  int s = createMainSock(port, qlen);

  while(1) {
    client cinfo = createSlaveSock(s);

    switch(fork()) {
      case 0:  // 子进程
        close(s);
        TCPechoed(cinfo.ssock);
        printf("关闭与%s:%d的连接\n", inet_ntoa(cinfo.cin.sin_addr), ntohs(cinfo.cin.sin_port));
        exit(0);
      default: // 都要执行
        close(cinfo.ssock);
        break;
      case -1: printf("fork失败\n");
    }
  }
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>

int BUFLEN = 1024;
int flag = 0;

int connectTCP(char* ip, int port);
int connectsock(char* ip, int port, const char* transport);
void TCPchat(int s);

void sendMsg();
void recvMsg();
void sendFile();
void revcFile();
void* mySend(void* sp);
void* myRecv(void* sp);

int main(int argc, char* argv[]) {
  char* ip = "127.0.0.1";
  int port = 5000;

  int s = connectTCP(ip, port);
  TCPchat(s);
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

void* mySend(void* sp) {
  int* s = (int*)sp;
  char msg[BUFLEN];
  while(!flag) {
    memset(msg, 0, BUFLEN);
    printf("\n请输入你要发送的消息：");
    scanf("%s", msg);
    write(*s, msg, strlen(msg));
    if(strcmp(msg, "exit") == 0) break;  // 我方主动退出
    printf("发送成功！\n");
  }
}

void* myRecv(void* sp) {
  int* s = (int*)sp;
  char msg[BUFLEN];
  while(!flag) {
    memset(&msg, 0, BUFLEN);
    read(*s, msg, BUFLEN);
    if(strcmp(msg, "exit") != 0) printf("\n收到消息: %s\n", msg);
    else {
      printf("对方退出\n");break;  // 对方退出，我方也退出
    }
  }
}

void TCPchat(int s) {
  char client[1];
  printf("等待对方在线\n");
  read(s, client, sizeof(client));
  printf("已创建会话\n");
  
  pthread_t th1, th2;
  pthread_create(&th1, NULL, mySend, (void*)&s);
  pthread_create(&th2, NULL, myRecv, (void*)&s);
  pthread_join(th1, NULL);
  pthread_join(th2, NULL);

  printf("关闭客户端\n");
}
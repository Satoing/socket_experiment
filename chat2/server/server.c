/* 使用两个线程分别进行两个客户端消息的收发 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>

int BUFLEN = 1024;

typedef struct client_info {
  int ssock;
  struct sockaddr_in cin;
} client;

typedef struct chat_group {
  client entity1;
  client entity2;
  int flag;
} group;

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

// 还需要实现的功能是超时断开与client1的连接
client waitJoin(int s) {
  struct sockaddr_in cin;
  socklen_t cin_size = sizeof(cin);
  printf("等待第二个客户端的连接\n");
  int ssock = accept(s, (struct sockaddr*)&cin, &cin_size);

  if(ssock < 0) printf("连接失败\n");
  else printf("%s:%d连接成功\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));

  client cinfo = {ssock, cin};

  return cinfo;
}

void* read_and_write_1(void* v_chat_two) {
  char buf[BUFLEN];
  group* chat_two = (group*)v_chat_two;
  while(strcmp(buf, "exit") != 0 && chat_two->flag != 1) {
    // client1向client2发送消息
    memset(buf, 0, BUFLEN);
    read(chat_two->entity1.ssock, buf, BUFLEN);
    write(chat_two->entity2.ssock, buf, sizeof(buf));
    if(strcmp(buf, "exit") != 0) {
      printf("client1发送的消息: %s\n", buf);
    } else {
      chat_two->flag = 1;
      break;
    }
  }
  return NULL;
}

void* read_and_write_2(void* v_chat_two) {
  char buf[BUFLEN];
  group* chat_two = (group*)v_chat_two;
  while(strcmp(buf, "exit") != 0 && chat_two->flag != 1) {
    // client1向client2发送消息
    memset(buf, 0, BUFLEN);
    read(chat_two->entity2.ssock, buf, BUFLEN);
    write(chat_two->entity1.ssock, buf, sizeof(buf));
    if(strcmp(buf, "exit") != 0) {
      printf("client2发送的消息: %s\n", buf);
    } else break;
  }
  return NULL;
}

void TCPchated(group chat_two) {
  write(chat_two.entity1.ssock, "1", 1);
  write(chat_two.entity2.ssock, "2", 1);
  pthread_t th1, th2;
  printf("DEBUG:已发送消息，唤醒客户端\n");
  pthread_create(&th1, NULL, read_and_write_1, (void*)&chat_two);
  pthread_create(&th2, NULL, read_and_write_2, (void*)&chat_two);
  printf("DEBUG:已创建线程\n");
  pthread_join(th1, NULL);
  pthread_join(th2, NULL);
}

int main() {
  int port = 5000, qlen = 4;

  int s = createMainSock(port, qlen);

  while(1) {
    group chat_two;
    chat_two.entity1 = createSlaveSock(s);
    chat_two.entity2 = waitJoin(s);
    chat_two.flag = 0;

    int fd = fork();
    if(fd == 0) {
      close(s);
      printf("DEBUG:开始聊天\n");
      TCPchated(chat_two);
      printf("关闭连接\n");
      close(chat_two.entity1.ssock);
      close(chat_two.entity2.ssock);
      break;
    }
    else if(fd == -1) printf("fork失败\n");

    close(chat_two.entity1.ssock);
    close(chat_two.entity2.ssock);
  }
  return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>

#define UNIXEPOCH 2208988800UL

int createMainSock(int port) {
  int s = socket(PF_INET, SOCK_DGRAM, 0);
  if(s < 0) printf("创建套接字失败\n");
  else printf("创建套接字成功\n");

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);
  if(bind(s, (struct sockaddr*)&sin, sizeof(sin)) < 0) printf("bind失败\n");
  else printf("绑定到%d端口\n", ntohs(sin.sin_port));

  return s;
}

int main() {
  int port = 5000;
  int s = createMainSock(port);

  struct sockaddr_in cin;
  int cin_len = sizeof(cin);
  char buf[1];
  time_t now;

  while(1) {
    memset(buf, 0, sizeof(buf));
    recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&cin, &cin_len);
    printf("%s", buf);

    time(&now);
    now = htonl((unsigned long)(now+UNIXEPOCH));
    sendto(s, &now, sizeof(now), 0, (struct sockaddr*)&cin, sizeof(cin));
    printf("已向%s:%d发送时间\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port));
  }
  return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>

#define MSG "What time is it?"
#define UNIXEPOCH 2208988800UL

int connectsock(char* ip, int port) {
  int s = socket(PF_INET, SOCK_DGRAM, 0);
  if(s < 0) printf("创建套接字失败\n");
  else printf("创建套接字成功\n");

  return s;
}

int connectUDP(char* ip, int port) {
  return connectsock(ip, port);
}

int main() {
  char* ip = "127.0.0.1";
  int port = 5000;
  time_t now;

  int s = connectUDP(ip, port);

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(ip);
  sin.sin_port = htons(port);
  int sin_len = sizeof(sin);

  sendto(s, MSG, sizeof(MSG), 0, (struct sockaddr*)&sin, sizeof(sin));

  int n = recvfrom(s, &now, sizeof(now), 0, (struct sockaddr*)&sin, &sin_len);
  if(n < 0) {
    printf("读取失败\n");
    exit(1);
  }

  now = ntohl((unsigned long)now);
  now -= UNIXEPOCH;
  printf("%s", ctime(&now));
  return 0;
}
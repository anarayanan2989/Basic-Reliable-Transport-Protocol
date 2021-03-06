#include "../rsocket.c"

int main(int argc,char **argv){
  int sk;
  char msg[MTU];
  int count = 0;
  struct sockaddr_in serverinfo;

  sk = r_socket(PF_INET, SOCK_BRP,0);

  serverinfo.sin_family = AF_INET;
  serverinfo.sin_port = htons(atoi(argv[2]));
  serverinfo.sin_addr.s_addr = inet_addr(argv[1]);
  memset(serverinfo.sin_zero, '\0', sizeof serverinfo.sin_zero);  

  while(1){
    int send;
    if(count == 0){
      printf("enter a string to send\n");
    }
      scanf("%s",msg);
    ++count;
    send = r_sendto(sk,msg,strlen(msg),0,(struct sockaddr *)&serverinfo,sizeof(serverinfo));
    printf("\nsend complete \n");
    if(send == 0){
      printf("\nThe entire message has been sent");
      count = 0;
    }
  }
  return 0;
}

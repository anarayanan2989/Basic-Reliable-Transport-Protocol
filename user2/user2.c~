#include "../rsocket.c"


int main(int argc,char **argv){
  int sk;
  char msg[MTU];
  struct sockaddr_in serverinfo;
  struct sockaddr_storage skstorage;
  socklen_t addrlen;

  sk = r_socket(PF_INET, SOCK_BRP, 0);

  serverinfo.sin_family = AF_INET;
  serverinfo.sin_port = htons(atoi(argv[1]));
  serverinfo.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverinfo.sin_zero, '\0', sizeof serverinfo.sin_zero);  
  r_bind(sk, (struct sockaddr *) &serverinfo, sizeof(serverinfo));

  while(1){
   
    r_recvfrom(sk,msg,1,0,(struct sockaddr *)&skstorage, &addrlen);
    //printf("\nChar received: %c ",msg[0]);
    
  
  }
  return 0;
}

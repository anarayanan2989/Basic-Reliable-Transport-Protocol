#include "rsocket.h"

int r_sendto(int sk,char *data,int len,int flag,struct sockaddr *serverinfo,int addrlen);
void *rthread(void *param);
void *sthread(void *param);
void creat_R_S_thread(struct conn_params *temp);


struct conn_params *head_node=NULL;
int tot_conns = 0;

int dropMessage(float p){
 float random=((float)(rand()%100)/100);
 return random<p?1:0;
}


void *rthread(void *param)
{
  struct conn_params *temp = (struct conn_params *)param;
  char data[MTU];
  int retlen;
  struct rx_data *recvbuf;
  struct rx_data *temprx = NULL;
  struct tx_data *temptx = NULL;
  int addrlen = sizeof(struct sockaddr);
  int ctr =0;
  while(1){
    retlen = recvfrom(temp->sk,data,MTU,0,&temp->serverinfo,&addrlen);
    if(dropMessage(prob_drop) || (retlen < sizeof(struct rx_data))){
 	recvbuf = (struct rx_data *)data;
	if(recvbuf->pkt_type == 1){
		printf("\nDropped Packet-Type: Data\t");
	}
	else{
		printf("\nDropped Packet-Type: ACK\t");
	}
	printf("Sequence Number: %d\t",recvbuf->sequence_num);
	printf("Content: %c \n",recvbuf->data);
      continue;
    }
    else{
	recvbuf = (struct rx_data *)data;
	if(recvbuf->pkt_type == 1)
	  print(recvbuf->data);
    }
    recvbuf = (struct rx_data *)data;


    switch(recvbuf->pkt_type){
      case 1:
        temprx = temp->rec_msg_tbl;
	ctr=0;
	while(temprx->pkt_type != 0){
	  temprx++;
	  ctr++;
	  if(ctr>MAX_BUF){
            printf("recv buffer full\n");
	    break;
	  }
	}
	if(ctr>MAX_BUF)
	  continue;
        memcpy(temprx,recvbuf,sizeof(struct rx_data));
        recvbuf->pkt_type=ackpkt;
        sendto(temp->sk,data,sizeof(struct rx_data),0,&temp->serverinfo, sizeof(struct sockaddr));
        break;

      case 2:
        temptx = temp->unack_msg_tbl;
	ctr=0;
        while(ctr < MAX_BUF){
	  if(temptx->sequence_num == recvbuf->sequence_num){
            break;
          }
          temptx++;
	  ctr++;
        }
	if(ctr>MAX_BUF)
          continue;
        memset(temptx,0,sizeof(struct tx_data));
	break;

      default:
	break;
    }
  }
  return NULL;
}

void *sthread(void *param)
{
  struct conn_params *temp = (struct conn_params *)param;
  struct rx_data *recvbuf;
  struct tx_data *temptx = NULL;
  int ctr;
  ctr=0;
  while(1){
    sleep(1);
    temptx = temp->unack_msg_tbl;
    ctr=0;
    while(ctr < MAX_BUF){
      if(temptx->pkt_type){
        temptx->timeout++;
        if(temptx->timeout > timeout_int){
	  temptx->timeout=0;
	  sendto(temp->sk,(char *)temptx,sizeof(struct rx_data),0,&temp->serverinfo, sizeof(struct sockaddr));
	/*if(temptx->data != '\0'){
	  printf("\n Time Out!!! \n")*/
	  printf("\n Retransmitting char: %c \n", temptx->data);
	  //printf("\t%d",count);
	}
      }
      ctr++;
      temptx++;
    }
  }
  return NULL;
}


struct conn_params *get_free_node(void){
  struct conn_params *temp = head_node;
  while(temp){
    if(!temp->next)
      break;
    temp = temp->next;
  }
  if(temp){
    temp->next=(struct conn_params *)malloc(1*sizeof(struct conn_params));
    temp = temp->next;
  }else{
    temp=(struct conn_params *)malloc(1*sizeof(struct conn_params));
    head_node=temp;
  }
  memset(temp,0,sizeof(struct conn_params));
  return temp;  
}



void creat_R_S_thread(struct conn_params *temp){
	pthread_create(&temp->Rth,NULL,rthread,(char *)temp);
        pthread_create(&temp->Sth,NULL,sthread,(char *)temp);
}

void initialise_buffers(struct conn_params *temp){
	temp->rec_msg_tbl   =  (struct rx_data *)malloc(MAX_BUF * sizeof(struct rx_data));
	temp->unack_msg_tbl =  (struct tx_data *)malloc(MAX_BUF * sizeof(struct tx_data));

}

int r_socket(int proto,int socket_type,int flag){
  struct conn_params *temp;
  if(socket_type == SOCK_BRP){
    temp = get_free_node(); 
    temp->sk = socket(proto, SOCK_DGRAM, flag);
    initialise_buffers(temp);
    creat_R_S_thread(temp);
  } else {
    return -1;
  }
  return temp->sk;
}



int r_bind (int sk, struct sockaddr * serverinfo,int len){
  struct conn_params *temp = head_node;
  while(temp){
    if(temp->sk == sk){
      memcpy(&temp->serverinfo,serverinfo,len);
      return bind(sk,serverinfo,len);
    }
    temp = temp->next;
  }
}


int r_sendto(int sk,char *data,int len,int flag,struct sockaddr *serverinfo,int addrlen){
  struct conn_params *temp = head_node;
  char *ch = data;
  int i=len,j=0;
  struct tx_data *temptx = NULL;
  while(temp){
    if(temp->sk == sk){
      break;
    }
    temp = temp->next;
  }
  memcpy(&temp->serverinfo,serverinfo,addrlen);
  //len = strlen(data);
  if(len > MTU){
    printf("buffer too large\n");
    return;
  }
  temptx = temp->unack_msg_tbl;
  while(j<i){
    while(temptx->pkt_type)
	temptx++;
    temptx->pkt_type = datapkt;
    temptx->data = data[j];
    temptx->sequence_num = temp->seq;
    temp->seq++;
    sendto(temp->sk,(char *)temptx,sizeof(struct rx_data),0,&temp->serverinfo, sizeof(struct sockaddr));
    j++;
  }
  while(1){
    sleep(1);
    temptx = temp->unack_msg_tbl;
    for(j=0;j<i;j++){
      if(temptx->pkt_type){
	break;
      }
      temptx++;
    }
    if(j==i)
	break;
  }
  return(i);

}

int r_recvfrom(int sk,char *data,int len,int flag,struct sockaddr *serverinfo,int *addrlen){
  struct conn_params *temp = head_node;
  struct rx_data *temprx = NULL;
  int i = len;
  int ctr = 0;
  char *ch =data;
  while(temp){
    if(temp->sk == sk){
      break;
    }
    temp = temp->next;
  }
  //memcpy(&(temp->serverinfo),(void)serverinfo,*addrlen);
  while(1){
    //sleep(1);
    temprx = temp->rec_msg_tbl;
    ctr=0;
    while((ctr < MAX_BUF)&&i){
      if(temprx-> pkt_type == 1){
        *ch = temprx->data;
	ch++;
	memset(temprx,0,sizeof(struct rx_data));
        i--;
      }
      temprx++;
      ctr++;
    }
    if(i!=len){
	//printf("i %d  %d  %c \n",i,len,*data);
	return(len-i); 
    }
    else
      return 0;
  }
}


int r_close(int sk){
  struct conn_params *temp = head_node;
  while(temp){
    if(temp->sk == sk){
      break;
    }
    temp = temp->next;
  }
  free(temp);
  return close(sk);
}

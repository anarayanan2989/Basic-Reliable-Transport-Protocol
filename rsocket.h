#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define SOCK_BRP 1234
#define timeout_int 2
#define prob_drop 0.35
#define MTU 512
#define datapkt 0x01
#define ackpkt 0x02
#define MAX_BUF 50


struct rx_data{
 char pkt_type;
 char data;
 int sequence_num;
};

void print(char ch);

struct tx_data{
 char pkt_type;
 char data;
 int sequence_num;
 int timeout;
 int lock;
 struct tx_data *next;
};

struct conn_params{
 int sk;
 struct sockaddr serverinfo;
 pthread_t Rth;
 pthread_t Sth;
 int seq;
 struct rx_data *rec_msg_tbl;
 struct tx_data *unack_msg_tbl;
 struct conn_params *next;
};

void print(char ch){
	printf("\nCharacter received: %c\n",ch);
}

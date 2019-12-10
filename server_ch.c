#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
/*typedef struct acc_info acc
{
	int socknum;
	char name[10];
	int pwd;
	int islog;
};*/
int socknum[10]={-1};
int islog[10]={-1};
int invited[10] ={-1};
int by[10]={-1};
int ht[10] ={-1};
int hosting[10] ={-1};
struct client_info {
	int sockno;
	char ip[INET_ADDRSTRLEN];
};
int clients[100];
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void sendtoall(char *msg,int curr)
{
	int i;
	pthread_mutex_lock(&mutex);
	for(i = 0; i < n; i++) {
		if(clients[i] != curr) {
			if(send(clients[i],msg,strlen(msg),0) < 0) {
				perror("sending failure");
				continue;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}
void sendinv(int host, int guest)
{
	
}
void *recvmg(void *sock)
{
	struct client_info cl = *((struct client_info *)sock);
	char msg[500];
	int len;
	int i;
	int j;
	while((len = recv(cl.sockno,msg,500,0)) > 0) {
		msg[len] = '\0';
		printf("msg = %s\n",msg);
		//sendtoall(msg,cl.sockno);
		if(strncmp(msg,":login:",7) == 0)// if login
		{	
			int gnum = msg[12] -'0';
			if(gnum >=0 && gnum <=5)
			{
				printf("system: user try to login\n");
				if(islog[gnum] == 1)
				{
					send(cl.sockno,"someone has login as this guest\n",strlen("someone has login as this guest\n"),0);	
				}
				else if(strncmp(msg+6,":guest",6) == 0)
				{
					char str[10]="@@@?\n";
					str[3] = gnum + '0';
					printf("system : guest%d has login\n",gnum);
					socknum[gnum] = cl.sockno;
					islog[gnum] = 1;
					send(cl.sockno,str,strlen(str),0);
					send(cl.sockno,"you have log in\n",strlen("you have log in\n"),0);
									
				}
			}
			else
			{	
				send(cl.sockno,"no such account\n",strlen("no such account\n"),0);
			}
				
		}
		if(strncmp(msg+1,"list:",5) == 0)
		{
			send(cl.sockno,"\n-----online users-----\n",strlen("\n-----online users-----\n"),0);
			
			for(int i = 0; i <10; i++)
			{
				char str[10] = "guest?\n";
				if(islog[i] == 1)
					{	
						str[5] = i + '0';		
						send(cl.sockno,str,strlen(str),0);
					}	
						
			}
			send(cl.sockno,"----------------------\n",strlen("----------------------\n"),0);
					
		}
		if(strncmp(msg+1,"logout:",7) == 0) //logout
		{
			int hnum = msg[0] -'0'; 
			if(hnum >=0 && hnum <=5)
			{
				printf("system: user try to logout\n");
				if(islog[hnum] == 1)
				{
					islog[hnum] = -1;
					socknum[hnum] = -1;
					send(cl.sockno,"!!@@!!",strlen("!!@@!!"),0);
				}
			}
		}
		if(strncmp(msg+1,"inv:",4) == 0) //invite
		{
			int hnum = msg[0] -'0'; 
			int gnum = msg[10] -'0';
			if(gnum >=0 && gnum <=5)
			{
				printf("system: user try to invite\n");
				if(islog[gnum] == 1)
				{
					char str[30] = "&&&&&";
					//char str[30] ="guest? invite you to play\n";
					char strr[30] = "you invite guest? to play\n";
					str[5] = hnum + '0';
					strr[16] = gnum +'0';
					invited[gnum] = 1;
					by[gnum] = hnum;
					hosting[hnum] = 1;
					ht[hnum] = gnum;
					send(socknum[gnum],str,strlen(str),0);
					send(cl.sockno,strr,strlen(strr),0);
				}
				else
				{
					send(cl.sockno,"the person you invite is not online\n",strlen("the person you invite is not online\n"),0);
				}
						/*else if(strncmp(msg+6,":guest",6) == 0)
				{
					
								
				}*/
		
			}
			else
			{	
				send(cl.sockno,"no such account\n",strlen("no such account\n"),0);
			}
		}
		if(strncmp(msg+1,"join:",5) == 0) //join
		{
			int gnum = msg[0]-'0';
			int hnum = by[gnum];
			if(invited[gnum])
				{
					send(cl.sockno,"_JOIN",strlen("_JOIN"),0);
					send(socknum[hnum],"_JOIN",strlen("_JOIN"),0);
					invited[gnum] = -1;
					by[gnum]=-1;
					ht[gnum] =-1;
					hosting[hnum] =-1;			
				}
			else
				send(cl.sockno,"you are not being invited\n",strlen("you are not being invited\n"),0);
		}
		if((strncmp(msg+1,"nope:",5) == 0))//nope
		{
			int gnum = msg[0]-'0';
			int hnum = by[gnum];
			if(invited[gnum])
				{
					send(socknum[hnum],"the other player dose not accept to paly\n",strlen("the other player dose not accept to paly\n"),0);
					 invited[gnum] = -1;
					 by[gnum]=-1;
					 ht[gnum] =-1;
					 hosting[hnum] =-1;
				}
			else
				send(cl.sockno,"you are not being invited\n",strlen("you are not being invited\n"),0);
		}
		memset(msg,'\0',sizeof(msg));
	}
	pthread_mutex_lock(&mutex);
	printf("%s disconnected\n",cl.ip);
	for(i = 0; i < 10; i++) {
		if(socknum[i] == cl.sockno) {
		socknum[i] = -1;
		islog[i] = -1;
	}
	}
	
	for(i = 0; i < n; i++) {
		if(clients[i] == cl.sockno) {
			j = i;
			while(j < n-1) {
				clients[j] = clients[j+1];
				j++;
			}
		}
	}
	n--;
	pthread_mutex_unlock(&mutex);
}
int main(int argc,char *argv[])
{
	struct sockaddr_in my_addr,their_addr;
	int my_sock;
	int their_sock;
	socklen_t their_addr_size;
	int portno;
	pthread_t sendt,recvt;
	char msg[500];
	int len;
	struct client_info cl;
	char ip[INET_ADDRSTRLEN];;
	;
	if(argc > 2) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[1]);
	my_sock = socket(AF_INET,SOCK_STREAM,0);
	memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(portno);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr_size = sizeof(their_addr);

	if(bind(my_sock,(struct sockaddr *)&my_addr,sizeof(my_addr)) != 0) {
		perror("binding unsuccessful");
		exit(1);
	}

	if(listen(my_sock,5) != 0) {
		perror("listening unsuccessful");
		exit(1);
	}

	while(1) {
		if((their_sock = accept(my_sock,(struct sockaddr *)&their_addr,&their_addr_size)) < 0) {
			perror("accept unsuccessful");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
		printf("%s connected\n",ip);
		cl.sockno = their_sock;
		strcpy(cl.ip,ip);
		clients[n] = their_sock;
		printf("system : clients[%d]=%d\n",n,clients[n]);
		n++;
		pthread_create(&recvt,NULL,recvmg,&cl);
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}

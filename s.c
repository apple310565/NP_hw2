#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

int sockfd;
int fds[100];
int size =100 ;
char* IP = "192.168.10.143";
short PORT = 10222;
typedef struct sockaddr SA;
char* account[105];
int authe(char buf[])
{
	FILE *fp;
	char tmp1[100];
	fp=fopen("passwd","r");
	while(fscanf(fp,"%s",tmp1)!=EOF)
	{
		if(strcmp(tmp1,buf)==0)return 1;
	}
	return 0;
}
void init(){
	sockfd = socket(PF_INET,SOCK_STREAM,0);
	if (sockfd == -1){
		perror("Create socket Failed");
		exit(-1);
	}
	struct sockaddr_in addr;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);

	if (bind(sockfd,(SA*)&addr,sizeof(addr)) == -1){
		perror("Bind Failed");
		exit(-1);
	}
	if (listen(sockfd,100) == -1){
		perror("Listen Failed");
		exit(-1);
	}

	for(int i=0; i<105; i++)
		account[i] = (char *)malloc(sizeof(char)*11);
}

void SendMsgToAll(char* msg){
	int i;
	for (i = 0;i < size;i++){
		if (fds[i] != 0){
			printf("System sending to %d\n",fds[i]);
			send(fds[i],msg,strlen(msg),0);
		}
	}
}

void getAlluser(int fd){

	char bar[100]="\n--------------------\n";
	printf("%d ls.\n", fd);
	send(fd,bar,strlen(bar),0);
	send(fd,"[目前在線名單]\n",strlen("[目前在線名單]\n"),0);
	for (int i = 0;i < size;i++){
		if (fds[i] != 0){
			char buf[100] = {};
			if(fds[i]!=fd){
				sprintf(buf, "user: [%s] fd:%d\n" ,account[fds[i]] ,fds[i]);
				send(fd,buf,strlen(buf),0);
			}
		}
	}
	send(fd,bar,strlen(bar),0);
	char buf[100] = {};
	strcpy(buf,"請選擇你的對手: (enter @fd)");
	send(fd,buf,strlen(buf),0);
}


void* service_thread(void* p){
	int fd = *(int*)p,z;
	char *ptr,tmp[100];
	printf("pthread = %d\n",fd);

	char buf[100] = {};
	/*!LOGIN*/
	while(1){
		send(fd ,"authenticate",strlen("authenticate"),0);
		recv(fd,buf,sizeof(buf),0);//收到帳密
		printf("buf=%s\n",buf);
		ptr=strstr(buf,":");
		*ptr='\0';
		strcpy(account[fd], buf);
		account[fd][strlen(account[fd])] = '\0';
		*ptr=':';
		ptr=strstr(buf,"@");
		*ptr='\0';
		if((z=authe(buf))==1){
			ptr=strstr(buf,":");
			*ptr='\0';
			printf("New account : %s\n",  account[fd]);
			SendMsgToAll(account[fd]);
			break;
		}
		if(z==0)
        {
            printf("fail\n");
            pthread_kill(fd,SIGALRM);
        }
	}
	while(1){
		char buf2[100] = {};

		if (recv(fd,buf2,sizeof(buf2),0) <= 0){
			// 對方關閉了
			int i;
			for (i = 0;i < size;i++){
				if (fd == fds[i]){
					fds[i] = 0; //設為這個fd不存在了
					break;
				}
			}
			printf("退出：fd = %d quit\n",fd);
			pthread_exit((void*)i);
		}

		if (strcmp(buf2,"ls") == 0){
			getAlluser(fd);
		}
		else if(buf2[0]=='@')
		{
			int oppofd=atoi(&buf2[1]);
			char *msg = (char*)malloc( 256*sizeof(char) );
			strcpy(msg,"CONNECT ");
			strcat(msg,account[fd]);
			sprintf(tmp," %d",fd);
			strcat(msg,tmp);
			send(oppofd,msg,strlen(msg),0);
			send(fd,"等待對方的回應...\n\0",strlen("等待對方的回應...\n\0"),0);
		}
		else if(strncmp(buf2,"AGREE ",6)==0)
		{
			int oppofd=atoi(&buf2[6]);
			char *msg = (char*)malloc( 256*sizeof(char) );
			strcpy(msg,"AGREE ");
			strcat(msg,account[fd]);
			sprintf(tmp," %d",fd);
			strcat(msg,tmp);
			send(oppofd,msg,strlen(msg),0);
		}
		else if(buf2[0]=='#')
		{
			int n=atoi(&buf2[1]),oppofd;
			char *ptr,tmp[100];
			ptr=strstr(buf2," ");
			ptr++;
			oppofd=atoi(ptr);
			sprintf(tmp,"#%d",n);
			printf("[%d] buf=%s\n",fd,tmp);
			send(oppofd,tmp,strlen(tmp),0);
		}
		else{
			SendMsgToAll(buf2);
		}
		memset(buf2,0,sizeof(buf2));
	}
}


int main(){
	init();
	printf("Server Initial Successful!\n");
	while(1){
		struct sockaddr_in fromaddr;
		socklen_t len = sizeof(fromaddr);
		int fd = accept(sockfd,(SA*)&fromaddr,&len);
		if (fd == -1){
			printf("Client occurr Error ...\n");
			continue;
		}
		int i = 0;
		for (i = 0;i < size;i++){
			if (fds[i] == 0){
				fds[i] = fd;
				pthread_t tid;
				pthread_create(&tid,0,service_thread,&fd);
				break;
			}
			if (size == i){
				char* str = "Sorry, the room is full!";
				send(fd,str,strlen(str),0);
				close(fd);
			}
		}
	}
}


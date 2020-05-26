#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#define MAX 80
#define PORT 8899
//#define SERVER_IP "10.65.142.187"
#define SERVER_IP "172.22.27.131"
#define SA struct sockaddr

void funcClient(int sockfd)
{
	char buff[MAX];
	int n;
	for(;;)
	{
		bzero(buff,MAX);
		n=0;
		printf("Enter Command to Listener : ");
		while((buff[n++]=getchar())!='\n');
		if(strncmp("exit",buff,4)==0)
		{
			printf("Server Exit...\n");
			break;
		}
		write(sockfd,buff,sizeof(buff));
		bzero(buff,MAX);
		read(sockfd,buff,sizeof(buff));
		printf ("From Listener: %s\n",buff);
	}
}


int main()
{
	int sockfd,connfd;
	struct sockaddr_in servaddr,cli;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd==-1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
		
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=inet_addr(SERVER_IP);
	servaddr.sin_port=htons(PORT);
	printf("Socket successfully created..\n");
	printf("Trying to connnect to server %s.",SERVER_IP);
	while(1)
	{
		if(connect(sockfd,(SA *)&servaddr,sizeof(servaddr))!=0)
		{
			printf(".");
			sleep(5);
		}
		else
		{
			printf("\nconnected to the server..\n");
			break;
		}
	}
	
	funcClient(sockfd);
	close(sockfd);
}

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define IP_ADDR "127.0.0.1"
#define PORT_NUM 55555
#define BUF_SIZE 1024
#define MAX_NAME_NUM 25
#define TRUE 1
#define FALSE 0
#define QUIT "$quit$"

int registername(int cliSockfd)
{
	char* k;
	char buf[2][BUF_SIZE];
	memset(buf[0], '\0', BUF_SIZE);
	memset(buf[1], '\0', BUF_SIZE);
	read(0, buf[0], BUF_SIZE-1);
	if(k = strchr(buf[0],'\n'))
	{
		*k = '\0';
	}		
	write(cliSockfd, buf[0], BUF_SIZE - 1);
	read(cliSockfd, buf[1], BUF_SIZE - 1);
	if(strcmp(buf[1], "succeed") == 0)
	{
		printf("successful registration.\nyou has entered the chat room\n");
		return 1;
	}
	else
	{
		printf("The name of has been registered, please enter another.\n");
		return 0;
	}
}

void* thread_read(int* fd)
{
	char buf_read[BUF_SIZE];
	while(1)
	{
		memset(buf_read, '\0', BUF_SIZE);
		read(*fd, buf_read, BUF_SIZE-1);
		write(1,buf_read,BUF_SIZE-1);
	}
}

void main(int argc,char* argv[])
{	
	int cliSockfd;//定义 客户端Socket文件描述符
	int port;
	const char* ip;//定义一个 指向 ip存放地址 的指针
	char c[] = IP_ADDR;

	
	char remote[INET_ADDRSTRLEN];
	char buf_send[BUF_SIZE];//写缓冲区
	char buf_recv[BUF_SIZE];//读缓冲区
	char str_recv[BUF_SIZE];
	pthread_t pid;

	if(argc == 1)
	{
		ip = c;
		port = PORT_NUM;
	}
	else if(argc == 2)
	{
		ip = c;
		port = atoi(argv[1]);
	}
	else
	{
		ip = argv[1];
		port = atoi(argv[2]);
	}

	if((cliSockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)//socket()
	{
		printf("socket error: %s (errno:%d)\n", strerror(errno), errno);
		exit(1);
	}
	
	struct sockaddr_in servAddr;//服务器端 监听socket 地址
	bzero(&servAddr, sizeof(servAddr));//初始化servAddr内存区
	servAddr.sin_family = AF_INET;//该socket使用Ipv4协议
	servAddr.sin_port = htons(port);//设置socket端口号
	inet_pton(AF_INET, ip, &servAddr.sin_addr);//设置socket ip	
	
	if(connect(cliSockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
	{
		printf("connect error: %s (errno:%d)\n", strerror(errno), errno);
		exit(1);
	}
	printf("connect server successful.\nplease register a name.\n");
	
	int ret = 0;
	while(!ret)
	{
		ret = registername(cliSockfd);
	}

	pthread_create(&pid, NULL, (void *)thread_read, &cliSockfd);
	while(1)
	{ 
		int ret = read(0, buf_send, BUF_SIZE - 1);
		buf_send[ret] = '\0';
		write(cliSockfd, buf_send, BUF_SIZE - 1);
	}
	pthread_join(pid, NULL);
	close(cliSockfd);
}

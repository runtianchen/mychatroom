#include<arpa/inet.h>
#include<errno.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/epoll.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include"drawer.h"

#define IP_ADDR "0.0.0.0"
#define MAX_EVENT_NUM 1024
#define MAX_NAME_NUM 25
#define BUFFER_SIZE 1024
#define TRUE 1
#define FALSE 0

typedef struct
{
	int fd_from;
	int fd_to;
	char msg[BUFFER_SIZE];
}mydata,*qmydata;

typedef struct
{
	int fd;
	char name[MAX_NAME_NUM];
	qmydata data;
}myevent,*qmyevent;

int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd(int epollfd, int fd, int enable_et)
{
	qcandy qcan = (qcandy)malloc(sizeof(candy));
	qmyevent qme = (qmyevent)malloc(sizeof(myevent));
	qmydata qmd = (qmydata)malloc(sizeof(mydata));
	qcan->data = qme;
	qme->fd = fd;
	strcpy(qme->name, "default");
	qme->data = qmd;
	struct epoll_event event;
	event.data.ptr = qcan;	
	event.events = EPOLLIN;
	if(enable_et)
	{
		event.events |= EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void delete(){}

int registername(qdrawer qdraw, qcandy qcan)
{
	qmyevent qme = qcan->data;
	candy head;
	GetHead(qdraw, &head);			
	while(head.next!=NULL)
	{
		qmyevent temp = head.next->data;
		if(strcmp(temp->name, qme->data->msg) == 0)
		{
			char str[] = "re-registration";
			write(qme->fd, str, sizeof(str)-1);
			return 0;
		}
		head.next = head.next->next;
	}
	strcpy(qme->name, qme->data->msg);
	qme->name[MAX_NAME_NUM-1] = '\0';
	EnDrawer(qdraw, qcan);
	char str[] = "succeed";
	write(qme->fd, str, sizeof(str)-1);
	return 1;
}

int recvmessage(int fd, qcandy qcan)
{
	qmyevent qme = qcan->data;
	memset(qme->data->msg, '\0', BUFFER_SIZE);
	while(TRUE)
	{
		char buf[BUFFER_SIZE];
		memset(buf, '\0', BUFFER_SIZE);
		int ret = read(fd, buf, BUFFER_SIZE-1);
		if(ret < 0)
		{
			if((errno == EAGAIN) || (errno == EWOULDBLOCK))
			{
				//读取完毕
				break;
			}
			close(fd);
			break;
		}
		else if(ret == 0)
		{
			//客户端退出
			//epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, events+i);
			//close(fd);
			return 0;
		}
		else
		{
			strcat(qme->data->msg, buf);
		}
	}
	qme->data->msg[BUFFER_SIZE-1] = '\0';
	return 1;
}

void sendmessage(qdrawer qdraw, qcandy qcan)
{
	char msg_send[BUFFER_SIZE];
	memset(msg_send, '\0', BUFFER_SIZE);
	qmyevent qme = qcan->data;
	if(strcmp(qme->name,"default") == 0)
	{
		if(registername(qdraw, qcan))
		{
			strcat(msg_send, qme->name);
			strcat(msg_send, " has entered the chat room.\n");
		}
		else
		{
			return;
		}
	}
	else if(strlen(qme->data->msg) != 0)
	{
		strcat(msg_send, qme->name);
		strcat(msg_send, " : ");
		strcat(msg_send, qme->data->msg);
	}
	else
	{
		strcat(msg_send, qme->name);
		strcat(msg_send, " left the chat room.\n");
	}
	msg_send[BUFFER_SIZE-1]='\0';
	write(1,msg_send,BUFFER_SIZE-1);
	candy head;
	GetHead(qdraw, &head);				
	while(head.next!=NULL)
	{
		qmyevent temp = head.next->data;
		if(temp->fd != qme->fd)
		{
			write(temp->fd, msg_send, BUFFER_SIZE-1);
		}
		head.next = head.next->next;
	}
}

void et(int epollfd, int servSockfd, struct epoll_event* events, qdrawer qdraw, int num)
{
	int i;
	for(i = 0; i < num; i++)
	{
		qcandy qcan = events[i].data.ptr;
		qmyevent qme = qcan->data;
		int fd = qme->fd;
		if(fd == servSockfd)
		{
			struct sockaddr_in cliAddr;
			socklen_t cliAddrLen = sizeof(cliAddr);
			int clifd = accept(servSockfd, (struct sockaddr*)&cliAddr, &cliAddrLen);
			addfd(epollfd, clifd, TRUE);
		}
		else if(events[i].events & EPOLLIN)
		{
			recvmessage(fd,qcan);
			sendmessage(qdraw, qcan);
			if(strlen(qme->data->msg) == 0)
			{
				epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, events+i);
				DeDrawer(qdraw, qcan);
				close(fd);
			}
		}
		else
		{
			printf("something else happened.\n");
		}	
	}
}

int main(int argc,char* argv[])
{
	int servSockfd, epollfd;
	int port;
	int num;
	const char* ip = IP_ADDR;

	drawer draw;
	qdrawer qdraw = &draw;
	InitDrawer(qdraw);

	if(argc <= 1)
	{
		port = 55555;
	}
	else
	{
		port = atoi(argv[1]);
	}

	if((servSockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket error: %s (errno:%d)\n", strerror(errno), errno);
		exit(1);
	}

	struct sockaddr_in servAddr;
	bzero(&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &servAddr.sin_addr);
	servAddr.sin_port = htons(port);
	
	if(bind(servSockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
	{
		printf("bind error: %s (errno:%d)\n", strerror(errno), errno);
		exit(1);
	}
	printf("server creates success. the port is : %d\n",port);
	
	if(listen(servSockfd, 5) < 0)
	{
		printf("listen error: %s (errno:%d)\n", strerror(errno), errno);
		exit(1);	
	}

	struct epoll_event events[MAX_EVENT_NUM];
	if((epollfd = epoll_create(5)) < 0)
	{
		printf("epoll create error: %s (errno:%d)\n", strerror(errno), errno);
		exit(1);
	}
	addfd(epollfd, servSockfd, TRUE);

	while(1)
	{
		if((num = epoll_wait(epollfd, events, MAX_EVENT_NUM, -1)) < 0)
		{
			printf("epoll failuer\n");
			break;
		}
		//lt(epollfd, servSockfd, events, num);
		et(epollfd, servSockfd, events, qdraw, num);
	}
	
	close(servSockfd);
	return 0;
}

/*
 * 使用epoll测试非阻塞connect
 */
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

using namespace std;

#define MAXLINE 500
#define OPEN_MAX 100
#define LISTENQ 20
#define SERVE_PORT 5000
#define INFTIME 1000


void setnonblocking(int sock)
{
	int opts;
	opts = fcntl(sock,F_GETFL);
	if(opts < 0)
	{
		cout<<"fcntl(sock, GETFL)"<<endl;
		return;
	}
	opts = opts | O_NONBLOCK;
	if(fcntl(sock, F_SETFL, opts) < 0)
	{
		cout<<"fcntl(sock, SETFL, opts)"<<endl;
		return;
	}
}
//返回>=0，则返回的是连接成功的sockfd , 返回-1,说明sockfd还正在连接
//返回-2出现错误
int create_socket(char* ip, int port, int* fd)
{
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout<<"socket error"<<endl;
		return -2;
	
	}

	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	if(inet_pton(AF_INET, ip, &serveraddr.sin_addr) <= 0)
	{
		cout<<"inet_pton error for"<<ip<<endl;
		return -2;
	}
	setnonblocking(sockfd);
	int res = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	*fd = sockfd;
	if(res == 0)
		return sockfd;
	else
	{
		if(errno == EINPROGRESS)
		{
			return -1;
		}
		else
		{
			return -2;
		}
	}
}


void CloseAndDisable(int sockid, epoll_event ee)
{
	close(sockid);
	ee.data.fd = -1;
}

int main(int argc, char** argv)
{

	if(argc != 3)
		return 0;
	char* ip = argv[1];
	int port = atoi(argv[2]);
	
	int i, maxi,  epfd, nfds;
	int sockfd; //epoll_wait返回时，event中的fd
	int connfd; //用于accept的新的连接描述符
	int listenfd;	// 监听fd
	int portnumber; //服务器端口

	char line[MAXLINE];	//数据缓存区
	socklen_t clilen;
	
	portnumber = 5000;

	//声明epoll_event　结构体的变量，ev用于注册事件,数组用于回传要处理的事件
	struct epoll_event ev, events[20];
	
	//生成用于处理accept的epoll专用的文件描述符
	epfd = epoll_create(256);

	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;

	int client_fd = -1;
	int res = create_socket(ip, port, &client_fd);
	if(res != -1)
	{
		cout<<"can not start test"<<endl;
		return 0;
	}


	//设置与要处理的事件相关的文件描述符
	ev.data.fd = client_fd;
	//设置要处理的事件类型
	//ev.events = EPOLLIN | EPOLLET;
	ev.events = EPOLLIN | EPOLLOUT;
	
	//注册epoll事件
	epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);


	maxi = 0;
	
	int bOut = 0;
	for(;;)
	{
		if(bOut == 1)
			break;
		//等待epoll事件的发生
		nfds = epoll_wait(epfd, events, 20, -1);
		//处理所发生的所有事件
		cout<<"\nepoll_wait returns, and res is : "<<nfds<<endl;

		for(i = 0; i < nfds; i++)
		{
			//如果新监测到一个ＳＯＣＫＥＴ用户连接到了绑定的ＳＯＣＫＥＴ端口，建立新的连接
			if(events[i].data.fd == client_fd)
			{
				if(events[i].events & EPOLLOUT)
				{
					int status = -3;
					socklen_t slen = sizeof(int);
					int res = getsockopt(client_fd,SOL_SOCKET,SO_ERROR,(void*)&status, &slen);
					if(res < 0)
					{
						cout<<"getsockopt error"<<endl;
					}
					if(status != 0)
					{
						cout<<"connect fail"<<endl;
					}
					cout<<"connect success"<<endl;
					return 0;
				}
			}
		}
	}
	return 0;
}


/*
int main(int argc, char **argv)
 {
      int                    sockfd;
      char                recvline[MAXLINE + 1];
      struct sockaddr_in    servaddr;
  
      if (argc != 2)
	  {
	  	cout<<"usage: a.out <IPaddress>"<<endl;
		return 1;
	}
 
     if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	 	cout<<"socket error"<<endl;
		return 1;
	}
 
     bzero(&servaddr, sizeof(servaddr));
     servaddr.sin_family = AF_INET;
     servaddr.sin_port   = htons(5000);   
     if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	 	cout<<"inet_pton error for "<<argv[1]<<endl;
 
     if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	 {
	 	cout<<"connect error "<<endl;
		return 1;
	 }
 
     char input[100];
     while (fgets(input, 100, stdin) != NULL)
     {
         write(sockfd, input, strlen(input));
 
         int n = 0;
         int count = 0;
         while (1)
         {
             n = read(sockfd, recvline + count, MAXLINE);
             if (n == MAXLINE)
             {
                 count += n;
                 continue;
             }
             else 
                 break;
         }
         printf("%s\n", recvline);
     }
}
*/

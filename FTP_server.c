#include "agreement.h"
#include "pthread.h"
#include <sys/epoll.h>

#define MAX_EVENTS 64
#define MAX_THREADS 10
pkg_ser temp;
/*
    服务器功能： 
		等待客户端的连接，支持多客户端并发(加分)。根据客户端发来的命令，执行相应操作，并向客户端发送其需要的数据
*/
int main(int argc ,char *argv[])//使用argv传入命令以及参数
{
	int epoll_wait_fd=0,client_sock=0;//epoll_wait
	int sockfd=0,n=0;
	struct epoll_event event, events[MAX_EVENTS]; //ev用于注册事件，数组用于传回要处理的事件
	long int server_sock;
	struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
	pthread_pool my_pthread_pool;
	
	//0.识别传入的参数，判断端口或者ip地址是否正确
	if(argc != 3)
	{
		fprintf(stderr,"Usage:%s <IP> <PORT>\n",argv[0]);
		return -1;
	}
  	//1.创建套接字 
	server_sock = socket(AF_INET,SOCK_STREAM,0);
/*
    AF_INET：这是套接字地址族（Address Family）的一种，表示 IPv4 地址族。它指定了 socket 使用 IPv4 地址。
    SOCK_STREAM：这是套接字类型（Socket Type）。在这里，使用 SOCK_STREAM 表示创建一个面向连接的 TCP 套接字。TCP 是一种可靠的、面向连接的协议，它提供了数据传输的完整性和有序性。
    0：这是协议参数，通常为 0 表示自动选择适合地址族和套接字类型的默认协议。对于 TCP 套接字，通常会使用 IPPROTO_TCP。
*/
	if(-1 == server_sock )
	{
		perror("socket failed!!");
		return -1;
	}
	int on =1;
	setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,(void*)&on,sizeof(on));//设置套接字地址端口重用
	setsockopt(server_sock,SOL_SOCKET,SO_REUSEPORT,(void*)&on,sizeof(on));
	//2.绑定服务器的IP和PORT
	struct sockaddr_in Serv;
	Serv.sin_family = AF_INET;
	Serv.sin_port=htons(atoi(argv[2]));//
	Serv.sin_addr.s_addr = inet_addr(argv[1]);
	
	int ret = bind(server_sock,(struct sockaddr *)&Serv,sizeof(Serv));
	if(ret == -1)
	{
		perror("bind failed!!");
		close(server_sock);
		return -1;
	}
	//3.监听模式
	ret = listen(server_sock,10);
	if(ret == -1)
	{
		perror("listen failed!!");
		close(server_sock);
		return -1;
	}
	setnonblocking(server_sock);     // 把监听的socket设置为非阻塞。
	//创建epoll句柄
	int epoll_create_fd =  epoll_create(1024);
	if(epoll_create_fd == -1){perror("epoll create failed!!!\n");}
	event.events = EPOLLIN|EPOLLET;//边缘触发和可读事件(边缘触发模式)
	event.data.fd = server_sock;
	//注册epoll事件
	if (epoll_ctl(epoll_create_fd,EPOLL_CTL_ADD,server_sock,&event) == -1) {
	perror("epoll_ctl");}
	//@size：用来告诉内核这个监听数量一共有多大。现在已经被忽略了，意思只要填>0即意味着监听任意数量，同时epoll本身也会占用一个文件描述符，使用完之后需要关闭
	//初始化线程池
	//init_pool(&my_pthread_pool,10);
	while (1) 
	{
		epoll_wait_fd = epoll_wait(epoll_create_fd, events, MAX_EVENTS, -1);//最后一个参数是超时时间;
		
		// 超时。
		if (epoll_wait_fd == 0)
		{
			printf("epoll() timeout.\n"); continue;
		}
		if (epoll_wait_fd<0) {
			perror("epoll_wait_failed");
			break;
		}
		//如果epoll_wait_fd>0，表示有事件发生的socket的数量。
		for (int i = 0; i < epoll_wait_fd; i++) // 遍历epoll返回的数组evs
		{
			//printf("ptr=%s,events=%d\n",(char *)events[i].data.ptr,events[i].events);
			
			if (events[i].data.fd == server_sock) {//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
				client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
				//int confd = accept(sockfd,(struct sockaddr *)&clie,&len);
				if (client_sock == -1) {
					perror("accept");
					continue;
				}
				printf("connect [%s][port:%d]\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
				
				// 为新客户端准备读事件，并添加到epoll中。
				event.data.fd=client_sock;
				event.events=EPOLLIN|EPOLLET;
				epoll_ctl(epoll_create_fd,EPOLL_CTL_ADD,client_sock,&event);
			}else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
			{
				// 如果是客户端连接的socke有事件，表示有报文发过来或者连接已断开。
				if ( (sockfd = events[i].data.fd) < 0)
            		continue;
				char buffer[1024]; // 存放从客户端读取的数据。
				memset(buffer,0,sizeof(buffer));
				if (recv(events[i].data.fd,buffer,sizeof(buffer),0)<=0)
				{
					// 如果客户端的连接已断开。
					printf("client(eventfd=%d) disconnected.\n",events[i].data.fd);
					close(events[i].data.fd);            // 关闭客户端的socket
					// 从epollfd中删除客户端的socket，如果socket被关闭了，会自动从epollfd中删除，所以，以下代码不必启用。
					// epoll_ctl(epollfd,EPOLL_CTL_DEL,evs[ii].data.fd,0);     
				}
				else
				{
					//多线程尝试
					{// 如果客户端有报文发过来。
					//printf("recv(eventfd=%d):%s\n",events[i].data.fd,buffer);
					// 把接收到的报文内容原封不动的发回去。
					//send(events[i].data.fd,buffer,strlen(buffer),0);
					//printf("send(eventfd=%d):%s\n",events[i].data.fd,buffer);}
					// 创建任务并添加到线程池
					// Task* task = (Task*)malloc(sizeof(Task));
					// task->client_sock = client_sock;
					// task->start_offset = 0;  // TODO: 根据断点续传逻辑设置起始偏移量
					// task->end_offset = -1;   // TODO: 根据断点续传逻辑设置结束偏移量
					// task->file_name = strdup("file_to_transfer");  // TODO: 设置要传输的文件名
					// add_task(&my_pthread_pool,(void (*)(void *))my_tcp_translate_task,(void*)task);
					}
					//bad ending
					//cp_dir(&pool,argv[1],argv[2]);->
					{
						//将传入的数据赋值到temp中
						//printf("cmd_no:%d%d%d%d\n",buffer[5],buffer[6],buffer[7],buffer[8]);
						memset(&temp,0,sizeof(temp));
						memcpy(&temp,buffer,sizeof(temp));
						//temp.cmd_no=(buffer[5]&0XFF)+((buffer[6]&0XFF)<<8)+((buffer[7]&0XFF)<<16)+((buffer[8]&0XFF)<<24);//命令在字符串数组中的位置
						//printf("recv data[%s]:%d\n",inet_ntoa(cile.sin_addr),temp.cmd_no);
						//识别命令
						printf("cmd_no:%d\n",temp.cmd_no);
						switch(temp.cmd_no)
						{
							case 1024:
								show_dir(events[i].data.fd);printf("show_dir successed\n");break;
							case 512:
								send_file(events[i].data.fd,buffer);break;
							case 256:
								recv_file(events[i].data.fd,buffer);break;
							case 128:
								client_exit(events[i].data.fd);sleep(200);close(events[i].data.fd);break;
							default:
								printf("the cmd was undefinited\n");null_return(events[i].data.fd);break;
						}
						printf("指令处理完毕\n");
					}
				}
			}if(events[i].events&EPOLLOUT)
			{

			}
		}
	}
	//destroy_pool(&my_pthread_pool);
	close(server_sock);
	close(epoll_create_fd);
	return 0;	
}
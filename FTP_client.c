#include "agreement.h"
/*
    客户端功能：
		负责连接服务器，向服务器发送命令，并等待服务器回应，同时处理服务器发过来的数据。
*/pkg_ser temp;
int main(int argc,char *argv[])
{
  	//1.创建套接字 
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sockfd)
	{
		perror("socket failed!!");
		return -1;
	}
    int on =1;
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(void*)&on,sizeof(on));//重用地址
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,(void*)&on,sizeof(on));//重用端口
	//2.创建连接
    struct sockaddr_in Serv;
	Serv.sin_family = AF_INET;
	Serv.sin_port=htons(atoi(argv[2]));
	Serv.sin_addr.s_addr = inet_addr(argv[1]);
	socklen_t len = sizeof(Serv);
    int confd = connect(sockfd,(struct sockaddr *)&Serv,len);
    if(confd == -1)
    {
        perror("confd failed!!");
        exit(1);
    }
    printf("connect [%s][port:%d]\n",inet_ntoa(Serv.sin_addr),ntohs(Serv.sin_port));
    //创建一个新的进程与客户端通信
		pid_t pid = fork();
		//while(1)
        {
            if(pid >0)
            {
                while(1)
                {
                    sleep(2);
                    char send_buf[1024]={0};
                    printf("请输入你要发送的内容\n");
                    scanf("%s",send_buf);
                    client_send(sockfd,send_buf);
                    if(0==strcmp("byebye",send_buf))
                        return 0;
                    //continue;
                    //printf("6\n");
                }
            }
            else if(pid ==0)
            {
                while(1)
                {
                    char recv_buf[1024]={0};
                    int ret = recv(sockfd,recv_buf,1024,0);
                    
                    if(ret > 0)
                    {
                        printf("client recv over\n");	
                        memset(&temp,0,sizeof(temp));
						memcpy(&temp,recv_buf,sizeof(temp));
                        //将传入的数据赋值到temp中
                        //temp.cmd_no=temp.cmd_no=(recv_buf[5]&0XFF)+((recv_buf[6]&0XFF)<<8)+((recv_buf[7]&0XFF)<<16)+((recv_buf[8]&0XFF)<<24);//命令在字符串数组中的位置
                        //识别命令
                        printf("recv_cmd:%d\n",temp.cmd_no);
                        switch(temp.cmd_no)
                        {
                            case 1024:
                                server_out(recv_buf);break;
                            case 512:
                            	client_recv(recv_buf);break;
                            case 256:
                                client_filesend(sockfd,recv_buf);break;
                            case 128:
                            	close(sockfd);printf("connect exit!\n");return 0;
                            default:
                                printf("the cmd was undefinited\n");//erver_out(recv_buf);
                                break;
                        } 
                    }
                    else if(ret<0)
                    {
                        printf("recv failed\n");
                        continue;
                    }
                    else
                    {
                        printf("connect lost");
                        break;
                    }
                }
            }
            else
            {
                perror("fork failed");
            }
        }	
    
}
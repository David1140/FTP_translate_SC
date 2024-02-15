#include "agreement.h"
#include "pthread.h"
unsigned char pack_temp[1024]={0};
// 构造结构体
pkg_ser package;
int setnonblocking(int fd)
{
    int  flags;
    // 获取fd的状态。
    if  ((flags=fcntl(fd,F_GETFL,0))==-1)
        flags = 0;
    return fcntl(fd,F_SETFL,flags|O_NONBLOCK);
}
long int getfile_size(char*filename)
{
    FILE *fp;
    long int size;
    fp = fopen(filename, "rb"); // 打开文件
    if(fp == NULL) {
        printf("打开文件失败！");
        return 1;
    }
    fseek(fp, 0L, SEEK_END); // 将文件指针移动到文件末尾
    size = ftell(fp); // 获取文件大小
    fclose(fp); // 关闭文件
    printf("文件大小为 %ld 字节。\n", size);
    return size;
}
void client_filesend(int sockfd,char*str)//send file to server str 是filename
{
    //int flag =0;//标记是否找到了目标文件夹
    unsigned char *cmd;//int x=0;//用于保存内容的长度
    //DIR *dir = opendir("./client");//打开要复制的目录
	// if(dir == NULL)
	// {
	// 	perror("opendir dir_src failed");
	// 	return ;
	// }
    //struct dirent *dirp = NULL;
    int ret=0;
	// while(dirp = readdir(dir))// {   
    char file_src[MAXNAMELEN]={0};
    char file_dest[MAXNAMELEN]={0};
    // printf("%d:%d",sizeof(dirp->d_name),sizeof(toclient.neirong));
    //printf("dirname:%scompare%s=%d\n",dirp->d_name,str,strcmp(str,dirp->d_name));
        // if(0 == strncmp(str,dirp->d_name,strlen(str)))
        // {
            sprintf(file_src,"%s/%s","./server",str);//dirp->d_name
            sprintf(file_dest,"%s/%s","./client",str);//dirp->d_name
            char buf[1024];
            //printf("!!!open%s!!!\n",file_dest);
            int fd = open(file_dest,O_RDONLY,0777);
            if(fd==-1)
            {
                perror("open failed");
                return;
            }
            long int temp_size=getfile_size(file_dest);
            while(1)
            {
                char *pathname=malloc(1024);
                ret = read(fd,buf,400);
                if(ret == 0){break;}//write(fd_dest,buf,ret);
                //printf("%s:%s",file_src,buf);
                //printf("neirong:%s\n",buf);
                sprintf(pathname,"%s:%s",file_src,buf);printf("the file:%s\n",pathname);
                cmd=packaging(FTP_CMD_PUT,strlen(pathname),pathname,ret,temp_size);
                int confd=send(sockfd,cmd,1024,0);
                free(pathname);
                if(-1==confd)
                {
                    perror("send failed\n");
                }
            }
            close(fd);
        //}
    //}
    //printf("line:%d\n",__LINE__);
	//closedir(dir);
}
void server_out(char *recv_buf)
{
    pkg_ser temp;
    //将传入的数据赋值到temp中
    memcpy(&temp,recv_buf,sizeof(pkg_ser));
    //temp.resp_len=recv_buf[9]+(recv_buf[10]<<8)+(recv_buf[11]<<16)+(recv_buf[12]<<24);
    printf(" the recv_len:%d\n",temp.resp_len);
    printf("收到的内容是：");
    //for(int i=0;i<temp.resp_len;i++)
    {
        printf("%s",temp.neirong);
    }
    printf("\n");
}
void client_recv(char *str)//recv file
{ 
    pkg_ser over;//int flag =0;
    memcpy(&over,str,sizeof(pkg_ser));
    
    printf("the filelen:%d\n",over.resp_len);
    int namelen=0;
    char *filename;char *all;
    printf("内容是：%s\n",over.neirong);
    char *dest;dest = over.neirong;//dest +=9; 
    //printf("内容是：%s\n",dest);
    filename=strtok(dest,":");
    //sprintf(filename,"./client/%s",filename);
    printf("filename:%s\n",filename);
    int fd = open(filename,O_RDWR|O_CREAT|O_APPEND,0777);
    all=strtok(NULL,":"); //printf("%s\n",filename);
    printf("neirong:%s\n",all);
    write(fd,all,strlen(all));//over.resp_len-sizeof(filename)
    close(fd);
}
void client_send(int sockfd,char*str)
{
    unsigned char *cmd=NULL;//=(unsigned char*)malloc(1024);
    // int i=0;
    // int pkg_len = 1024;//数据包的总长度
    int temp=0;//用于保存指令
    int send_len =0;//用于保存发送内容的长度
    enum CMD_NO cmd_no = FTP_CMD_UNKNOW;
    //识别输入的指令
    if(0==strcmp(str,"ls"))
    {   
        enum CMD_NO cmd_no = FTP_CMD_LS;
        temp=cmd_no;
        printf("cmd_no/temp:%d\n",temp);
        cmd=packaging(temp,0,"\0",0,0);
        //printf("line:%d\n",__LINE__);
        int fd=send(sockfd,cmd,1024,0);
        //printf("send_cmd_no:%s\n",cmd);
        if(-1==fd)
        {
            perror("send failed1");
        }
    }
    else if(0==strcmp(str,"get"))
    {   
        enum CMD_NO cmd_no = FTP_CMD_GET;temp=cmd_no;
        char filename[100];
        printf("请输要下载的文件名:\n");
        scanf("%99s", filename);  // 注意限制输入字符数，避免缓冲区溢出
        send_len = strlen(filename);
        printf("文件名长度为:%d\n",send_len);
        printf("cmd_no/temp:%d\n",temp);
        cmd=packaging(temp,1024,filename,strlen(filename),strlen(filename));
        int fd=send(sockfd,cmd,1024,0);
        //printf("send_cmd_no:%s\n",cmd);
        if(-1==fd)
        {
            perror("send failed1");
        }
    }
    else if(0==strcmp(str,"put"))
    {   
        enum CMD_NO cmd_no = FTP_CMD_PUT;temp=cmd_no;
        char filename[100];
        printf("请输要上传的文件名:\n");
        //scanf("%s",&filename);
        scanf("%99s", filename);  // 注意限制输入字符数，避免缓冲区溢出
        //fread(&filename,sizeof(char),sizeof(filename),stdin);
        send_len = strlen(filename);
        printf("文件名长度为:%d\n",send_len);
        client_filesend(sockfd,filename);
        return ;
    }
    else if(0==strcmp(str,"byebye"))
    {   
        enum CMD_NO cmd_no = FTP_CMD_BYE;temp=cmd_no;
        cmd=packaging(temp,0,'\0',0,0);
        int fd=send(sockfd,cmd,1024,0);
        //printf("send_cmd_no:%s\n",cmd);
        if(-1==fd)
        {
            perror("send failed1");
        }
    }
    else 
    {
        enum CMD_NO cmd_no = FTP_CMD_UNKNOW;temp=cmd_no;
        cmd=packaging(temp,0,"\0",0,0);
        int fd=send(sockfd,cmd,1024,0);
        //printf("send_cmd_no:%s\n",cmd);
        if(-1==fd)
        {
            perror("send failed1");
        }
    }
    //free(cmd);
    return ;
}
//****************************************************服务器用的*****************************************
void show_dir(int sockfd)
{
   
    unsigned char *cmd;int x=0,all=0;char file_all[MAXNAMELEN]={0};
    DIR *dir = opendir("./server");//打开要复制的目录
	if(dir == NULL)
	{
		perror("opendir dir_src failed");
		return ;
	}
    struct dirent *dirp = NULL;
	while(dirp = readdir(dir))
	{   
        char file_src[MAXNAMELEN]={0};
		//将路径组装进来
		sprintf(file_src,"%s%s","@",dirp->d_name);
        for(int i=0;i<strlen(file_src);i++)
        {
            file_all[x]=file_src[i];
            x++;
        }all+=x;
		
    }
    file_all[x]='\0';
	closedir(dir);
    printf("ls dir:%s\n",file_all);
    enum CMD_NO cmd_no = FTP_CMD_LS;
    cmd=packaging(cmd_no,1024,file_all,strlen(file_all),strlen(file_all));
    int confd=send(sockfd,cmd,1024,0);
    if(-1==confd)
    {
        perror("send failed\n");
    }
    printf("\n");
}
void send_file(int sockfd,char*str)
{
    pkg_ser toclient,fromclient;//int flag =0;
    memcpy(&fromclient,str,sizeof(pkg_ser));
    printf("the get filename_len:%d\n",fromclient.resp_len);
    printf("the need get filename:%s\n",fromclient.neirong);
    char *finame;
    unsigned char *cmd;//int x=0;
    // DIR *dir = opendir("./server");//打开要复制的目录
	// if(dir == NULL)
	// {
	// 	perror("opendir dir_src failed");
	// 	return ;
	// }
    // struct dirent *dirp = NULL;
    
	// while(dirp = readdir(dir))
	// {   
        char file_src[MAXNAMELEN]={0};
        char file_dest[MAXNAMELEN]={0};
        //printf("%d:%d",sizeof(dirp->d_name),sizeof(toclient.neirong));
        // printf("dirname:%scompare%s=%d\n",dirp->d_name,toclient.neirong,strncmp(toclient.neirong,dirp->d_name,toclient.resp_len));
        // if(0 == strncmp(toclient.neirong,dirp->d_name,toclient.resp_len))
        // {
        //flag=1;
            sprintf(file_src,"%s/%s","./server",fromclient.neirong);
            sprintf(file_dest,"%s/%s","./client",fromclient.neirong);
            int fd = open(file_src,O_RDONLY,0777);
            if(fd==-1)
            {
                perror("open erroer or no such file");
                return ;
            }
            int ret=0;
            while(1)
            {
               
                char buf[1024]={0};
                char *pathname=malloc(1024);
                ret = read(fd,buf,400);
                if(ret == 0){break;}//write(fd_dest,buf,ret);
                printf("neirong:%s\n",buf);
                sprintf(pathname,"%s:%s",file_dest,buf);
                //printf("the file:%s\n",pathname);
                memcpy(toclient.neirong,pathname,strlen(pathname));
                printf("\npathname_len%ld\n",strlen(pathname));
                enum CMD_NO cmd_no = FTP_CMD_GET;
                cmd = packaging(cmd_no,strlen(pathname),pathname,strlen(pathname),getfile_size(file_src));
                int confd=send(sockfd,cmd,1024,0);
                if(-1==confd)
                {
                    perror("send failed\n");
                }
                free(pathname);
            }
            close(fd);
        //}
    //}
    //printf("line:%d\n",__LINE__);
    // if(0==flag)
    // { unsigned char *cmd=malloc(1024);int x=11;
    //     char str[11]={"nosuchfilen"};
    //     for(int i=0;i<x;i++)
    //     {
    //         cmd[13+i]=str[i];
    //     }
    //     int i=0;
    //     int pkg_len = 8;//数据包的总长度
    //     enum CMD_NO cmd_no = FTP_CMD_GET;
    //     cmd=packaging(cmd_no,11,str,0,0);
    //     int confd=send(sockfd,cmd,1024,0);
    //     if(-1==confd)
    //     {
    //         perror("send failed\n");
    //     }
    // }
	//closedir(dir);
    //free(cmd);
}
void recv_file(int sockfd,char*str)
{
    pkg_ser over;//int flag =0;
    memcpy(&over,str,sizeof(pkg_ser));
    //over.resp_len=str[9]+(str[10]<<8)+(str[11]<<16)+(str[12]<<24);
    printf("the filelen:%d\n",over.resp_len);
    int namelen=0;
    char *filename;char *all;
    printf("内容是：%s\n",over.neirong);
    char *dest;dest = over.neirong;//dest +=9;//将文件名加内容的形式去掉路劲 
    filename=strtok(dest,":");
    printf("filename:%s\n",filename);
    //if(over.ready_transferred<over.need_transferred)
    int fd = open(filename+2,O_RDWR|O_CREAT|O_APPEND,0777);
    all=strtok(NULL,":"); printf("write:%s\n",all);
    write(fd,all,strlen(all));//over.resp_len-sizeof(filename)
    close(fd);
    return ;
}
void client_exit(int sockfd)
{
    unsigned char *cmd;
    enum CMD_NO cmd_no = FTP_CMD_BYE;
    cmd=packaging(cmd_no,0,'\0',0,0);
    int confd=send(sockfd,cmd,1024,0);
    if(-1==confd)
    {
        perror("send failed\n");
    }
    close(sockfd);
}
void null_return(int sockfd)
{
    unsigned char *cmd;
    char str[11]={"cmdundefine"};
    enum CMD_NO cmd_no = FTP_CMD_LS;
    cmd=packaging(cmd_no,11,str,0,0);
    int confd=send(sockfd,cmd,1024,0);
    if(-1==confd)
    {
        perror("send failed\n");
    }
    printf("this is 发送的内容：\n");
}
void *task(void *arg)
{
    long int sockfd=(long int)arg;
    struct sockaddr_in clie;
    socklen_t len = sizeof(clie);
    //4.阻塞等待连接
    int confd = accept(sockfd,(struct sockaddr *)&clie,&len);
    if(confd == -1)
    {
        perror("confd failed!!");
        //continue;
    }
    printf("connect [%s][port:%d]\n",inet_ntoa(clie.sin_addr),ntohs(clie.sin_port));
    //add_task(&pool,(void*)task,(void*)sockfd);
    while(1)
	{
		//cp_dir(&pool,argv[1],argv[2]);->
		char recv_buf[1024]={0};    
	    int ret = recv(confd,recv_buf,1024,0);
        if(ret > 0)
		{
			pkg_ser temp;
			//将传入的数据赋值到temp中
			printf("cmd_no:%d%d%d%d\n",recv_buf[5],recv_buf[6],recv_buf[7],recv_buf[8]);
			temp.cmd_no=(recv_buf[5]&0XFF)+((recv_buf[6]&0XFF)<<8)+((recv_buf[7]&0XFF)<<16)+((recv_buf[8]&0XFF)<<24);//命令在字符串数组中的位置
		  	printf("recv data[%s]:%d\n",inet_ntoa(clie.sin_addr),temp.cmd_no);
   			//识别命令
     		switch(temp.cmd_no)
            {
                case 1024:
                    show_dir(confd);printf("show_dir successed\n");break;
                case 512:
                     send_file(confd,recv_buf);break;
            	case 256:
                    recv_file(confd,recv_buf);break;
                case 128:
                    client_exit(confd);sleep(200);close(confd);break;
                default:
        	        printf("the cmd was undefinited\n");null_return(confd);break;
            }
            printf("指令处理完毕\n");
        }
    }
}
void* my_tcp_translate_task(void* arg){
    while(1)
	{
		//cp_dir(&pool,argv[1],argv[2]);->
		char recv_buf[1024]={0};
        Task* temp = (Task*)arg; 
        int client_sock_temp =temp->client_sock;   
	    int ret = recv(client_sock_temp,recv_buf,1024,0);
        if(ret > 0)
		{
			pkg_ser temp;
			//将传入的数据赋值到temp中
			printf("cmd_no:%d%d%d%d\n",recv_buf[5],recv_buf[6],recv_buf[7],recv_buf[8]);
			temp.cmd_no=(recv_buf[5]&0XFF)+((recv_buf[6]&0XFF)<<8)+((recv_buf[7]&0XFF)<<16)+((recv_buf[8]&0XFF)<<24);//命令在字符串数组中的位置
		  	//printf("recv data[%s]:%d\n",inet_ntoa(cile.sin_addr),temp.cmd_no);
   			//识别命令
     		switch(temp.cmd_no)
            {
                case 1024:
                    show_dir(client_sock_temp);printf("show_dir successed\n");break;
                case 512:
                     send_file(client_sock_temp,recv_buf);break;
            	case 256:
                    recv_file(client_sock_temp,recv_buf);break;
                case 128:
                    client_exit(client_sock_temp);sleep(200);close(client_sock_temp);break;
                default:
        	        printf("the cmd was undefinited\n");null_return(client_sock_temp);break;
            }
            printf("指令处理完毕\n");
        }
    }
}
//********************************打包函数******************************************************************
unsigned char *packaging(int mingling,int len,char *neirong,int ready_transferred,long int need_transferred)
{
    memset(pack_temp,0,1024);
    package.package_head = 'H';
    package.pkg_len = sizeof(package);
    package.cmd_no = mingling;
    package.resp_len = ready_transferred;
    package.need_transferred = need_transferred;
    //strcpy(package.neirong, neirong);
    memcpy(package.neirong, neirong, strlen(neirong));
    package.result = 'N';
    package.package_end = 'E';
    printf("packing end\n");
    // 将结构体的数据按网络字节序转换
    // package.pkg_len = htonl(package.pkg_len);
    // package.resp_len = htonl(package.resp_len);
    // package.ready_transferred = htonll(package.ready_transferred);
    // package.need_transferred = htonll(package.need_transferred);

    // 将结构体复制到缓冲区中
    memcpy(pack_temp, &package, sizeof(package));
    return pack_temp;
}
#ifndef __AGREEMENT_H__
#define __AGREEMENT_H__
#define MAXNAMELEN 1024
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>//计时器
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE  "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define RESET "\033[0m"
/*DT_UNKNOWN：未知的文件类型。
        DT_REG：普通文件。~
        DT_DIR：目录文件。!
        DT_FIFO：FIFO 管道文件。@
        DT_SOCK：Socket 文件。#
        DT_LNK：符号链接文件。$
        DT_BLK：块设备文件。^
        DT_CHR：字符设备文件。&
        */
//pthread_pool pool;
//命令描述枚举		
enum CMD_NO
{
    FTP_CMD_LS = 1024,
    FTP_CMD_GET = 512,
    FTP_CMD_PUT = 256,
    FTP_CMD_BYE = 128,
    FTP_CMD_CD  = 64,
    FTP_CMD_UNKNOW = 0xFFFFFFFF
};
typedef struct PACKAGE_FTP
{
    char package_head ;
    int pkg_len;//占四个字节表示数据包的总长度，不包括包头包尾
    int cmd_no;//占四个字节，表示你的命令
    int resp_len;//内容长度
    long int ready_transferred;//已经发送总长度
    long int need_transferred;//需要发送总长度
	char neirong[512];//
    char result;//表示执行命令成功或失败
    //int reap_conent;//文件的内容或者错误码
    char package_end;
}pkg_ser,pkg_cli;
extern char server_dir[FILENAME_MAX];
//工具函数
int setnonblocking(int fd);
long int getfile_size(char*filename);
int custom_strrchr(const char* str, char c, char* buffer, size_t buffer_size);
//服务端
void show_dir(int sockfd);//ls
void send_file(int sockfd,char*str);//put
void recv_file(int sockfd,char*str);//get
void client_exit(int sockfd);//close
void cd_dir(char*str);
void null_return(int sockfd);//undefine
//线程调度用的函数
void *task(void *arg);
void* my_tcp_translate_task(void* arg);

//客户端
void client_send(int sockfd,char*str);// recv get
void server_out(char *recv_buf);//out recv ls or null
void client_recv(char *str);//recv file
void client_filesend(int sockfd,char*str);//send file to server
void client_cd_dir(int sockfd);//切换目录
//打包
unsigned char *packaging(int cmd,int len,char *neirong,int ready_transferred,long int need_transferred);
#endif
查看最大连接量命令： cat /proc/sys/fs/file-max 
查看最大监听数目：/proc/sys/net/core/somaxconn 
cd /mnt/hgfs/free/yb/project1/code_2.0/SC_code
./FTP_server 192.168.75.129 1024
./FTP_client 192.168.75.129 1024
./fortest_client  192.168.75.129 1024
gdb --args ./FTP_client 192.168.75.129 1025
gdb ./server
set args 192.168.75.129 1024 
192.168.75.129

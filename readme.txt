使用方法：程序名+配置文件名

注意事项：
如果报文分包，除了最后一个车载协议报文，其他报文长度均需要为1023字节。
总命令报文长度不能超过1023×10，若超过此长度需要截取为多个命令发送。
RSA加密报文规则：在需要加密字段前4个字节以网络字节序填入所需加密数据长度，然后分为100字节、100字节的块，对这些100字节的块进行加密，组成128字节的数据后再拼装起来。

依赖库：
apt-get install mysql-server mysql-client libmysqlclient-dev libssl-dev

调试：
崩溃的时候，会出现崩溃行信息，
如
./server
widebright received SIGSEGV! Stack trace:
0 ./server() [0x40728c] 
1 /lib/x86_64-linux-gnu/libc.so.6(+0x360b0) [0x7fd3dd3b10b0] 
2 ./server(main+0x44) [0x404984] 
3 /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xed) [0x7fd3dd39c7ed] 
4 ./server() [0x404a6d] 

查看0x404984对应代码：
addr2line 0x404984 -e server -f

invalide_pointer_error
/home/cqp/github/c_epoll_thread_pool_server/main.c:44
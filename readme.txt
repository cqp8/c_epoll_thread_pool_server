ʹ�÷�����������+�����ļ���

ע�����
������ķְ����������һ������Э�鱨�ģ��������ĳ��Ⱦ���ҪΪ1023�ֽڡ�
������ĳ��Ȳ��ܳ���1023��10���������˳�����Ҫ��ȡΪ�������͡�
RSA���ܱ��Ĺ�������Ҫ�����ֶ�ǰ4���ֽ��������ֽ�����������������ݳ��ȣ�Ȼ���Ϊ100�ֽڡ�100�ֽڵĿ飬����Щ100�ֽڵĿ���м��ܣ����128�ֽڵ����ݺ���ƴװ������

�����⣺
apt-get install mysql-server mysql-client libmysqlclient-dev libssl-dev

���ԣ�
������ʱ�򣬻���ֱ�������Ϣ��
��
./server
widebright received SIGSEGV! Stack trace:
0 ./server() [0x40728c] 
1 /lib/x86_64-linux-gnu/libc.so.6(+0x360b0) [0x7fd3dd3b10b0] 
2 ./server(main+0x44) [0x404984] 
3 /lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xed) [0x7fd3dd39c7ed] 
4 ./server() [0x404a6d] 

�鿴0x404984��Ӧ���룺
addr2line 0x404984 -e server -f

invalide_pointer_error
/home/cqp/github/c_epoll_thread_pool_server/main.c:44
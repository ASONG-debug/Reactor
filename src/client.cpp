// 网络通讯的客户端程序。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>

bool writen(int fd,const char* data,size_t size)
{
    size_t total=0;
    while (total<size)
    {
        const ssize_t n=send(fd,data+total,size-total,0);
        if (n>0) { total+=n; continue; }
        if ((n==-1)&&(errno==EINTR)) continue;
        return false;
    }
    return true;
}

bool readn(int fd,char* data,size_t size)
{
    size_t total=0;
    while (total<size)
    {
        const ssize_t n=recv(fd,data+total,size-total,0);
        if (n>0) { total+=n; continue; }
        if ((n==-1)&&(errno==EINTR)) continue;
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage:./client ip port\n"); 
        printf("example:./client 192.168.150.128 5085\n\n"); 
        return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];
 
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) { printf("socket() failed.\n"); return -1; }
    
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr=inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(sockfd);  return -1;
    }

    printf("connect ok.\n");
    printf("开始时间：%d\n",time(0));

    for (int ii=0;ii<10000;ii++)
    {
        memset(buf,0,sizeof(buf));
        sprintf(buf,"这是第%d个超级女生。",ii);

        char tmpbuf[1024];                 // 临时的buffer，报文头部+报文内容。
        memset(tmpbuf,0,sizeof(tmpbuf));
        uint32_t len=strlen(buf);                 // 计算报文的大小。
        uint32_t netlen=htonl(len);
        memcpy(tmpbuf,&netlen,4);       // 拼接报文头部。
        memcpy(tmpbuf+4,buf,len);  // 拼接报文内容。

        if (writen(sockfd,tmpbuf,len+4)==false) break;  // 把请求报文发送给服务端。
        
        if (readn(sockfd,reinterpret_cast<char*>(&netlen),4)==false) break;            // 先读取4字节的报文头部。
        len=ntohl(netlen);

        memset(buf,0,sizeof(buf));
        if (readn(sockfd,buf,len)==false) break;           // 读取报文内容。

        // printf("recv:%s\n",buf);
    }
    printf("结束时间：%d\n",time(0));

    /*
    for (int ii=0;ii<10;ii++)
    {
        memset(buf,0,sizeof(buf));
        sprintf(buf,"这是第%d个超级女生。",ii);

        send(sockfd,buf,strlen(buf),0);  // 把请求报文发送给服务端。
        
        memset(buf,0,sizeof(buf));
        recv(sockfd,buf,1024,0);           // 读取报文内容。

        printf("recv:%s\n",buf);
sleep(1);
    }
    */

} 

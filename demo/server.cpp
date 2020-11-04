#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <endian.h>
#include <byteswap.h>
#include <getopt.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "rdmabase.h"
#include <infiniband/verbs.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif

int main(int argc, char *argv[])
{
    RDMA res;
    /*显示配置*/
    res.print_config();
    
    /*连接QP*/
    res.connect_qp();
    
    /*发送数据*/
    char* msg = "Hello, I am server\n";
    int msgsize;
    msgsize = strlen(msg);
    res.Send(msg, msgsize);
    
    /*接受数据*/
    res.Recv();
    /*显示*/
    fprintf(stdout, "Message is %s", res.recvbuf);
    
    return 0;
}

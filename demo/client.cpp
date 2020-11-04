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

int main(int argc, char *argv[])
{
	RDMA res;

	if(argc == 2){
        res.server_name = std::string((argv[1]));
	}
	#ifdef DEBUG
	res.print_config();
	#endif

	/* 连接 QPs */
	res.connect_qp();
    
    /*接受数据*/
    res.Recv();
    
    /*发送数据*/
    char *msg = "Hello, I am client\n";
    int msgsize;
    msgsize = strlen(msg);
    res.Send(msg, msgsize);
    /*显示*/
    fprintf(stdout, "Message is %s", res.recvbuf);
    
    return 0;
}

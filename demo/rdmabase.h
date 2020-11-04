#ifndef _RDMABASE_H
#define _RDMABASE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <string>
#include <thread>
#include <infiniband/verbs.h>


// x should be equal to non-zero
#define RDMA_CHECK_NP(x, y) if(!(x)) {fprintf(stderr, "Error:%s\n", y); }
// x should be equal to zero
#define RDMA_CHECK_Z(x, y) RDMA_CHECK_NP(!(x), y)



#define MAX_POLL_CQ_TIMEOUT 2000


struct config_t{
    char        *dev_name;              // IB device name
    int         ib_port;                // local IB port to work with
    int         cq_size;                // cq max size
    int         sr_max;                 // send request max times in one time
    int         rr_max;                 // recv request max times in one time
    int         send_sge_max;
    int         recv_sge_max;
    size_t      sendbuf_size;           // send buffer bytes size
    size_t      recvbuf_size;           // recv buffer bytes size 
    
};

/*  Structure of exchanging data which is needed to connect the QPs 
    ------------------------------------------------------------------------------------------------------------
    __atribute__((packed)) 作用是告诉编译器取消结构在编译过程中的优化对齐，按照实际
    占用字节数进行对齐，GCC 特有语法
    ------------------------------------------------------------------------------------------------------------
*/
struct cm_con_data_t{
    uint64_t        addr;           // Buffer Address
    uint32_t        rkey;           // Remote Key
    uint32_t        qp_num;         // QP Number
    uint16_t        lid;            // LID of the IB port
}__attribute__((packed));

class RDMA{
private: 
    static struct config_t              config;             // config
    struct ibv_device_attr       *device_attr;        // device attributes
    struct ibv_port_attr         *port_attr;          // IB port attributes
    struct ibv_device            **dev_list;         // devie list
    struct ibv_context          *ib_ctx;            // device handle
    struct ibv_comp_channel     *channel;           // event channel
    struct ibv_pd               *pd;                // PD handle
    struct ibv_cq               *sendcq;            // Send CQ handle
    struct ibv_cq               *recvcq;            // Receive CQ handle 
    struct ibv_qp                      *qp;                // QP handle
    struct ibv_mr                      *sendmr;            // Send MR handle
    struct ibv_mr                      *recvmr;            // Recv MR handle
    
public:
    bool                        is_ready;           // RDMA connections ready flag
    int                  		num_devices;
    void                        *sendbuf;           // send buff pointer
    void                        *recvbuf;           // recv buff pointer
    struct cm_con_data_t        local_props;       // local info
    struct cm_con_data_t        remote_props;       // value to connect to remote side


	// functions
    RDMA();
    ~RDMA();
    int                         modify_qp_to_init();
    int                         modify_qp_to_rtr(uint32_t remote_qpn, uint16_t dlid);
    int                         modify_qp_to_rts();
    int                         connect_qp();
    int                         post_send(void* msg, size_t msgsize);
    int                         post_recv();
    int                 		notify_event();
    int                  		get_event();
    size_t               		poll_completion(struct ibv_cq* cq);
//    void                        print_config();
    size_t                      Send(void* msg, size_t msgsize);
    size_t                      Recv();
//    void                        StartRecv();
//    void                        Receiving();
};



/* Initialize the static member */
// ---------------------------------------------------------------------------------------------
//int RDMA::Instance_Num = 0;
//int RDMA::num_devices = 0;

struct config_t RDMA::config = {
    NULL,
    1,
    20,
    10,
    10,
    10,
    10,
    48*1000*1000,
    48*1000*1000
};

// ---------------------------------------------------------------------------------------------
/* print config information */
/*
void RDMA::print_config(){
    fprintf(stdout, " ------------------------------------------------\n");
    if (config.dev_name) 
        fprintf(stdout, " Device name                  : \"%s\"\n", config.dev_name);
    else 
        fprintf(stdout, " Device name                  : No Default Device\n");
	fprintf(stdout, " IB port                      : %u\n", config.ib_port);
	if (!server_name.empty())
	    fprintf(stdout, " IP                           : %s\n", server_name.c_str());
    fprintf(stdout, " TCP port                     : %u\n", config.tcp_port);
    fprintf(stdout, " CQ size                      : %u\n", config.cq_size);
    fprintf(stdout, " Send Requests Max:           : %u\n", config.sr_max);
    fprintf(stdout, " Recv Requests Max:           : %u\n", config.rr_max);
    fprintf(stdout, " Send Sge Max:                : %u\n", config.send_sge_max);
    fprintf(stdout, " Recv Sge Max:                : %u\n", config.recv_sge_max);
    fprintf(stdout, " Send Buffer Size:            : %zu\n", config.sendbuf_size);
    fprintf(stdout, " Send Buffer Size:            : %zu\n", config.recvbuf_size);
    fprintf(stdout, " CQE Max Number:              : %u\n", device_attr->max_cqe);
    fprintf(stdout, " WQ Max Number:               : %u\n", device_attr->max_qp_wr);
    fprintf(stdout, " Sge Max Number:              : %u\n", device_attr->max_sge);
    fprintf(stdout, " QP Max Number:               : %u\n", device_attr->max_qp);
	fprintf(stdout, " ------------------------------------------------\n\n");
}
*/
/*  资源初始化 
    配置 config
    获取 IB 设备名单
    查找特定 IB 设备
    获取 IB 设备句柄
    获取 IB 设备属性
    分配 Protection Domain
    创建 cq
    创建 Memory Buffer
    注册 Memory Buffer
    创建 QP
*/
RDMA::RDMA(){	
	//printf("start init RDMA res\n");
	dev_list = NULL;
	device_attr = NULL;
	port_attr = NULL;
	ib_ctx = NULL;
	channel = NULL;
	pd = NULL;              
	sendcq = NULL;           
	recvcq = NULL;   
    
    //获取IB设备名单
    dev_list = ibv_get_device_list(&num_devices);
    RDMA_CHECK_NP(dev_list, "ibv_device");
    RDMA_CHECK_NP(num_devices, "num_devices");
   // fprintf(stderr, "found %d IB device.\n", num_devices);
    
    //查找特定IB设备

    
    //获取设备句柄
    if(!ib_ctx){
        ib_ctx = ibv_open_device(*dev_list);
        RDMA_CHECK_NP(ib_ctx, "ibv_open_device");
    }
    
    //获取设备属性
    if(!device_attr){
        device_attr = new struct ibv_device_attr;
        int rc = ibv_query_device(ib_ctx, device_attr);
        RDMA_CHECK_Z(rc, "ibv_query_device");
    }
    if(!port_attr){
        port_attr = new struct ibv_port_attr;
        int rc = ibv_query_port(ib_ctx, config.ib_port, port_attr);
        RDMA_CHECK_Z(rc, "ibv_query_port");
    }
    
    //分配PD
    if(!pd){
        pd = ibv_alloc_pd(ib_ctx);
        RDMA_CHECK_NP(pd, "ibv_alloc_pd");
    }
    
    //创建事件channel
    if(!channel){
        channel = ibv_create_comp_channel(ib_ctx);
        RDMA_CHECK_NP(channel, "ibv_create_comp_channel");
    }
    
    //创建CQ
    if(!sendcq){
        sendcq = ibv_create_cq(ib_ctx, config.cq_size, NULL, channel, 0);
        RDMA_CHECK_NP(sendcq, "ibv_create_cq");
    }
    if(!recvcq){
        recvcq = ibv_create_cq(ib_ctx, config.cq_size, NULL, channel, 0);
        RDMA_CHECK_NP(sendcq, "ibv_create_cq");
    }
    
    //分配内存给memory buffer
    sendbuf = malloc(config.sendbuf_size);
    recvbuf = malloc(config.recvbuf_size);
    memset(sendbuf, 0, config.sendbuf_size);
    memset(recvbuf, 0, config.recvbuf_size);
    if(!recvbuf || !sendbuf){
        fprintf(stderr, "failed to mafailed to malloc %Zu bytes to memory buffer.\n", config.sendbuf_size);
        return;
    }

    
    //注册 memory buffer
    int mr_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE;
    recvmr = ibv_reg_mr(pd, recvbuf, config.recvbuf_size, mr_flags);
    sendmr = ibv_reg_mr(pd, sendbuf, config.sendbuf_size, mr_flags);
    if(!recvmr || !sendmr){
        fprintf(stderr, "ibv_reg_mr failed with mr_flags=0x%x\n", mr_flags);
        return;
    }

    /*
    fprintf(stdout, "Recv MR was registered with \n\taddr=%p, lkey=0x%x, rkey=0x%x, flags=0x%x\n",
            recvbuf, recvmr->lkey, recvmr->rkey, mr_flags);
    fprintf(stdout, "Send MR was registered with \n\taddr=%p, lkey=0x%x, rkey=0x%x, flags=0x%x\n",
            sendbuf, sendmr->lkey, sendmr->rkey, mr_flags);
  */

    //创建qp
/* struct ibv_qp_cap:
     uint32_t max_send_wr     - The number of Send Requests that can be outstanding in the QP
     uint32_t max_recv_wr     - The number of Receive Requests that can be outstanding in the QP
     uint32_t max_send_sge   - The number of S/G entries that each Send Request may hold 
     uint32_t max_recv_sge    - The number of S/G entries that each Receive Request may hold 
     uint32_t max_inline_data; - The requested inline data (in bytes) to be sent
*/
/* struct ibv_qp_init_attr:
   void *qp_context                 - A private context that the QP will be associated with 
   struct ibv_cq *send_cq          - The CQ to be associated with the QP’s Send Queue 
   struct ibv_cq *recv_cq          - The CQ to be associated with the QP’s Receive Queue 
   struct ibv_srq *srq              - Optional: if not NULL,  the SRQ to be associated with 
   struct ibv_qp_cap cap;          - The QP attributes to be created 
   enum ibv_qp_type qp_type        - The QP transport type 
   int  sq_sig_all        -Indication if every completed Send Request will generate a Work Completion
}; 
*/
    struct ibv_qp_init_attr qp_init_attr;
    memset(&qp_init_attr, 0, sizeof qp_init_attr);
    qp_init_attr.qp_type = IBV_QPT_RC;
    qp_init_attr.sq_sig_all = 1;
    qp_init_attr.send_cq = sendcq;     
    qp_init_attr.recv_cq = recvcq;
    qp_init_attr.cap.max_send_wr = config.sr_max;       
    qp_init_attr.cap.max_recv_wr = config.rr_max;
    qp_init_attr.cap.max_send_sge = config.send_sge_max;    // 每个 SGE 都会指向一个内存中的 buffer 用于读写
    qp_init_attr.cap.max_recv_sge = config.recv_sge_max;
    
    qp = ibv_create_qp(pd, &qp_init_attr);
    RDMA_CHECK_NP(qp, "qp_create");
    
    // 初始化套接字
   // sock = -1;

    // 请求事件非阻塞
    if(notify_event()){
        fprintf(stderr, "notify_event() fails\n");
        return;
    }
    

	local_props.addr   = (uintptr_t)recvbuf;            // 对面得到的地址为本地的接收地址
	local_props.rkey   = recvmr->rkey;
	local_props.qp_num = qp->qp_num;
    local_props.lid    = port_attr->lid;
	
	is_ready = false;


    return;
}

//销毁资源
RDMA::~RDMA(){
	if (qp) {
		if (ibv_destroy_qp(qp))
			fprintf(stderr, "failed to destroy QP\n");
	}
    if (recvmr) {
		if (ibv_dereg_mr(recvmr))
			fprintf(stderr, "failed to deregister Recv MR\n");
    }  
    if (sendmr) {
		if (ibv_dereg_mr(sendmr))
			fprintf(stderr, "failed to deregister Send MR\n");
	}  
    if(recvbuf)
        free(recvbuf);  
    if(sendbuf)
        free(sendbuf);
    
    if(device_attr)
        delete device_attr;
    if(port_attr)
        delete port_attr;
    
	if (sendcq) {
		if (ibv_destroy_cq(sendcq))
			fprintf(stderr, "failed to destroy Send CQ\n");
    }   
    if (recvcq) {
		if (ibv_destroy_cq(recvcq))
			fprintf(stderr, "failed to destroy Recv CQ\n");
	}
	if (pd) {
		if (ibv_dealloc_pd(pd))
			fprintf(stderr, "failed to deallocate PD\n");
	}
	if (ib_ctx) {
		if (ibv_close_device(ib_ctx))
			fprintf(stderr, "failed to close device context\n");
	}
	if (dev_list)
		ibv_free_device_list(dev_list);
/*	if (sock >= 0) {
		if (close(sock))
			fprintf(stderr, "failed to close socket\n");
	}
*/	
    if(channel){
        if (ibv_destroy_comp_channel(channel))
	        fprintf(stderr, "Error, ibv_destroy_comp_channel() failed\n");
    }
/*    
    if(RecvThread && !Instance_Num){
        RecvThread->join();
        delete RecvThread;
    }
 */  
	
   // fprintf(stdout, "~RDMA() executes successfully.\nAll resources have been destroyed.\n");
    
}
//inline内联函数，类似宏
// QP 状态机转换： RESET -> INIT
inline int RDMA::modify_qp_to_init(){
    struct ibv_qp_attr attr;
    int flags;
    int rc;

    memset(&attr, 0, sizeof attr);
    attr.qp_state = IBV_QPS_INIT;
    attr.port_num = config.ib_port;
    attr.pkey_index = 0;
    attr.qp_access_flags = IBV_ACCESS_REMOTE_WRITE;

    flags = IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS;

    rc = ibv_modify_qp(qp, &attr, flags);
    if(rc){
        fprintf(stderr, "failed to modify QP state to INIT\n");
        is_ready = false;
        return rc;
    }

    return 0;
}
// QP 状态机转换： INIT -> RTR
inline int RDMA::modify_qp_to_rtr(uint32_t remote_qpn, uint16_t dlid){
	struct ibv_qp_attr attr;
	int flags;
	int rc;

	memset(&attr, 0, sizeof(attr));

	attr.qp_state = IBV_QPS_RTR;
	attr.path_mtu = IBV_MTU_256;
	attr.dest_qp_num = remote_qpn;
	attr.rq_psn = 0;
	attr.max_dest_rd_atomic = 0;
	attr.min_rnr_timer = 0x12;
	attr.ah_attr.is_global = 0;
	attr.ah_attr.dlid = dlid;
	attr.ah_attr.sl = 0;
	attr.ah_attr.src_path_bits = 0;
    attr.ah_attr.port_num = config.ib_port;
/*    attr.timeout = 0x12;    // additional
	attr.qp_state = IBV_QPS_RTR;
	attr.path_mtu = IBV_MTU_256;
	attr.dest_qp_num = remote_props.qp_num;
	attr.rq_psn = 0;
	attr.max_dest_rd_atomic = 0;
	attr.min_rnr_timer = 0x6;
	attr.ah_attr.is_global = 0;
	attr.ah_attr.dlid = remote_props.lid;
	attr.ah_attr.sl = 0;
	attr.ah_attr.src_path_bits = 0;
	attr.ah_attr.port_num = config.ib_port;
*/
	flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN | 
		IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;

	rc = ibv_modify_qp(qp, &attr, flags);
	if (rc) {
        fprintf(stderr, "failed to modify QP state to RTR\n");
        is_ready = false;
		return rc;
	}

	return 0;
}
// QP 状态机转换： RTR -> RTS
inline int RDMA::modify_qp_to_rts(){
	struct ibv_qp_attr attr;
	int flags;
	int rc;


	/* do the following QP transition: RTR -> RTS */
	memset(&attr, 0, sizeof(attr));

	attr.qp_state = IBV_QPS_RTS;
	attr.timeout = 0x12;
	attr.retry_cnt = 6;
	attr.rnr_retry = 6;
	attr.sq_psn = 0;
    attr.max_rd_atomic = 0;
/*    attr.qp_state = IBV_QPS_RTS;
    attr.timeout = 12;           // change
    attr.min_rnr_timer = 0x6;   // change
    attr.retry_cnt = 7;         // change
    attr.rnr_retry = 7;         // change
    attr.sq_psn = 0;
    attr.max_rd_atomic = 0;
*/
 	flags = IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | 
		IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC;

	rc = ibv_modify_qp(qp, &attr, flags);
	if (rc) {
        fprintf(stderr, "failed to modify QP state to RTS\n");
        is_ready = false;
		return rc;
	}

	return 0;
}


//连接QP TCP交换必要数据 cm_con_data_t
int RDMA::connect_qp(){
	int rc;
	rc = modify_qp_to_init();
	if(rc){
    	fprintf(stderr, "change QP state to INT failed\n");
    	is_ready = false;
    	return rc;
    }
    /* QP 状态调整为 RTR，只具备接受能力 */
	rc = modify_qp_to_rtr(remote_props.qp_num, remote_props.lid);
	if (rc) {
        fprintf(stderr, "failed to modify QP state from RESET to RTS\n");
        is_ready = false;
		return rc;
	}

    /* QP 状态调整为 RTS，也具备接收能力 */
	rc = modify_qp_to_rts();
	if (rc) {
        fprintf(stderr, "failed to modify QP state from RESET to RTS\n");
        is_ready = false;
		return rc;
	}

    //fprintf(stdout, "QP state has changed to RTS\n");

	/* sync to make sure that both sides are in states that they can connect to prevent packet loose */
/*	if (sock_sync_ready(sock, server_name.empty())) {
        fprintf(stderr, "sync after QPs are were moved to RTS\n");
        is_ready = false;
		return 1;
    }
*/   
 

	//fprintf(stdout, "connect_qp() executes successfully.\n");


	return 0;
}



size_t RDMA::poll_completion(struct ibv_cq* cq){
	static int counter = 0;
	counter++;
	struct ibv_wc wc;
    int rc;
    do{
        rc = ibv_poll_cq(cq, 1, &wc);
        if(rc < 0){
            fprintf(stderr, "Poll CQ failed\n");
            return -1;
        }
  
    }while(rc == 0);

    if(rc == 0){
        fprintf(stderr, "Completion wasn't found in the CQ after timeout.\n");
        return -1;
    }

    if(wc.status != IBV_WC_SUCCESS){
        fprintf(stderr, "got bad completion with status: 0x%x, vendor syndrome: 0x%x\n",
                wc.status, wc.vendor_err);
        return -1;
    }

   
	//fprintf(stdout, "%d poll_completion() executes successfully.\n",counter);


    return (size_t)wc.imm_data;
}


/*  post_send & post_recv 网络 I/O 通信组
    ------------------------------------------------------------------------------------------------------------
    struct ibv_send_wr
        describes the Work Request to the Send Queue of the QP.
    struct ibv_sge{
        uint64_t        addr;       The address of the buffer to read from write to
        uint32_t        length;     The length of the buffer in bytes.
        uint32_t        lkey        The Local key of the Memory Region that this memory buffer was registered with
    };
        describes a scatter / gather entry 
    ------------------------------------------------------------------------------------------------------------
    int ibv_post_send(struct ibv_qp *qp, struct ibv_send_wr *sr,
                      struct ibv_send_wr **bad_sr);
    posts a linked list of Work Requests to the Send Queue of a QP
    wr: linked list of Work Request to be posted to the Send Queue of the Queue Pair
    bad_wr: a pointer to that will be filled with the first Work Request that its processing failed.
    Return: 0: On success.
    ------------------------------------------------------------------------------------------------------------
*/
int RDMA::post_send(void *msg, size_t msgsize){
    struct ibv_send_wr sr;
    struct ibv_sge sge;
    struct ibv_send_wr *bad_sr;
    int rc;
    
    memcpy(sendbuf, msg, msgsize);
   // fprintf(stdout, "%lu bytes: ", msgsize);
    //fprintf(stdout, "%s\n", (char*)msg);
 

    memset(&sge, 0, sizeof sge);
    sge.addr = (uintptr_t)sendbuf;
    sge.length = msgsize;
    sge.lkey = sendmr->lkey;
    
    memset(&sr, 0, sizeof sr);
    sr.next = NULL; // Address of the next Send Request. Should be NULL in the last Send Request
    sr.wr_id = 0;
    sr.sg_list = &sge;
    sr.num_sge = 1;
    sr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
    sr.send_flags = IBV_SEND_SOLICITED;
    sr.imm_data = msgsize;
    sr.wr.rdma.remote_addr = remote_props.addr;
    sr.wr.rdma.rkey = remote_props.rkey;
    
    rc = ibv_post_send(qp, &sr, &bad_sr);
    if(rc){
        fprintf(stderr, "failed to post SR\n");
        return 1;
    }

	//printf("post_send() executes successfully.\n");


    return 0;
    
}

int RDMA::post_recv(){
    struct ibv_recv_wr wr;
    struct ibv_sge sge;
    struct ibv_recv_wr *bad_wr;
    int rc;

    memset(&sge, 0, sizeof sge);
    sge.addr = (uintptr_t)recvbuf;         
    sge.length = config.sendbuf_size;
    sge.lkey = recvmr->lkey;

    memset(&wr, 0, sizeof wr);
    wr.next = NULL;
    wr.wr_id = 0;
    wr.sg_list = &sge;
    wr.num_sge = 1;

    rc = ibv_post_recv(qp, &wr, &bad_wr);
    if(rc){
        fprintf(stderr, "failed to post RR\n");
        return -1;
    }

   
    //printf("post_recv() executes successfully.\n");

    return 0;
    
}

size_t RDMA::Send(void *msg, size_t msgsize){
    post_send(msg, msgsize);
    poll_completion(sendcq);
    return msgsize;
}
/*
void RDMA::Receiving(){
	while(true){
		post_recv();
    	get_event();
    	poll_completion(recvcq);
		printf("RDMA_Recv() done\n");
		printf("current recv: %s\n",(char*)recvbuf);
	}
	return;
}
*/
size_t RDMA::Recv(){
	post_recv();
    get_event();
    size_t msgsize = poll_completion(recvcq);
	return msgsize;
}
/*  请求事件  --只有 Receiver Side 可以调用, 只需调用一次
    Send 使用 IMM 和 Solicited 事件通知模式，
    Receiver Side 需要用 get_event 和 notify_event 非阻塞调用写入
*/
int RDMA::notify_event(){
    if(ibv_req_notify_cq(recvcq, 1)){
        fprintf(stderr, "Coudn't request CQ notification\n");
		return 1;
	}

	#ifdef DEBUG
	fprintf(stdout, "ibv_req_notify_cq() executes successfully.\n");
	#endif

	/*  配置句柄使其非阻塞 */
	int flags = fcntl(channel->fd, F_GETFL);
	int rc = fcntl(channel->fd, F_SETFL, flags | O_NONBLOCK);
	if(rc < 0){
		fprintf(stderr, "Failed to change file descriptor of Completion Event Channel\n");
		return 1;
	}

    #ifdef DEBUG
	fprintf(stdout, "fcntl() executes successfully.\n");
	#endif

    return 0;
}

/*  响应事件  --只有 Receiver Side 可以调用，每次读取时均需调用
    Send 使用 IMM 和 Solicited 事件通知模式，
    Receiver Side 需要用 get_event 和 notify_event 配合非阻塞写入
    最后再次调用 notify 等待下次事件到来
*/
int RDMA::get_event(){
    /* 配置句柄使其非阻塞 */
	struct pollfd my_pollfd;
	int ms_timeout = 10;
	my_pollfd.fd      = channel->fd;
	my_pollfd.events  = POLLIN;
	my_pollfd.revents = 0;
    int rc;
	do {
        rc = poll(&my_pollfd, 1, ms_timeout);
	}while (rc == 0);
	if (rc < 0) {
        fprintf(stderr, "poll failed\n");
        return 1;
	}

	
	//fprintf(stdout, "%d poll() executes successfully.\n",counter);
	

	/*  事件处理  */
	struct ibv_cq *ev_cq;
	void *ev_ctx;
    ev_cq = recvcq;
	if(ibv_get_cq_event(channel, &ev_cq, &ev_ctx)){
        fprintf(stderr, "Failed to get cq_event\n");
		return 1;
	}
	ibv_ack_cq_events(ev_cq, 1);

    
	//fprintf(stdout, "ibv_ack_cq_events() executes successfully.\n");
	

    /* 等待下次完成事件 */
    if (ibv_req_notify_cq(recvcq, 0)) {
        fprintf(stderr, "Couldn't request CQ notification\n");
        return 1;
    }

    return 0;
}    


#endif

    

#RDMA_demo:
#构造
获取IB设备名单，属性，分配PD，创建事件channel ，创建CQs（sendcq，recvcq），分配内存，注册buffer（配给读写权限），创建qp……

#析构

#connect-qp
目的：交换信息(TCP)，调整QP机状态（此处需要配置）
1.TCP连接交换必要数据
server先开守护进程，client连接
2.调整QP状态（在init阶段对client和server的读写操作权限进行配置）

#send/recv
send:post + poll
recv:get_event + poll 

(->降低CPU消耗)事件处理配置（solicited event）1，notifyevent 2,getevent

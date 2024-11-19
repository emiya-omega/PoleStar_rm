#ifndef __IMPL_MQTT_H
#define __IMPL_MQTT_H
#include "stm32f1xx.h"
#include "MQTTClient.h"
#include "MQTTConnect.h"
#include "MQTTSubscribe.h"
#include "MQTTPublish.h"
#include "MQTTPacket.h"
#include "MQTTFormat.h"
#include "MQTTUnsubscribe.h"
#include "socket.h"
#include "impl_mqtt.h" // 确保包含正确的头文件

// 全局变量声明
extern int mqttstate;
extern int cnt_ping_not_response;
extern int cnt_sock_init;
extern uint8_t buf_pub[1024];
extern uint32_t ping_timestamp, now_timestamp;
enum MQTT_STATE {MQTT_INIT, MQTT_CONNOK, MQTT_SUBOK};


// 函数声明
// 函数：func_tcp_sock_send
// 功能：发送TCP数据包
// 参数：sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度
// 返回值：int
int func_tcp_sock_send(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend);
// 函数：func_tcp_sock_read
// 功能：读取TCP数据包
// 参数：sockno：套接字编号，buf_mqrecv：接收缓冲区，len_mqrecv：接收缓冲区长度
// 返回值：int
int func_tcp_sock_read(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqrecv);
// 函数：func_judge_timeout_ms
// 功能：判断超时时间
// 参数：timespan：时间间隔
// 返回值：void
void func_judge_timeout_ms(uint32_t *timespan);
// 函数：func_judge_mqtt_recvmsg_package_type
// 功能：判断接收到的MQTT消息包类型
// 参数：buf_mqrecv：接收缓冲区，len_mqrecv：接收缓冲区长度
// 返回值：uint8_t
uint8_t func_judge_mqtt_recvmsg_package_type(uint8_t *buf_mqrecv, uint16_t len_mqrecv);
// 函数：func_mqtt_client_dealwith_recvmsg
// 功能：处理接收到的MQTT消息
// 参数：sockno：套接字编号，buf_mqrecv：接收缓冲区，len_mqbuf：接收缓冲区长度，len_mqrecv：接收缓冲区长度
// 返回值：void
void func_mqtt_client_dealwith_recvmsg(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqbuf, uint16_t len_mqrecv);
// 函数：func_mqtt_client_connect_broker
// 功能：连接MQTT代理
// 参数：state：状态，sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度，conn_mqtt：MQTT连接数据
// 返回值：void
void func_mqtt_client_connect_broker(int *state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt);
// 函数：func_mqtt_client_ping_broker
// 功能：向MQTT代理发送心跳包
// 参数：state：状态，sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度
// 返回值：void
void func_mqtt_client_ping_broker(int *state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend);
// 函数：func_run_mqtt_publish
// 功能：发布MQTT消息
// 参数：sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度，topicName：主题名称，payload：负载，payloadlen：负载长度
// 返回值：int
int func_run_mqtt_publish(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTString topicName, uint8_t* payload, int payloadlen);
// 函数：func_mqtt_client_subtopic_from_broker
// 功能：从MQTT代理订阅主题
// 参数：sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度，count：主题数量，topicFilters：主题过滤器，requestedQoSs：请求的QoS
// 返回值：int
int func_mqtt_client_subtopic_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, int count, MQTTString topicFilters[], int requestedQoSs[]);
// 函数：func_mqtt_client_recvmsg_from_broker
// 功能：从MQTT代理接收消息
// 参数：sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度
// 返回值：void
void func_mqtt_client_recvmsg_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend);
// 函数：func_run_mqtt_tcpsock
// 功能：运行MQTT TCP套接字
// 参数：sockno：套接字编号，broker_ip：代理IP，broker_port：代理端口，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度，conn_mqtt：MQTT连接数据
// 返回值：uint8_t
uint8_t func_run_mqtt_tcpsock(uint8_t sockno, uint8_t *broker_ip, uint16_t broker_port, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt);
// 函数：func_run_mqtt_progress
// 功能：运行MQTT进度
// 参数：state：状态，sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度，conn_mqtt：MQTT连接数据
// 返回值：uint8_t
uint8_t func_run_mqtt_progress(int state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt);

#endif // __IMPL_MQTT_H

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
int func_tcp_sock_send(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend);
int func_tcp_sock_read(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqrecv);
void func_judge_timeout_ms(uint32_t *timespan);
uint8_t func_judge_mqtt_recvmsg_package_type(uint8_t *buf_mqrecv, uint16_t len_mqrecv);
void func_mqtt_client_dealwith_recvmsg(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqbuf, uint16_t len_mqrecv);
void func_mqtt_client_connect_broker(int *state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt);
void func_mqtt_client_ping_broker(int *state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend);
int func_run_mqtt_publish(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTString topicName, uint8_t* payload, int payloadlen);
int func_mqtt_client_subtopic_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, int count, MQTTString topicFilters[], int requestedQoSs[]);
void func_mqtt_client_recvmsg_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend);
uint8_t func_run_mqtt_tcpsock(uint8_t sockno, uint8_t *broker_ip, uint16_t broker_port, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt);
uint8_t func_run_mqtt_progress(int state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt);

#endif // __IMPL_MQTT_H

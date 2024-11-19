#include "impl_mqtt.h"

 
int mqttstate = MQTT_INIT;
int cnt_ping_not_response = 0;
int cnt_sock_init = 0;
uint8_t buf_pub[1024];
uint32_t ping_timestamp, now_timestamp;
 
 uint32_t get_systick_timestamp(void)
{
    return HAL_GetTick();
}

int func_tcp_sock_send(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend)
{
	if(getSn_SR(sockno) == SOCK_ESTABLISHED)
	{
		return send(sockno, buf_mqsend, len_mqsend);
	}
	return -1;		
}
 
int func_tcp_sock_read(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqrecv)
{
	if((getSn_SR(sockno) == SOCK_ESTABLISHED))
	{
		len_mqrecv = getSn_RX_RSR(sockno);
		if(len_mqrecv > 0)
		{
			return recv(sockno, buf_mqrecv, len_mqrecv);
		}
	}
	return -1; 
}
 
void func_judge_timeout_ms(uint32_t *timespan)
{
	HAL_Delay(1);
	*timespan = *timespan + 1;
}
 
uint8_t func_judge_mqtt_recvmsg_package_type(uint8_t *buf_mqrecv, uint16_t len_mqrecv)
{
	MQTTHeader header = {0};
	if(len_mqrecv > 0)
	{
		header.byte = buf_mqrecv[0];
		return header.bits.type;
	}
	return 0;
}
 
void func_mqtt_client_dealwith_recvmsg(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqbuf, uint16_t len_mqrecv)
{
	if(len_mqrecv > 0)
	{
		ping_timestamp = get_systick_timestamp();
		// package type to deal
		switch(func_judge_mqtt_recvmsg_package_type(buf_mqrecv, len_mqrecv))
		{
			case CONNACK:
			break;
			case PUBLISH://analysis msg
			{
				int rc;
				uint8_t buf_recv[1024];
				uint8_t* payload;
				int len_payload;
				unsigned char retained, dup;
				int qos;
				unsigned short packetid; 
				MQTTString topicrecv;
				MQTTString topicpub;
				payload = buf_recv;
				rc = MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicrecv, &payload, &len_payload, buf_mqrecv, len_mqrecv);
				if(rc == 1)
				{
					//TODO ...
					//TEST code
					topicpub.cstring = (char*)"mytopic";
					memset(buf_pub, 0, sizeof(buf_pub));
					func_run_mqtt_publish(sockno, buf_pub, sizeof(buf_pub), topicpub, payload, len_payload);
				}
			}			
			break;
			case PUBACK:
			break;
			case PUBREC://Qos2 msg receipt
			case PUBREL://Qos2 msg receipt
			case PUBCOMP://Qos2 msg receipt
			{
				unsigned char packettype, dup;
				unsigned short packetid; 
				if (MQTTDeserialize_ack(&packettype, &dup, &packetid, buf_mqrecv, len_mqbuf) == 1)
				{
					memset(buf_mqrecv, 0, len_mqbuf);
					len_mqrecv = MQTTSerialize_ack(buf_mqrecv, len_mqbuf, packettype, dup, packetid);
					if(len_mqrecv > 0)
					{
						func_tcp_sock_send(sockno, buf_mqrecv, len_mqrecv);
					}
				}
			}
			break;
			case SUBACK:
			case UNSUBACK:
			case PINGREQ:
			case PINGRESP:
			case DISCONNECT:
			break;
			default:
			break;
		}
	}
}
 
void func_mqtt_client_connect_broker(int *state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt)
{
	uint32_t timespan;
	int len_cont;
	int res;
	len_cont = MQTTSerialize_connect(buf_mqsend, len_mqsend, conn_mqtt);
	if(len_cont > 0)
	{
		res = func_tcp_sock_send(sockno, buf_mqsend, len_cont);
		if(res > 0)
		{
			timespan = 0;
			memset(buf_mqsend, 0, len_mqsend);//reuse buffer
			for(;;)
			{
				if((len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend)) > 3)
				{
					if(func_judge_mqtt_recvmsg_package_type(buf_mqsend, len_cont) == CONNACK) 
					{
						*state = MQTT_CONNOK;
						ping_timestamp = get_systick_timestamp();
					}
					break;
				}
				func_judge_timeout_ms(&timespan);
				if(timespan > 500)
				{
					break;
				}
			}
		}
	}
}
 
void func_mqtt_client_ping_broker(int *state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend)
{
	uint32_t timespan;
	int len_cont;
	int res;
	if(get_systick_timestamp() - ping_timestamp > 5*1000)
	{		
		len_cont = MQTTSerialize_pingreq(buf_mqsend, len_mqsend);
		if(len_cont > 0)
		{
			res = func_tcp_sock_send(sockno, buf_mqsend, len_cont);
			if(res > 0)
			{
				timespan = 0;
				memset(buf_mqsend, 0, len_mqsend);//reuse buffer
				for(;;)
				{
					len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend);
					// pingrsp or others' published msg 
					if(len_cont > 0)
					{
						//recv pingrsp
						ping_timestamp = get_systick_timestamp();
						cnt_ping_not_response = 0;
						
						// other type msg to deal with
						func_mqtt_client_dealwith_recvmsg(sockno, buf_mqsend, len_mqsend, len_cont);
						break;
					}
					
					func_judge_timeout_ms(&timespan);
					if(timespan > 10)
					{
						cnt_ping_not_response ++;
						if(cnt_ping_not_response > 1)
						{
							*state = MQTT_INIT;
							close(sockno);
							cnt_ping_not_response = 0;
						}
						break;
					}
				}
			}
			else
			{
				cnt_ping_not_response ++;
				if(cnt_ping_not_response > 2)
				{
					*state = MQTT_INIT;
					close(sockno);
					cnt_ping_not_response = 0;
				}
			}
		}
	}
}
 
int func_run_mqtt_publish(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTString topicName, uint8_t* payload, int payloadlen)
{
	int len;
	int rc;
	
	if(mqttstate >= MQTT_CONNOK)
	{
		len = MQTTSerialize_publish(buf_mqsend, len_mqsend, 0, 0, 0, 0, topicName, payload, payloadlen);
		if(len > 0)
		{
			memcpy(buf_pub, buf_mqsend, len);
			rc = func_tcp_sock_send(sockno, buf_pub, len);
			if(rc > 0)
			{
				ping_timestamp = get_systick_timestamp();
			}
			return rc;
		}
	}	
	return 0;
}
 
int func_mqtt_client_subtopic_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, int count,\
		MQTTString topicFilters[], int requestedQoSs[])
{
	uint32_t timespan;
	int len_cont;
	int res;
	if(mqttstate != MQTT_SUBOK)
	{
		len_cont = MQTTSerialize_subscribe(buf_mqsend, len_mqsend, 0, SUBSCRIBE, count, topicFilters, requestedQoSs);
		if(len_cont > 0)
		{
			res = func_tcp_sock_send(sockno, buf_mqsend, len_cont);
			if(res > 0)
			{
				timespan = 0;
				memset(buf_mqsend, 0, len_mqsend);//reuse buffer
				for(;;)
				{
					if((len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend)) > 0 && func_judge_mqtt_recvmsg_package_type(buf_mqsend, len_cont) == SUBACK)//nowtime, ignore other type msg 
					{
						mqttstate = MQTT_SUBOK;
						ping_timestamp = get_systick_timestamp();
						return 0;
					}
					func_judge_timeout_ms(&timespan);
					if(timespan > 500)
					{
						return -2;
					}
				}
			}
		}
	}
	return -1;
}
 
void func_mqtt_client_recvmsg_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend)
{
	int len_cont;
	memset(buf_mqsend, 0, len_mqsend);
	len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend);
	if(len_cont > 0)
	{
		func_mqtt_client_dealwith_recvmsg(sockno, buf_mqsend, len_mqsend, len_cont);
	}	
}
 
uint8_t func_run_mqtt_tcpsock(uint8_t sockno, uint8_t *broker_ip, uint16_t broker_port, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt)
{
	static uint16_t any_port = 50000;
	uint8_t res;
	
	switch(getSn_SR(sockno))
	{
		case SOCK_CLOSED:
		{
			close(sockno);
			socket(sockno, Sn_MR_TCP, any_port++, 0x00);
			cnt_sock_init++;
			if(cnt_sock_init > 30)
			{
				cnt_sock_init = 0;
				close(sockno);
				mqttstate = MQTT_INIT;
			}
			if(any_port > 64000)
			{
				any_port =50000;
			}
		}			
		break;
		case SOCK_INIT:
		{
			res = connect(sockno, broker_ip, broker_port);
			if(res)
			{
				//mqtt connect request
				mqttstate = func_run_mqtt_progress(mqttstate, sockno, buf_mqsend, len_mqsend, conn_mqtt);
			}
			else
			{
				if(cnt_ping_not_response > 0)
				{
					mqttstate = MQTT_INIT;
				}
			}
		}
		break;
		case SOCK_ESTABLISHED:
		{
			//run mqtt progress
			mqttstate = func_run_mqtt_progress(mqttstate, sockno, buf_mqsend, len_mqsend, conn_mqtt);
		}
		break;
		case SOCK_CLOSE_WAIT: 
		{
			mqttstate = MQTT_INIT;
			close(sockno);
		}
		break;
		default:
			break;
	}
	return mqttstate;
}
 
uint8_t func_run_mqtt_progress(int state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt)
{
	switch(state)
	{
		case MQTT_INIT:
		{
			func_mqtt_client_connect_broker(&state, sockno, buf_mqsend, len_mqsend, conn_mqtt);
		}
		break;
		case MQTT_CONNOK:
		{
			func_mqtt_client_ping_broker(&state, sockno, buf_mqsend, len_mqsend);
			if(state > MQTT_INIT)
			{
//				func_mqtt_client_subtopic_from_broker(&state, sockno, buf_mqsend, len_mqsend);
			}
		}
		break;
		case MQTT_SUBOK:
		{
			func_mqtt_client_ping_broker(&state, sockno, buf_mqsend, len_mqsend);
			if(state != MQTT_INIT)
			{
				func_mqtt_client_recvmsg_from_broker(sockno, buf_mqsend, len_mqsend);
			}
		}
		break;
		default:
		break;
	}
	return state;
}
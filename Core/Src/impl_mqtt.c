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

// 函数：func_tcp_sock_send
// 功能：发送TCP数据
// 参数：sockno：套接字编号；buf_mqsend：发送缓冲区；len_mqsend：发送缓冲区长度
// 返回值：发送成功返回发送的字节数，发送失败返回-1
int func_tcp_sock_send(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend)
{
	// 判断套接字状态是否为已连接
	if (getSn_SR(sockno) == SOCK_ESTABLISHED)
	{
		// 发送数据
		return send(sockno, buf_mqsend, len_mqsend);
	}
	// 发送失败
	return -1;
}

int func_tcp_sock_read(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqrecv)
{
	// 判断socket是否已经建立连接
	if ((getSn_SR(sockno) == SOCK_ESTABLISHED))
	{
		// 获取socket接收缓冲区中的数据长度
		len_mqrecv = getSn_RX_RSR(sockno);
		// 如果接收缓冲区中有数据
		if (len_mqrecv > 0)
		{
			// 从socket接收缓冲区中读取数据
			return recv(sockno, buf_mqrecv, len_mqrecv);
		}
	}
	// 如果socket没有建立连接或者接收缓冲区中没有数据，则返回-1
	return -1;
}

// 函数：判断是否超时
// 参数：timespan：时间间隔
void func_judge_timeout_ms(uint32_t *timespan)
{
	// 延时1ms
	HAL_Delay(1);
	// 时间间隔加1
	*timespan = *timespan + 1;
}

// 函数：判断mqtt接收到的消息包类型
// 参数：buf_mqrecv：mqtt接收到的消息包
//       len_mqrecv：mqtt接收到的消息包长度
// 返回值：消息包类型
uint8_t func_judge_mqtt_recvmsg_package_type(uint8_t *buf_mqrecv, uint16_t len_mqrecv)
{
	// 定义mqtt消息头
	MQTTHeader header = {0};
	// 如果消息包长度大于0
	if (len_mqrecv > 0)
	{
		// 将消息包的第一个字节赋值给mqtt消息头
		header.byte = buf_mqrecv[0];
		// 返回消息包类型
		return header.bits.type;
	}
	// 否则返回0
	return 0;
}
const char *DEVICE_ID = "dev_slimspark";

void handle_request_and_publish_response(uint8_t sockno, const char *request_topic, const char *response_topic, const uint8_t *payload, int payloadlen)
{
	printf("Received request on topic\n");
	// 解析请求的JSON数据
	cJSON *root = cJSON_Parse((const char *)payload);
	if (root == NULL)
	{
		printf("JSON解析失败，处理错误");
		return;
	}

	// 提取字段
	cJSON *mode = cJSON_GetObjectItemCaseSensitive(root, "mode");
	cJSON *requestId = cJSON_GetObjectItemCaseSensitive(root, "requestId");
	cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
	MQTTString topicpub;
	// TODO: 根据data中的type和working字段进行相应的操作
	if (strcmp(mode->valuestring, "work") == 0)
	{
	}
	// 构造响应的JSON数据
	cJSON *responseRoot = cJSON_CreateObject();
	cJSON_AddStringToObject(responseRoot, "id", DEVICE_ID);
	if (requestId != NULL && requestId->type == cJSON_String)
	{
		cJSON_AddStringToObject(responseRoot, "requestId", requestId->valuestring);
	}
	cJSON_AddBoolToObject(responseRoot, "success", true); // 假设操作成功

	// 序列化响应的JSON数据
	char *responseStr = cJSON_PrintUnformatted(responseRoot);
	if (responseStr == NULL)
	{
		printf("JSON序列化失败，处理错误");
		cJSON_Delete(responseRoot);
		return;
	}

	// 发布响应消息

	// topicpub.cstring = (char *)response_topic;
	// topicpub.cstring = (char *)response_topic;
	printf("Publishing response on topic\n");
	topicpub.cstring = (char *)"clock/work/response";
	memset(buf_pub, 0, sizeof(buf_pub));

	func_run_mqtt_publish(sockno, buf_pub, sizeof(buf_pub), topicpub, (uint8_t *)responseStr, strlen(responseStr));

	// 清理
	cJSON_Delete(responseRoot);
	cJSON_free(responseStr);
}

void func_mqtt_client_dealwith_recvmsg(uint8_t sockno, uint8_t *buf_mqrecv, uint16_t len_mqbuf, uint16_t len_mqrecv)
{
	// 如果接收到的消息长度大于0
	if (len_mqrecv > 0)
	{
		// 获取当前系统时间戳
		ping_timestamp = get_systick_timestamp();
		// package type to deal
		switch (func_judge_mqtt_recvmsg_package_type(buf_mqrecv, len_mqrecv))
		{
		case CONNACK:
			break;
		case PUBLISH: // analysis msg
		{
			int rc;
			uint8_t buf_recv[1024];
			uint8_t *payload;
			int len_payload;
			unsigned char retained, dup;
			int qos;
			unsigned short packetid;
			MQTTString topicrecv;
			MQTTString topicpub;
			payload = buf_recv;
			// 解析接收到的publish消息
			rc = MQTTDeserialize_publish(&dup, &qos, &retained, &packetid, &topicrecv, &payload, &len_payload, buf_mqrecv, len_mqrecv);
			if (rc == 1)
			{

				// TODO ...
				// TEST code

				// TODO ...
				// TEST code
				// topicpub.cstring = (char*)"clock/work/response";
				printf("clock/work/request");
				// handle_request_and_publish_response(sockno, (char*)"clock/work/request", (char*)"clock/work/response", payload, len_payload);

				cJSON *root = cJSON_Parse((const char *)payload);
				if (root == NULL)
				{
					printf("JSON解析失败，处理错误");
					return;
				}

				// 提取字段
				cJSON *mode = cJSON_GetObjectItemCaseSensitive(root, "mode");
				cJSON *requestId = cJSON_GetObjectItemCaseSensitive(root, "requestId");
				cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
				if (strcmp(mode->valuestring, "work") == 0)
				{
					if (data != NULL && cJSON_IsObject(data))
					{
						cJSON *type = cJSON_GetObjectItemCaseSensitive(data, "type");
						cJSON *working = cJSON_GetObjectItemCaseSensitive(data, "working");

						if (type != NULL && cJSON_IsString(type) && working != NULL && cJSON_IsString(working))
						{

							// 示例逻辑：根据type和working的值进行操作
							if (strcmp(type->valuestring, "0") == 0) // 钟表
							{
								if (strcmp(working->valuestring, "0") == 0) // 停止
								{
									// 处理钟表停止的逻辑
									printf("Handling clock stop\n");
								}
								else if (strcmp(working->valuestring, "1") == 0) // 运行
								{
									// 处理钟表运行的逻辑
									printf("Handling clock run\n");
								}
							}
							else if (strcmp(type->valuestring, "1") == 0) // 扩大机
							{
								if (strcmp(working->valuestring, "0") == 0) // 停止
								{
									// 处理扩大机停止的逻辑(音频静音)
									HAL_GPIO_WritePin(GPIOF, XMT_Pin, GPIO_PIN_SET);
									printf("Handling amplifier stop\n");
								}
								else if (strcmp(working->valuestring, "1") == 0) // 运行
								{
									// 处理扩大机运行的逻辑（静音解除）
									HAL_GPIO_WritePin(GPIOF, XMT_Pin, GPIO_PIN_RESET);
									printf("Handling amplifier run\n");
								}
							}
							else if (strcmp(type->valuestring, "2") == 0) // 灯光
							{
								if (strcmp(working->valuestring, "0") == 0) // 停止
								{
									// 处理灯光停止的逻辑（继电器断）
									HAL_GPIO_WritePin(Relay_IN_GPIO_Port, Relay_IN_Pin, GPIO_PIN_SET);
									printf("Handling light stop\n");
								}
								else if (strcmp(working->valuestring, "1") == 0) // 运行
								{
									// 处理灯光运行的逻辑（继电器通）
									HAL_GPIO_WritePin(Relay_IN_GPIO_Port, Relay_IN_Pin, GPIO_PIN_RESET);
									printf("Handling light run\n");
								}
							}
						}
					}
					topicpub.cstring = (char *)"clock/work/response";
				}
				else if (strcmp(mode->valuestring, "clock") == 0)
				{
					uint8_t batteryVoltage; // 电池电压
					uint8_t clockTemperature; // 设备温度
					uint8_t workingVoltage;   // 工作电压
					uint8_t workingCurrent;   // 工作电流
					uint8_t workingDays;      // 运行天数
					uint8_t hardwareVersion;  // 硬件版本
					uint8_t softwareVersion;  // 软件版本
					uint8_t longitude;        // 经度
					uint8_t latitude;         // 纬度
					// 处理 clock 模式的逻辑
            		// TODO: 获取设备状态信息，例如电池电压、设备温度等
					printf("Handling clock mode\n");
					topicpub.cstring = (char *)"clock/clock/response";
				}
				else if (strcmp(mode->valuestring, "changeTime") == 0)
				{

					topicpub.cstring = (char *)"clock/changeTime/response";
				}
				else if (strcmp(mode->valuestring, "changeSection") == 0)
				{

					topicpub.cstring = (char *)"clock/changeSection/response";
				}

				cJSON *responseRoot = cJSON_CreateObject();
				cJSON_AddStringToObject(responseRoot, "id", DEVICE_ID);
				if (requestId != NULL && requestId->type == cJSON_String)
				{
					cJSON_AddStringToObject(responseRoot, "requestId", requestId->valuestring);
				}
				cJSON_AddBoolToObject(responseRoot, "success", true); // 假设操作成功
				if (strcmp(mode->valuestring, "clock") == 0)
				{
					cJSON *dataObj = cJSON_AddObjectToObject(responseRoot, "data");
					cJSON_AddStringToObject(dataObj, "batteryVoltage", "获取的电池电压");
					cJSON_AddStringToObject(dataObj, "clockTemperature", "获取的设备温度");
					cJSON_AddStringToObject(dataObj, "workingVoltage", "获取的工作电压");
					cJSON_AddStringToObject(dataObj, "workingCurrent", "获取的工作电流");
					cJSON_AddStringToObject(dataObj, "workingDays", "获取的运行天数");
					cJSON_AddStringToObject(dataObj, "hardwareVersion", "获取的硬件版本");
					cJSON_AddStringToObject(dataObj, "softwareVersion", "获取的软件版本");
					cJSON_AddStringToObject(dataObj, "longitude", "获取的经度");
					cJSON_AddStringToObject(dataObj, "latitude", "获取的纬度");
				}

				// 序列化响应的JSON数据
				char *responseStr = cJSON_PrintUnformatted(responseRoot);
				if (responseStr == NULL)
				{
					printf("JSON序列化失败，处理错误");
					cJSON_Delete(responseRoot);
					return;
				}
				// 发布消息
				memset(buf_pub, 0, sizeof(buf_pub));
				memset(buf_mqrecv, 0, len_mqbuf);

				func_run_mqtt_publish(sockno, buf_pub, sizeof(buf_pub), topicpub, responseStr, strlen(responseStr));

				len_mqrecv = 0;
				cJSON_free(responseStr); // 清理
				cJSON_Delete(responseRoot);

				//  发布消息d
				// func_run_mqtt_publish(sockno, buf_pub, sizeof(buf_pub), topicpub, payload, len_payload);
			}
		}
		break;
		case PUBACK:
			break;
		case PUBREC:  // Qos2 msg receipt
		case PUBREL:  // Qos2 msg receipt
		case PUBCOMP: // Qos2 msg receipt
		{
			unsigned char packettype, dup;
			unsigned short packetid;
			// 解析接收到的ack消息
			if (MQTTDeserialize_ack(&packettype, &dup, &packetid, buf_mqrecv, len_mqbuf) == 1)
			{
				memset(buf_mqrecv, 0, len_mqbuf);
				// 序列化ack消息
				len_mqrecv = MQTTSerialize_ack(buf_mqrecv, len_mqbuf, packettype, dup, packetid);
				if (len_mqrecv > 0)
				{
					// 发送ack消息
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
	// 定义变量timespan，len_cont，res
	uint32_t timespan;
	int len_cont;
	int res;
	// 调用MQTTSerialize_connect函数，将conn_mqtt中的数据序列化到buf_mqsend中，返回序列化后的数据长度
	len_cont = MQTTSerialize_connect(buf_mqsend, len_mqsend, conn_mqtt);
	// 如果序列化成功
	if (len_cont > 0)
	{
		// 调用func_tcp_sock_send函数，将buf_mqsend中的数据发送到sockno对应的socket中，返回发送的数据长度
		res = func_tcp_sock_send(sockno, buf_mqsend, len_cont);
		// 如果发送成功
		if (res > 0)
		{
			// 重置timespan为0
			timespan = 0;
			// 重置buf_mqsend为0
			memset(buf_mqsend, 0, len_mqsend); // reuse buffer
			// 循环读取socket中的数据
			for (;;)
			{
				// 调用func_tcp_sock_read函数，从sockno对应的socket中读取数据到buf_mqsend中，返回读取的数据长度
				if ((len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend)) > 3)
				{
					// 调用func_judge_mqtt_recvmsg_package_type函数，判断buf_mqsend中的数据类型，如果是CONNACK，则将state设置为MQTT_CONNOK，并记录ping_timestamp
					if (func_judge_mqtt_recvmsg_package_type(buf_mqsend, len_cont) == CONNACK)
					{
						*state = MQTT_CONNOK;
						ping_timestamp = get_systick_timestamp();
					}
					// 跳出循环
					break;
				}
				// 调用func_judge_timeout_ms函数，判断是否超时，如果超时则跳出循环
				func_judge_timeout_ms(&timespan);
				if (timespan > 500)
				{
					break;
				}
			}
		}
	}
}

void func_mqtt_client_ping_broker(int *state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend)
{
	// 获取当前时间戳
	uint32_t timespan;
	int len_cont;
	int res;
	if (get_systick_timestamp() - ping_timestamp > 5 * 1000)
	{
		// 序列化ping请求
		len_cont = MQTTSerialize_pingreq(buf_mqsend, len_mqsend);
		if (len_cont > 0)
		{
			// 发送ping请求
			res = func_tcp_sock_send(sockno, buf_mqsend, len_cont);
			if (res > 0)
			{
				timespan = 0;
				memset(buf_mqsend, 0, len_mqsend); // reuse buffer
				for (;;)
				{
					len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend);
					// pingrsp or others' published msg
					if (len_cont > 0)
					{
						// recv pingrsp
						ping_timestamp = get_systick_timestamp();
						cnt_ping_not_response = 0;

						// other type msg to deal with
						func_mqtt_client_dealwith_recvmsg(sockno, buf_mqsend, len_mqsend, len_cont);
						break;
					}

					func_judge_timeout_ms(&timespan);
					if (timespan > 10)
					{
						cnt_ping_not_response++;
						if (cnt_ping_not_response > 1)
						{
							*state = MQTT_INIT;
							close1(sockno);
							cnt_ping_not_response = 0;
						}
						break;
					}
				}
			}
			else
			{
				cnt_ping_not_response++;
				if (cnt_ping_not_response > 2)
				{
					*state = MQTT_INIT;
					close1(sockno);
					cnt_ping_not_response = 0;
				}
			}
		}
	}
}

int func_run_mqtt_publish(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTString topicName, uint8_t *payload, int payloadlen)
{
	// 定义变量len和rc
	int len;
	int rc;

	// 如果mqttstate大于等于MQTT_CONNOK
	if (mqttstate >= MQTT_CONNOK)
	{
		// 调用MQTTSerialize_publish函数，将buf_mqsend中的数据序列化为publish消息，并将长度赋值给len
		len = MQTTSerialize_publish(buf_mqsend, len_mqsend, 0, 0, 0, 0, topicName, payload, payloadlen);
		// 如果len大于0
		if (len > 0)
		{
			// 将buf_mqsend中的数据复制到buf_pub中
			memcpy(buf_pub, buf_mqsend, len);
			// 调用func_tcp_sock_send函数，将buf_pub中的数据发送到sockno指定的socket，并将返回值赋值给rc
			rc = func_tcp_sock_send(sockno, buf_pub, len);
			// 如果rc大于0
			if (rc > 0)
			{
				// 获取当前系统时间，并赋值给ping_timestamp
				ping_timestamp = get_systick_timestamp();
			}
			// 返回rc
			return rc;
		}
	}
	// 返回0
	return 0;
}

int func_mqtt_client_subtopic_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, int count,
										  MQTTString topicFilters[], int requestedQoSs[])
{
	// 定义变量timespan，len_cont，res
	uint32_t timespan;
	int len_cont;
	int res;
	// 如果mqttstate不等于MQTT_SUBOK
	if (mqttstate != MQTT_SUBOK)
	{
		// 调用MQTTSerialize_subscribe函数，将buf_mqsend，len_mqsend，0，SUBSCRIBE，count，topicFilters，requestedQoSs作为参数传入，将返回值赋给len_cont
		len_cont = MQTTSerialize_subscribe(buf_mqsend, len_mqsend, 0, SUBSCRIBE, count, topicFilters, requestedQoSs);
		// 如果len_cont大于0
		if (len_cont > 0)
		{
			// 调用func_tcp_sock_send函数，将sockno，buf_mqsend，len_cont作为参数传入，将返回值赋给res
			res = func_tcp_sock_send(sockno, buf_mqsend, len_cont);
			// 如果res大于0
			if (res > 0)
			{
				// 将timespan赋值为0
				timespan = 0;
				// 调用memset函数，将buf_mqsend，len_mqsend，0作为参数传入，将buf_mqsend清零
				memset(buf_mqsend, 0, len_mqsend); // reuse buffer
				// 进入无限循环
				for (;;)
				{
					// 调用func_tcp_sock_read函数，将sockno，buf_mqsend，len_mqsend作为参数传入，将返回值赋给len_cont
					if ((len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend)) > 0 && func_judge_mqtt_recvmsg_package_type(buf_mqsend, len_cont) == SUBACK) // nowtime, ignore other type msg
					{
						// 将mqttstate赋值为MQTT_SUBOK
						mqttstate = MQTT_SUBOK;
						// 调用get_systick_timestamp函数，将返回值赋给ping_timestamp
						ping_timestamp = get_systick_timestamp();
						// 返回0
						return 0;
					}
					// 调用func_judge_timeout_ms函数，将timespan作为参数传入
					func_judge_timeout_ms(&timespan);
					// 如果timespan大于500
					if (timespan > 500)
					{
						// 返回-2
						return -2;
					}
				}
			}
		}
	}
	// 返回-1
	return -1;
}

// 函数：从代理接收消息
// 参数：sockno：套接字编号，buf_mqsend：发送缓冲区，len_mqsend：发送缓冲区长度
void func_mqtt_client_recvmsg_from_broker(uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend)
{
	// 定义变量：len_cont：接收到的数据长度
	int len_cont;
	// 将发送缓冲区清零
	memset(buf_mqsend, 0, len_mqsend);
	// 从套接字中读取数据
	len_cont = func_tcp_sock_read(sockno, buf_mqsend, len_mqsend);
	// 如果接收到的数据长度大于0
	if (len_cont > 0)
	{
		// 处理接收到的数据
		func_mqtt_client_dealwith_recvmsg(sockno, buf_mqsend, len_mqsend, len_cont);
	}
}

uint8_t func_run_mqtt_tcpsock(uint8_t sockno, uint8_t *broker_ip, uint16_t broker_port, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt)
{
	// 定义一个静态变量，用于保存端口号
	static uint16_t any_port = 50000;
	uint8_t res;

	// 根据sockno获取socket的状态
	switch (getSn_SR(sockno))
	{
	// 如果socket状态为关闭
	case SOCK_CLOSED:
	{
		// 关闭socket
		close1(sockno);
		// 创建一个新的socket
		socket(sockno, Sn_MR_TCP, any_port++, 0x00);
		// 初始化socket计数器
		cnt_sock_init++;
		// 如果socket计数器大于30
		if (cnt_sock_init > 30)
		{
			// 重置socket计数器
			cnt_sock_init = 0;
			// 关闭socket
			close1(sockno);
			// 设置mqtt状态为初始化
			mqttstate = MQTT_INIT;
		}
		// 如果端口号大于64000
		if (any_port > 64000)
		{
			// 重置端口号
			any_port = 50000;
		}
	}
	break;
	// 如果socket状态为初始化
	case SOCK_INIT:
	{
		// 连接到broker
		res = connect(sockno, broker_ip, broker_port);
		// 如果连接成功
		if (res)
		{
			// mqtt connect request
			mqttstate = func_run_mqtt_progress(mqttstate, sockno, buf_mqsend, len_mqsend, conn_mqtt);
		}
		else
		{
			if (cnt_ping_not_response > 0)
			{
				mqttstate = MQTT_INIT;
			}
		}
	}
	break;
	case SOCK_ESTABLISHED:
	{
		// run mqtt progress
		mqttstate = func_run_mqtt_progress(mqttstate, sockno, buf_mqsend, len_mqsend, conn_mqtt);
	}
	break;
	case SOCK_CLOSE_WAIT:
	{
		mqttstate = MQTT_INIT;
		close1(sockno);
	}
	break;
	default:
		break;
	}
	return mqttstate;
}

uint8_t func_run_mqtt_progress(int state, uint8_t sockno, uint8_t *buf_mqsend, uint16_t len_mqsend, MQTTPacket_connectData *conn_mqtt)
{
	// 根据状态执行不同的操作
	switch (state)
	{
	case MQTT_INIT:
	{
		// 连接到MQTT代理
		func_mqtt_client_connect_broker(&state, sockno, buf_mqsend, len_mqsend, conn_mqtt);
	}
	break;
	case MQTT_CONNOK:
	{
		// 向MQTT代理发送ping请求
		func_mqtt_client_ping_broker(&state, sockno, buf_mqsend, len_mqsend);
		// 如果状态大于MQTT_INIT，则订阅主题
		if (state > MQTT_INIT)
		{
			//				func_mqtt_client_subtopic_from_broker(&state, sockno, buf_mqsend, len_mqsend);
		}
	}
	break;
	case MQTT_SUBOK:
	{
		// 向MQTT代理发送ping请求
		func_mqtt_client_ping_broker(&state, sockno, buf_mqsend, len_mqsend);
		// 如果状态不等于MQTT_INIT，则从MQTT代理接收消息
		if (state != MQTT_INIT)
		{
			func_mqtt_client_recvmsg_from_broker(sockno, buf_mqsend, len_mqsend);
		}
	}
	break;
	default:
		break;
	}
	// 返回状态
	return state;
}
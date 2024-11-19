#include "httppost.h"
 
// 函数用于打包HTTP POST请求的body
uint16_t func_pack_httppost_body(char *buff_body, char *productId, char *deviceSn, char *deviceMac, char *devicePassword, char *hardwareVersion)
{
	// 定义变量len，用于存储返回的字符串长度
	uint16_t len;
	// 使用sprintf函数将参数中的字符串格式化到buff_body中
	len = sprintf(buff_body, "a=%s&b=%s&c=%s&d=%s&e=%s",\
		productId, deviceSn, deviceMac, devicePassword, hardwareVersion);
	// 返回字符串长度
	return len;
}
 
uint16_t func_pack_httppost_head_body(char *buff_post, char *url_tail, uint8_t *host, uint16_t port, char *body, uint16_t body_len)
{
	// 定义变量len，用于存储返回的字符串长度
	uint16_t len;
	// 使用sprintf函数将HTTP POST请求头和请求体打包到buff_post中
	len = sprintf(buff_post, "POST %s HTTP/1.1\r\n"
		"Connection: close\r\n"
		"User-Agent:W5100s\r\n"
		"Content-Type:application/x-www-form-urlencoded\r\n"
		"Host: %d.%d.%d.%d:%d\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s\r\n", url_tail, host[0], host[1], host[2], host[3], port, body_len, body
	);
	// 返回字符串长度
	return len;
}
 
static uint16_t local_port = 50000;
 
uint8_t func_http_post(uint8_t sock_no, uint8_t *rip, uint16_t port, char *buf_post, uint16_t len_post,char *buf_recv, uint16_t *len_recv, uint16_t timeout_ms)
{
	// 定义变量
	uint16_t cnt, len;	
	char *body_cont;
	cnt = 0;
	
	// 无限循环
	for(;;)
	{
		// 根据socket状态进行不同的操作
		switch(getSn_SR(sock_no))
		{
			case SOCK_INIT:
				// 连接服务器
				connect(sock_no, rip, port);
			break;
			case SOCK_ESTABLISHED:
				// 发送POST请求
				send(sock_no, (uint8_t*)buf_post, len_post); 
				// 如果有连接中断，清除中断标志
				if(getSn_IR(sock_no) & Sn_IR_CON)   					
				{
					setSn_IR(sock_no, Sn_IR_CON);
				}
				// 获取接收缓冲区中的数据长度
				len = getSn_RX_RSR(sock_no);
				// 如果有数据，接收数据
				if(len > 0)
				{
					// 清空接收缓冲区
					memset(buf_recv, 0, len_post);
					// 接收数据
					len = recv(sock_no, (uint8_t*)buf_recv, len);
					// 查找HTTP响应状态码
					body_cont = strstr((char*)buf_recv, "HTTP/1.1 200");
					// 如果状态码不是200，返回错误
					if(body_cont == NULL)
					{
						return 2;
					}
					// 查找响应体
					body_cont = strstr((char*)buf_recv, "\r\n\r\n");
					// 如果找到响应体，提取响应体内容
					if(body_cont != NULL)
					{
						len = strlen(body_cont) - 4;
						memcpy(buf_recv, body_cont + 4, len);
						buf_recv[len] = '\0';
						*len_recv = len;
						// 关闭socket
						close(sock_no);
						return 0;
					}					
				}				
			break;
			case SOCK_CLOSE_WAIT:
				// 关闭socket
				close(sock_no);
			break;
			case SOCK_CLOSED:
				// 创建socket
				socket(sock_no, Sn_MR_TCP, local_port++, Sn_MR_ND);
				// 如果本地端口超过64000，重置为50000
				if(local_port > 64000)
				{
					local_port = 50000;
				}
			break;
		}
		
		// 计数器加1
		cnt ++;
		// 如果计数器超过超时时间，关闭socket并返回错误
		if(cnt >= timeout_ms)
		{
			close(sock_no);
			return 1;
		}
		// 延时1ms
		wiz_delay_ms(1);
	}
}
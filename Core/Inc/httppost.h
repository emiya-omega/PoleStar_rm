#ifndef __HTTPPOST_H
#define __HTTPPOST_H

#include "stm32f1xx.h"
#include "w5100s.h"
#include "socket.h"
#include "string.h"
#include "stdio.h"
#endif
uint16_t func_pack_httppost_body(char *buff_body, char *productId, char *deviceSn, char *deviceMac, char *devicePassword, char *hardwareVersion);
uint16_t func_pack_httppost_head_body(char *buff_post, char *url_tail, uint8_t *host, uint16_t port, char *body, uint16_t body_len);
uint8_t func_http_post(uint8_t sock_no, uint8_t *rip, uint16_t port, char *buf_post, uint16_t len_post,char *buf_recv, uint16_t *len_recv, uint16_t timeout_ms);

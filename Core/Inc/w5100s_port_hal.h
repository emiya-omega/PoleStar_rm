#ifndef _W5100s_PORT_HAL_
#define _W5100s_PORT_HAL_
 
#include "wizchip_conf.h"
#include "stm32f1xx.h"
#include <string.h>
#include <stdio.h>
 
#define W5100s_SPI_HANDLE    hspi3
#define W5100s_CS_PORT       GPIOA
#define W5100s_CS_PIN        GPIO_PIN_15
#define W5100s_RST_PORT      GPIOA
#define W5100s_RST_PIN       GPIO_PIN_12


 
/* 定义该宏则表示使用自动协商模式，取消则设置为100M全双工模式 */
#define USE_AUTONEGO
 
/* 定义该宏则表示在初始化网络信息时设置DHCP */
//#define USE_DHCP
 
extern SPI_HandleTypeDef W5100s_SPI_HANDLE;
 
void do_tcpc(void);


void W5100s_ChipInit(void);
#endif
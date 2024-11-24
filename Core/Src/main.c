/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "i2s.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include  <errno.h>
#include  <sys/unistd.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "wave.h"

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main()
{

  /* USER CODE BEGIN 1 */
	uint8_t res;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_USART1_UART_Init();
  MX_I2S2_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  MX_SPI3_Init();
  /* USER CODE BEGIN 2 */
  W5100s_ChipInit();//初始化W5100s


// 定义MQTT代理IP地址
	uint8_t ip_broker[] = {10, 100, 115, 173};
	// 定义MQTT代理端口
	uint16_t port_broker = 1883;
	// 定义MQTT发送缓冲区``````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````````〔筆畫〕〔筆畫〕`
	uint8_t buf_mqtt_send[1024];
	// 定义MAC地址
	uint8_t mac[6]={0x78, 0x83, 0x68, 0x88, 0x56, 0x72 };
	// 定义MQTT状态
	int mqtt_stat;
	
	// 定义MQTT订阅主题
	MQTTString sub_topic  = MQTTString_initializer;
	MQTTString sub_topic2 = MQTTString_initializer;
	MQTTString sub_topic3 = MQTTString_initializer;
	MQTTString sub_topic4 = MQTTString_initializer;
  MQTTString sub_topic5 = MQTTString_initializer;
  MQTTString sub_topic6 = MQTTString_initializer;
  MQTTString sub_topic7 = MQTTString_initializer;

	MQTTString sub_topics[8];
	// 定义MQTT订阅主题的QoS
	int nums_sub_topic_qoss[8] = {0,0,0,0,0,0,0};
	// 定义字符串
	char stpc_str[64] = {'t', 'c'};
	
	// 定义MQTT连接数据
	MQTTPacket_connectData conn_mqtt = MQTTPacket_connectData_initializer;
	// 设置遗嘱标志
	conn_mqtt.willFlag = 0;
	// 设置MQTT版本
	conn_mqtt.MQTTVersion = 3;
	// 设置客户端ID
	conn_mqtt.clientID.cstring = (char*)"dev_slimspark";
	// 设置用户名
	conn_mqtt.username.cstring = (char*)"";
	// 设置密码
	conn_mqtt.password.cstring = (char*)"";
	// 设置保活时间
	conn_mqtt.keepAliveInterval = 60;
	// 设置清理会话
	conn_mqtt.cleansession = 1;	
	
	// 复制字符串
	memcpy(stpc_str, (char*)"subtopic", strlen("subtopic"));
	// 设置订阅主题
	sub_topic.cstring = stpc_str;
	sub_topics[0] = sub_topic;
	sub_topic2.cstring = "clock/clock/response";
	sub_topics[1] = sub_topic2;
	sub_topic3.cstring = "clock/work/request";
	sub_topics[2] = sub_topic3;
	sub_topic4.cstring = "clock/changeTime/request";
	sub_topics[3] = sub_topic4;
  sub_topic5.cstring = "clock/changeTime/response";
	sub_topics[4] = sub_topic5;
  sub_topic6.cstring = "clock/changeSection/request";
	sub_topics[5] = sub_topic6;
  sub_topic7.cstring = "clock/changeSection/response";
	sub_topics[6] = sub_topic7;

 if(retSD == 0){
		printf("MX_FATFS_Init OK\r\n");
		if(f_mount(&SDFatFS, (TCHAR const*)SDPath, 0) != FR_OK){
			printf("f_mount failed\r\n");
		}
    
	}
  				memset(buf_mqtt_send, 0, sizeof(buf_mqtt_send));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // do_tcpc();//TCP客户端处理函数
    mqtt_stat = func_run_mqtt_tcpsock(2, ip_broker, port_broker, buf_mqtt_send, sizeof(buf_mqtt_send), &conn_mqtt);
		if(mqtt_stat >= MQTT_CONNOK)
		{
			if(mqtt_stat == MQTT_CONNOK)
			{
				memset(buf_mqtt_send, 0, sizeof(buf_mqtt_send));
				func_mqtt_client_subtopic_from_broker(2, buf_mqtt_send, sizeof(buf_mqtt_send), 4,	sub_topics, nums_sub_topic_qoss);
 
			}
			
		}		
		HAL_Delay(500);

/**   	mqtt_stat = func_run_mqtt_tcpsock(2, ip_broker, port_broker, buf_mqtt_send, sizeof(buf_mqtt_send), &conn_mqtt);
      		memset(buf_mqtt_send, 0, sizeof(buf_mqtt_send));
		// 调用func_run_mqtt_tcpsock函数，参数为2，ip_broker，port_broker，buf_mqtt_send，sizeof(buf_mqtt_send)，conn_mqtt，返回mqtt_stat
		if(mqtt_stat >= MQTT_CONNOK)
		{
     // func_mqtt_client_recvmsg_from_broker(2, buf_mqtt_send, sizeof(buf_mqtt_send));
       // func_mqtt_client_ping_broker(&mqtt_stat, 2, buf_mqtt_send, sizeof(buf_mqtt_send));
			// 如果mqtt_stat等于MQTT_CONNOK，则执行以下代码

			if(mqtt_stat == MQTT_CONNOK)
			{
				// 如果mqtt_stat等于MQTT_CONNOK，则执行以下代码
				memset(buf_mqtt_send, 0, sizeof(buf_mqtt_send));
				// 将buf_mqtt_send清零
				func_mqtt_client_subtopic_from_broker(2, buf_mqtt_send, sizeof(buf_mqtt_send), 7,	sub_topics, nums_sub_topic_qoss);
 
				// 调用func_mqtt_client_subtopic_from_broker函数，参数为2，buf_mqtt_send，sizeof(buf_mqtt_send)，4，sub_topics，nums_sub_topic_qoss
			}

		}	*/
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2S2;
  PeriphClkInit.I2s2ClockSelection = RCC_I2S2CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

int _write(int file, char *data, int len)// printf重定向
{
   if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
   {
      errno = EBADF;
      return -1;
   }

   // arbitrary timeout 1000
   HAL_StatusTypeDef status =
      HAL_UART_Transmit(&huart1, (uint8_t*)data, len, 1000);

   // return # of bytes written - as best we can tell
   return (status == HAL_OK ? len : 0);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  printf("Error_Handler\r\n");
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the fi ,,,,,,,,,,,,,,,,,le name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

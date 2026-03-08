/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "lora.h"
#include "gateway_status.h"
#include "shumeipai.h"
#include "bobao.h"
#include "lte_api.h"
#include "lte_onenet.h"
#include "tjc_hmi.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define U5_REQUIRE_LOOPBACK_AT_BOOT 0
#define U4_REQUIRE_LOOPBACK_AT_BOOT 0
#define U3_REQUIRE_LOOPBACK_AT_BOOT 0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t oled_last_tick = 0;
static char oled_fall_last[24] = "";
static char oled_vcc_last[24] = "";
static char oled_pir_last[24] = "";
static char oled_pi_last[24] = "";
static uint8_t g_lte_enabled = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_UART5_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  u2_BuffInit();
  LoRa_Init();

  u1_printf("\r\n\r\n1-正常传输模式 定点传输（网关板 F407）\r\n");
  u1_printf("系统启动成功\r\n");

  OLED_Init();
  OLED_Clear();
  ShumeiPi_Init();
  Bobao_Init();
  HAL_UART_Receive_IT(&huart3, &u3_rx_byte, 1);
  HAL_UART_Receive_DMA(&huart5, U4_RxBuf, U4_RXMAX_SIZE);
  __HAL_UART_CLEAR_IDLEFLAG(&huart5);
  __HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);

  if (lte_comm_init() == 0)
  {
    LTE_Onenet_Init();
    g_lte_enabled = 1;
  }
  else
  {
    u1_printf("[4G] lte_comm_init failed\r\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (HAL_GetTick() - oled_last_tick >= 500)
    {
      char fall_buf[24];
      char vcc_buf[24];
      char pir_buf[24];
      oled_last_tick = HAL_GetTick();

      sprintf(fall_buf, "FALL:%-8s", GW_Status_ToStr(GW_Status_GetFall()));
      sprintf(vcc_buf,  "VCC :%-8s", GW_Status_ToStr(GW_Status_GetVcc()));
      sprintf(pir_buf,  "PIR :%-8s", GW_Status_ToStr(GW_Status_GetPir()));
      ShumeiPi_Counts_t pi_counts;
      char pi_buf[24];
      ShumeiPi_GetCounts(&pi_counts);
      sprintf(pi_buf, "P:%u H:%u W:%u", pi_counts.person, pi_counts.half, pi_counts.work);
      TJC_UpdateMainPage(
        GW_Status_ToStr(GW_Status_GetFall()),
        GW_Status_ToStr(GW_Status_GetVcc()),
        GW_Status_ToStr(GW_Status_GetPir()),
        pi_counts.person,
        pi_counts.half,
        pi_counts.work
      );
      {
        tri_state_t tfall = GW_Status_GetFall();
        tri_state_t tvcc  = GW_Status_GetVcc();
        tri_state_t tpir  = GW_Status_GetPir();
        Bobao_Input_t in;

        in.fall_valid = (tfall != ST_UNKNOWN);
        in.fall = (tfall == ST_TRUE);
        in.vcc_valid = (tvcc != ST_UNKNOWN);
        in.vcc = (tvcc == ST_TRUE);
        in.pir_valid = (tpir != ST_UNKNOWN);
        in.pir = (tpir == ST_TRUE);
        in.person_cnt = pi_counts.person;
        in.half_cnt = pi_counts.half;
        in.work_cnt = pi_counts.work;

        Bobao_UpdateAll(&in);

        if (g_lte_enabled)
        {
          LTE_Onenet_UpdateData(
            (tfall == ST_TRUE) ? 1 : 0,
            (tpir == ST_TRUE) ? 1 : 0,
            (tvcc == ST_TRUE) ? 1 : 0,
            pi_counts.person,
            pi_counts.half,
            pi_counts.work
          );
          LTE_Onenet_Run();
        }
      }

      if (strcmp(fall_buf, oled_fall_last) != 0)
      {
        OLED_ShowString(0, 0, (uint8_t *)fall_buf, 16);
        strcpy(oled_fall_last, fall_buf);
      }

      if (strcmp(vcc_buf, oled_vcc_last) != 0)
      {
        OLED_ShowString(0, 2, (uint8_t *)vcc_buf, 16);
        strcpy(oled_vcc_last, vcc_buf);
      }

      if (strcmp(pir_buf, oled_pir_last) != 0)
      {
        OLED_ShowString(0, 4, (uint8_t *)pir_buf, 16);
        strcpy(oled_pir_last, pir_buf);
      }

      if (strcmp(pi_buf, oled_pi_last) != 0)
      {
        OLED_ShowString(0, 6, (uint8_t *)pi_buf, 16);
        strcpy(oled_pi_last, pi_buf);
      }
    }

		while (UART2_ControlCB.UsartRxDataOutPtr != UART2_ControlCB.UsartRxDataInPtr)
		{
			uint8_t *pStart = UART2_ControlCB.UsartRxDataOutPtr->StartPtr;
			uint16_t len = (uint16_t)(UART2_ControlCB.UsartRxDataOutPtr->EndPtr - pStart);

			if (len >= 3)
			{
				uint8_t srcH = pStart[0];
				uint8_t srcL = pStart[1];

				u1_printf("收到节点[%02X%02X] [%d]: ", srcH, srcL, len - 2);
				for (uint16_t i = 2; i < len; i++)
				{
					while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET);
					huart1.Instance->DR = pStart[i];
				}
				u1_printf("\r\n");

				GW_Status_ProcessText(&pStart[2], (uint16_t)(len - 2));
			}
			else if (len > 0)
			{
				u1_printf("收到短包[%d]\r\n", len);
			}

			UART2_ControlCB.UsartRxDataOutPtr++;
			if (UART2_ControlCB.UsartRxDataOutPtr == UART2_ControlCB.UsartRxDataEndPtr)
			{
				UART2_ControlCB.UsartRxDataOutPtr = &UART2_ControlCB.Usart_RxLocation[0];
			}
		}
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */



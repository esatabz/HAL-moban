
/* Includes ------------------------------------------------------------------*/
#include<stdio.h>
#include<string.h>
#include "lcd.h"
#include "i2c.h"
#include "main.h"
#include "adc.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
uint8_t Lcd_String[21];
uint8_t Uart_Rx;
uint8_t Iic_Eeprom_W_String[6] = {0x00, 0x13, 0x0, 0x2,0x15};
uint8_t Iic_Eeprom_R_String[6] = {0};
float Res = 0;
uint16_t Time6 = 0;
uint16_t T_Count = 0;
uint16_t D_Count = 0;
float Pwm_Duty = 0;
float Adc = 0;
RTC_TimeTypeDef H_M_S_Time;
RTC_DateTypeDef Y_M_D_Date;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void LED(uint8_t led);
uint8_t KEY(void);
void Lcd_Proc(void);
void Iic_Proc(void);
void Adc_Proc(void);
void Rtc_Proc(void);


/* Private user code ---------------------------------------------------------*/

int main(void)
{
	HAL_Init();
	LCD_Init();
	I2CInit();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_ADC2_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_TIM15_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
	
	LED(0x00);
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	HAL_UART_Receive_IT(&huart1, &Uart_Rx, 1);
	HAL_TIM_Base_Start_IT(&htim6);
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
	HAL_TIM_OC_Start_IT(&htim15,TIM_CHANNEL_1);

  while (1)
  {
//		if((KEY()) == 1) LED(0x55);
//		if((KEY()) == 2) LED(0xff);
//		if((KEY()) == 3) LED(0x77);
//		if((KEY()) == 4) LED(0x00);
		Lcd_Proc();
		Iic_Proc();
		Adc_Proc();
		Rtc_Proc();
  }

}

void LED(uint8_t led)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
	
	HAL_GPIO_WritePin(GPIOC,led<<8,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

uint8_t KEY(void)
{
	uint8_t key = 0;
	if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)) == GPIO_PIN_RESET) key = 1;
	if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)) == GPIO_PIN_RESET) key = 2;
	if((HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)) == GPIO_PIN_RESET) key = 3;
	if((HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)) == GPIO_PIN_RESET) key = 4;
	return key;
}

void Lcd_Proc(void)
{
	sprintf((char *)Lcd_String, "Time:%02d-%02d-%02d",H_M_S_Time.Hours, H_M_S_Time.Minutes, H_M_S_Time.Seconds);
	LCD_DisplayStringLine(Line0, Lcd_String);
	sprintf((char *)Lcd_String, "Date:%02d-%02d-%02d",Y_M_D_Date.Year, Y_M_D_Date.Month, Y_M_D_Date.Date);
	LCD_DisplayStringLine(Line1, Lcd_String);
	sprintf((char *)Lcd_String, "WeekDay:%d",Y_M_D_Date.WeekDay);
	LCD_DisplayStringLine(Line2, Lcd_String);
	sprintf((char *)Lcd_String, "Adc:%4.2f",Adc);
	LCD_DisplayStringLine(Line3, Lcd_String);
	sprintf((char *)Lcd_String, "Eeprom:%x%x%x%x%x",Iic_Eeprom_R_String[0],Iic_Eeprom_R_String[1],Iic_Eeprom_R_String[2],Iic_Eeprom_R_String[3],Iic_Eeprom_R_String[4]);
	LCD_DisplayStringLine(Line4, Lcd_String);
	sprintf((char *)Lcd_String, "Res:%4.2f",Res);
	LCD_DisplayStringLine(Line5, Lcd_String);
	sprintf((char *)Lcd_String, "Time6:%05d",Time6);
	LCD_DisplayStringLine(Line6, Lcd_String);
	sprintf((char *)Lcd_String, "Fequency:%05d",(uint16_t)(1000000/T_Count));
	LCD_DisplayStringLine(Line7, Lcd_String);
	sprintf((char *)Lcd_String, "Duty:%4.2f%%",Pwm_Duty*100);
	LCD_DisplayStringLine(Line8, Lcd_String);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Transmit(&huart1, &Uart_Rx, 1, 50);
	HAL_UART_Receive_IT(&huart1, &Uart_Rx, 1);
}

//i2c
void Iic_Proc(void)
{
	Iic_Eeprom_Write(Iic_Eeprom_W_String, 0, 5);
	HAL_Delay(2);
	Iic_Eeprom_Read(Iic_Eeprom_R_String, 0, 5);
	
	Iic_Res_Write(0x0d);
	Res = (float)(Iic_Res_Read())*0.7874f;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6) Time6++;
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			T_Count = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1)+1;
			Pwm_Duty = (float)D_Count/T_Count;
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			D_Count = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2)+1;
		}
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM15)
	{
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			__HAL_TIM_SET_COMPARE(htim,TIM_CHANNEL_1,__HAL_TIM_GetCounter(htim)+500);
		}
	}
}

//ADC
void Adc_Proc(void)
{
	HAL_ADC_Start(&hadc2);
	Adc = ((float)(HAL_ADC_GetValue(&hadc2))/4096)*3.3f;
}

//RTC
void Rtc_Proc(void)
{
	HAL_RTC_GetTime(&hrtc, &H_M_S_Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Y_M_D_Date, RTC_FORMAT_BIN);
}
















void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  /* User can add his own implementation to report the HAL error return state */
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


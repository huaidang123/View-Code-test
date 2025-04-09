/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * 开关量输入输出模块
  * 华中堂13855361411，egg_car，wuhuaviation@163.com
  *
  * 控制函数与状态字。
  * 效仿PLC的架构。
  * Yout(5,1);输出5号脚置1。Y[5]==1。Xin(3);读取3号输入脚，X[3]==0
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "hmi_driver.h"
#include "rs485.h"

extern uint16_t Step;//自动检测的步骤
extern uint8_t status;//触摸屏、电位器、步进、耐久选择0,1,2,3
uint8_t Y[9]={0};  //输出开关的状态数组，同硬件初始化
void Yout(uint8_t Port,uint8_t State)	//Port 是端口，state是状态(0弹起，1吸合，2翻转)
{
	GPIO_TypeDef*  GPIO_Port;
	unsigned short GPIO_Pin;
	uint8_t Port_Value;
	switch(Port)
	{
		case 0:   GPIO_Port=GPIOC; GPIO_Pin=GPIO_PIN_13;break; //呼吸灯
		case 1:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_3; break; //常电
		case 2:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_2; break; //点火
		case 3:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_1; break; //正控，RS485接收使能
		case 4:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_0; break; //负控，RS485发送使能
		case 5:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_7; break; //燃油
		case 6:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_6; break; //水温
		case 7:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_5; break; //油压
		case 8:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_4; break; //气压
		default:	break;
	}
	switch(State)
	{
		case 0:  HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_RESET);Port_Value=0;break;
		case 1:	 HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);Port_Value=1;break;
		case 2:	 HAL_GPIO_TogglePin(GPIO_Port, GPIO_Pin); Port_Value=1-Y[Port]; break;
		default:	return;
	}
	Y[Port]=Port_Value;//输出端口状态值
}//输出端口设置完毕
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
uint8_t X[1]={0}; //输入按键的状态数组,数组是从0开始	
static uint8_t Read_Key(uint8_t Port)//瞬间读取按键值
{
	uint8_t Port_Value=0;
	switch(Port)
		{
		case 0:	 Port_Value= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);	break;	//摄像头
		default:	break;
		}
return Port_Value;	 
}

void Xin(uint8_t Port)	//读取按键后滤波
{ 
	uint8_t Port_Value=0;
	if(Read_Key(Port)==1)//第一次读取
	{
	HAL_Delay(20);
	if(Read_Key(Port)==1)//第二次读取
	Port_Value=1;
	}
	X[Port]=Port_Value;
 }
/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_SET);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB5 PB6
                           PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 PB3 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void Scan_KEY(void)
{
 Xin(0);
 if(X[0]==0)status=0;//摄像头报故障
}
/* USER CODE END 2 */

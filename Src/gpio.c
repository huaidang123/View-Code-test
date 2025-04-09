/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * �������������ģ��
  * ������13855361411��egg_car��wuhuaviation@163.com
  *
  * ���ƺ�����״̬�֡�
  * Ч��PLC�ļܹ���
  * Yout(5,1);���5�Ž���1��Y[5]==1��Xin(3);��ȡ3������ţ�X[3]==0
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "hmi_driver.h"
#include "rs485.h"

extern uint16_t Step;//�Զ����Ĳ���
extern uint8_t status;//����������λ�����������;�ѡ��0,1,2,3
uint8_t Y[9]={0};  //������ص�״̬���飬ͬӲ����ʼ��
void Yout(uint8_t Port,uint8_t State)	//Port �Ƕ˿ڣ�state��״̬(0����1���ϣ�2��ת)
{
	GPIO_TypeDef*  GPIO_Port;
	unsigned short GPIO_Pin;
	uint8_t Port_Value;
	switch(Port)
	{
		case 0:   GPIO_Port=GPIOC; GPIO_Pin=GPIO_PIN_13;break; //������
		case 1:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_3; break; //����
		case 2:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_2; break; //���
		case 3:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_1; break; //���أ�RS485����ʹ��
		case 4:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_0; break; //���أ�RS485����ʹ��
		case 5:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_7; break; //ȼ��
		case 6:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_6; break; //ˮ��
		case 7:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_5; break; //��ѹ
		case 8:   GPIO_Port=GPIOB; GPIO_Pin=GPIO_PIN_4; break; //��ѹ
		default:	break;
	}
	switch(State)
	{
		case 0:  HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_RESET);Port_Value=0;break;
		case 1:	 HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);Port_Value=1;break;
		case 2:	 HAL_GPIO_TogglePin(GPIO_Port, GPIO_Pin); Port_Value=1-Y[Port]; break;
		default:	return;
	}
	Y[Port]=Port_Value;//����˿�״ֵ̬
}//����˿��������
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
uint8_t X[1]={0}; //���밴����״̬����,�����Ǵ�0��ʼ	
static uint8_t Read_Key(uint8_t Port)//˲���ȡ����ֵ
{
	uint8_t Port_Value=0;
	switch(Port)
		{
		case 0:	 Port_Value= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);	break;	//����ͷ
		default:	break;
		}
return Port_Value;	 
}

void Xin(uint8_t Port)	//��ȡ�������˲�
{ 
	uint8_t Port_Value=0;
	if(Read_Key(Port)==1)//��һ�ζ�ȡ
	{
	HAL_Delay(20);
	if(Read_Key(Port)==1)//�ڶ��ζ�ȡ
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
 if(X[0]==0)status=0;//����ͷ������
}
/* USER CODE END 2 */

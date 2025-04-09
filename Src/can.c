/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  * �û�˵������ԭ���������޸ģ����ֳ���Ĵ�����һ�¡�
  * �Զ��岨����CAN_Baud����׼֡500k,��չ֡250k
  * CAN���η���CAN_Tx_Msg�����ݱ��ĵ�id,ѡ��֡��ʽ
  * CAN��η���CANking�����Kvasar��Ϲ��ߣ�������ά���Ӵ���
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
CAN_TxHeaderTypeDef Header;   //CAN��������ͷ

void CAN_Baud(uint16_t Baudrate)
{
	uint16_t Pre;
	switch(Baudrate)    //�ж�����Ĳ����ʣ��ҵ���Ӧ�ķ�Ƶϵ��
	{
		case 10:Pre = 600;break;
		case 20:Pre = 300;break;
		case 50:Pre = 120;break;
		case 100:Pre = 60;break;
		case 125:Pre = 48;break;
		case 250:Pre = 24;break;
		case 500:Pre = 12;break;
		case 1000:Pre = 6;break;
		default: return;
	}
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = Pre;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
}	//�������������

HAL_StatusTypeDef CAN_Tx_Msg(CanMsgTypeDef TxMsg)//CAN���ͺ���
{
	uint32_t msg_box;              //���ͻ�����
	if(TxMsg.Id<0x7ff)
	{
		Header.StdId=TxMsg.Id;        //��ֵ����׼ID
		Header.IDE=CAN_ID_STD;        //��ʽΪ��׼֡
	}
	else
	{
		Header.ExtId=TxMsg.Id;        //��ֵ����չID
		Header.IDE=CAN_ID_EXT;        //��ʽΪ��չ֡
	}	
	Header.RTR = CAN_RTR_DATA;    //ѡ������֡
	Header.DLC=8;                 //���ݳ���8λ
	Header.TransmitGlobalTime = DISABLE;//������ʱ���
  HAL_CAN_AddTxMessage(&hcan,&Header,TxMsg.Data,&msg_box);
	return HAL_OK;
}

void CANking(CanMsgTypeDef TxMsg)
{
	int Times=10;
	while(Times--){CAN_Tx_Msg(TxMsg);HAL_Delay(20);}
}
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 24;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_CAN1_2();

  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PB8     ------> CAN_RX
    PB9     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

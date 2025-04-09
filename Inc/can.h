/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */
typedef struct
{
  uint32_t Id;       //CAN���ĵ�Id�����Id�ĳ��ȣ��ж��Ǳ�׼֡������չ֡

  uint8_t Data[8];   //CAN���ĵ����ݣ�Ĭ��8λ������DLC��λ��������������

}CanMsgTypeDef;	   //CAN���Ľṹ�嶨��

void CAN_Baud(uint16_t Baudrate);//CAN����������
HAL_StatusTypeDef CAN_Tx_Msg(CanMsgTypeDef TxMsg); //CAN���η���
void CANking(CanMsgTypeDef TxMsg); //CAN�ظ����ͺ���
/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */


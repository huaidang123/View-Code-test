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
  uint32_t Id;       //CAN报文的Id。检测Id的长度，判断是标准帧还是扩展帧

  uint8_t Data[8];   //CAN报文的数据，默认8位，大于DLC的位数，丢弃不发送

}CanMsgTypeDef;	   //CAN报文结构体定义

void CAN_Baud(uint16_t Baudrate);//CAN波特率设置
HAL_StatusTypeDef CAN_Tx_Msg(CanMsgTypeDef TxMsg); //CAN单次发送
void CANking(CanMsgTypeDef TxMsg); //CAN重复发送函数
/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */


#ifndef __RS485_H
#define __RS485_H	

#include "main.h"
void RS485_SendBytes(uint8_t *buf, uint8_t len);//485发送字符串
void K(uint8_t port, uint8_t state);//志远电子32路继电器操作
void DataR(uint16_t res);//8421数字电阻
void IGN(uint8_t state);//点火函数
#endif

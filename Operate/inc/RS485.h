#ifndef __RS485_H
#define __RS485_H	

#include "main.h"
void RS485_SendBytes(uint8_t *buf, uint8_t len);//485�����ַ���
void K(uint8_t port, uint8_t state);//־Զ����32·�̵�������
void DataR(uint16_t res);//8421���ֵ���
void IGN(uint8_t state);//�����
#endif

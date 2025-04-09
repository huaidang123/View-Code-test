/************************************�û�����********************************************
**��Ҧ�����Զ����豸���޹�˾ �����������ӿƼ����޹�˾ ���ջ���ְҵ����ѧԺ
**-----------------------------------�ļ���Ϣ--------------------------------------------
** �ļ�����:   operate.c
** �޸�ʱ��:   2022-04-08
** �ļ�˵��:   �û�����������
** ����֧��    ���� Tel:13855361411��΢��ͬ�ţ�
----------------------------------------------------------------------------------------*/
#include "operate.h"      //�Զ��������ʽ
#include "Vehicle.h"      //����Ӳ�ز���
#include "usart.h"        //�����շ�����
#include "gpio.h"         //����������
#include "can.h"          //����˫������
#include "rs485.h"        //��ҵ�������
#include "tim.h"          //��ʱ��PWM
#include "cmd_process.h"  //����������ָ��

extern uint16_t  Step;           //��ⲽ��
extern CanMsgTypeDef CANmsg[];  //CAN��������
extern uint16_t current_screen_id; //��ǰ����ID
extern uint8_t status;//����������λ�����������;�ѡ��0,1,2,3
extern uint16_t FuelR[10];
extern uint16_t CoolR[10];
extern uint16_t GasR[10];
extern uint16_t FluidR[10];

void ByteToHexStr(const unsigned char* source, char* dest, int Length)  
{  
    int i;  
    unsigned char highByte, lowByte;  
  
    for (i = 0; i < Length; i++)  
    {  
        highByte = source[i] >> 4;  
        lowByte = source[i] & 0x0f ;  
  
        highByte += 0x30;  
  
        if (highByte > 0x39)  
                dest[i * 2] = highByte + 0x07;  
        else  
                dest[i * 2] = highByte;  
  
        lowByte += 0x30;  
        if (lowByte > 0x39)  
            dest[i * 2 + 1] = lowByte + 0x07;  
        else  
            dest[i * 2 + 1] = lowByte;  
    }  
    return ;  
	}

void Study(void)//��ѧ��ʾ
{
	int i;
	IGN(1);
	Fuel(5);
	for(i=7;i<17;i++)
	{
	SetScreen(i);//�л�ҳ��
	HAL_Delay(300);//ͣ��һ��
	}
	SetScreen(12);//�����綯
	for(i=1;i<9;i++)
  {
	Yout(i,1);//IO���Ƽ��
	SetButtonValue(12,i,1);	
	HAL_Delay(500);//ͣ��500����
	}
	SetScreen(2);//CAN
	SetTextValue(2,129,(uint8_t*)&"�ͻ�ת��");//ת��
	SetTextValue(2,209,(uint8_t*)&"0CF00400");
	for(i=0;i<0x5dc0;i+=5)
  {
	 CANmsg[50].Data[4]=(i>>8)&0xff;CANmsg[50].Data[3]=i&0xff;
   CAN_Tx_Msg(CANmsg[50]);
	 SetTextInt32(2,205,(i>>8)&0xff,0,0);
	 SetTextInt32(2,204,i&0xff,0,0);
	}
	SetTextValue(2,129,(uint8_t*)&"��ȴˮ��");//ˮ��
	SetTextValue(2,209,(uint8_t*)&"18FEEE00");
	for(i=0x50;i<0xa0;i++)
  {
	 CANmsg[70].Data[0]=i;
   CAN_Tx_Msg(CANmsg[70]);
	 HAL_Delay(100);
   SetTextInt32(2,201,i,0,0);
	}
	SetTextValue(2,129,(uint8_t*)&"����ѹ��");//����
	SetTextValue(2,209,(uint8_t*)&"18FEEF00");
	for(i=0x00;i<0xfa;i++)
  {
	 CANmsg[80].Data[3]=i;
   CAN_Tx_Msg(CANmsg[80]);
	SetTextInt32(2,204,i,0,0);
	 HAL_Delay(20);
	}
  SetScreen(3);//������
	Cool(5);
	for(i=1;i<17;i++)
  {
  K(i,1);
	SetButtonValue(3,i,1);
	HAL_Delay(500);//ͣ��1��	
	}
	SetScreen(0);//����
	for(i=1;i<9;i++)SetButtonValue(12,i,0);
	SetScreen(1);//��ҳ
	for(i=0;i<17;i++)SetButtonValue(3,i,0);
	SetButtonValue(1,800,0);
	IGN(0);
	K(0,0);	
}//�л����еĽ��棬������ʾ

void Test(uint8_t Model,uint8_t num) //�������,Model���ͺţ�step�ǵ�������
{int i;
if(num==0){SetTextValue(Model,256,(uint8_t*)&"����");\
           SetScreen(Model);IGN(0);}//��ʼ�����ϵ�����Ǳ�
if(num==1){SetTextValue(Model,256,(uint8_t*)&"���");IGN(1);\
	         for(i=0;i<100;i++){SetButtonValue(Model,i,0);HAL_Delay(10);SetButtonValue(Model,119,i%2);}
           }//���Ǳ��Լ�����ʱ��
if(num==2){SetTextValue(Model,256,(uint8_t*)&"�Ƽ�");\
           K(0,1);HAL_Delay(1000);K(0,0);}//���ټ��
if(num==3){SetTextValue(Model,256,(uint8_t*)&"Ƶ��");\
	         for(i=0;i<10;i++){RPM(i*100);SetButtonValue(Model,40+i,1);HAL_Delay(300);}
           }//Ƶ��ת��
if(num==4){SetTextValue(Model,256,(uint8_t*)&"����");\
	         for(i=0;i<100;i++){CANmsg[50].Data[4]=i;CAN_Tx_Msg(CANmsg[50]);HAL_Delay(20);\
          SetButtonValue(Model,40+i/10,0);}}//CANת��
if(num==5){SetTextValue(Model,256,(uint8_t*)&"ȼ��");
	         for(i=1;i<10;i+=2){Fuel(i); SetButtonValue(Model,50+i,1);\
	         CANking(CANmsg[50+i]);}}//����1��������ȼ�͸�Ϊ���ͱ���           
if(num==6){SetTextValue(Model,256,(uint8_t*)&"ˮ��");
	        for(i=1;i<10;i+=2){Cool(i); SetButtonValue(Model,60+i,1);\
           CANking(CANmsg[60+i]);}}//����2��ˮ�±���
if(num==7){SetTextValue(Model,256,(uint8_t*)&"��ѹ");
	         for(i=1;i<10;i+=2){Gas(i); SetButtonValue(Model,70+i,1); \
           CANking(CANmsg[70+i]);}}//����3������ѹ������
if(num==8){SetTextValue(Model,256,(uint8_t*)&"��ѹ");
	        for(i=1;i<10;i+=2){Fluid(i);HAL_Delay(100);SetButtonValue(Model,80+i,1);\
//          CANking(CANmsg[80+i]);
          }}//��ѹ��ģ����
if(num>8 && num<25){SetTextInt32(Model,256,num,0,0);K(num-8,1);SetButtonValue(Model,num,1);\
          HAL_Delay(500);K(num-7,0); CANking(CANmsg[num]);\
           SetButtonValue(Model,num+1,0);}//��һ����Ϩ���Ǻ��˼�����Ҫ         
if(num>24 && num<30){SetTextInt32(Model,256,num,0,0);HAL_Delay(50);
	        CANking(CANmsg[num]);SetButtonValue(Model,num,1);SetButtonValue(Model,num+1,0);
          }//�ŷ�����ƣ�˾��������Ч
if(num==30){IGN(0);K(0,0);SetButtonValue(Model,101,0);Step=0;status=0;\
           for(i=0;i<100;i++)//������ť����,������������OK
           {if(i%2)SetTextValue(Model,256,(uint8_t*)&"OK");else SetTextValue(Model,256,(uint8_t*)&"����");}}
else return;
}

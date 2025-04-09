/************************************��Ȩ����********************************************
**                             ���ݴ�ʹ��Ƽ����޹�˾
**                             http://www.gz-dc.com
**-----------------------------------�ļ���Ϣ-------------------------------------------
** �ļ�����:   cmd_process.c
** �޸�ʱ��:   2018-05-18
** �ļ�˵��:   ����������
** ����֧�֣�  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------*/
#include "cmd_process.h"   //����������ģ��
#include "usart.h"         //���ڲ���
#include "gpio.h"          //ֱ��IO��
#include "tim.h"           //��ʱ��PWM
#include "adc.h"           //ģ����������
#include "can.h"           //CAN�����շ�
#include "operate.h"       //�û���������
#include "rs485.h"         //485��չģ��
#include "Vehicle.h"       //����Ӳ��
#include <stdio.h>         //C���������������,֧��sscanf()

/*�������·�ָ��ĵĲ�����������������*/
uint16_t current_screen_id=1; //��ǰ����ID
uint16_t current_control_id=45;//��ǰ�ؼ�ID
uint8_t  cmd_buffer[CMD_MAX_SIZE]; //��������ָ���

long     Value=0;      //���崰��ʮ��������
long     Data=0x00;    //���崰��ʮ����������

uint8_t  ch=0;        //PWM��ͨ��
float    Hz=1500;     //PWM��Ƶ��
uint8_t  Duty=50;     //PWM��ռ�ձ�
uint16_t LifeTime=100; //�ϻ�ʱ��

extern uint8_t status; //�������ϸı�֡��ʽ
extern CAN_TxHeaderTypeDef Header; //�������ϸı�֡��ʽ
extern CanMsgTypeDef CANmsg[];     //���߱���,֡��ʽʶ��

/*! 
*  \brief ɨ��������Ļ��ָ��
*/
void Scan_TFT(void)
{
	static qsize size=0;
	size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);    //�ӻ������л�ȡһ��ָ��         
	if(size>0&&cmd_buffer[1]!=0x07)                    //���յ�ָ�� �����ж��Ƿ�Ϊ������ʾ
	{
		ProcessMessage((PCTRL_MSG)cmd_buffer, size);      //ָ���  
	}                                                                           
	else if(size>0&&cmd_buffer[1]==0x07)               //���Ϊָ��0x07��������STM32  
	{                                                                           
		__disable_fault_irq();                                                   
		NVIC_SystemReset();                                                                                                                                          
	}
	
	// �ر�ע��
	// MCU��ҪƵ���򴮿����������ݣ�������ӵ���ᶪʧ����(��������С����׼��8K��������4.7K)
	// 1) һ������£�����MCU�򴮿����������ݵ����ڴ���100ms���Ϳ��Ա������ݶ�ʧ�����⣻
	// 2) �����Ȼ�����ݶ�ʧ�����⣬���жϴ�������BUSY���ţ�Ϊ��ʱ���ܷ������ݸ���������

	// TODO: ����û�����
	// �����и���ʱ��־λ�����ı䣬��ʱ100����ˢ����Ļ
	
//  if( HAL_GetTick()%100==0)UpdateUI();//���½���
}
/*! 
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
*/
void ProcessMessage( PCTRL_MSG msg, uint16_t size )
{
	uint8_t cmd_type = msg->cmd_type;                                               //ָ������
	uint8_t ctrl_msg = msg->ctrl_msg;                                               //��Ϣ������
	uint8_t control_type = msg->control_type;                                       //�ؼ�����
	uint16_t screen_id = PTR2U16(&msg->screen_id);                                  //����ID
	uint16_t control_id = PTR2U16(&msg->control_id);                                //�ؼ�ID
	uint32_t value = PTR2U32(msg->param);                                           //��ֵ

	switch(cmd_type)
	{  
		case NOTIFY_TOUCH_PRESS:                                                        //����������
		case NOTIFY_TOUCH_RELEASE:                                                      //�������ɿ�
				NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4)); 
				break;                                                                    
		case NOTIFY_WRITE_FLASH_OK:                                                     //дFLASH�ɹ�
				NotifyWriteFlash(1);                                                      
				break;                                                                    
		case NOTIFY_WRITE_FLASH_FAILD:                                                  //дFLASHʧ��
				NotifyWriteFlash(0);                                                      
				break;                                                                    
		case NOTIFY_READ_FLASH_OK:                                                      //��ȡFLASH�ɹ�
				NotifyReadFlash(1,cmd_buffer+2,size-6);                                     //ȥ��֡ͷ֡β
				break;                                                                    
		case NOTIFY_READ_FLASH_FAILD:                                                   //��ȡFLASHʧ��
				NotifyReadFlash(0,0,0);                                                   
				break;                                                                    
		case NOTIFY_READ_RTC:                                                           //��ȡRTCʱ��
				NotifyReadRTC(cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7],cmd_buffer[8]);
				break;
		case NOTIFY_CONTROL:
				{
						if(ctrl_msg==MSG_GET_CURRENT_SCREEN)                                    //����ID�仯֪ͨ
						{
								NotifyScreen(screen_id);                                            //�����л������ĺ���
						}
						else
						{
								switch(control_type)
								{
								case kCtrlButton:                                                   //��ť�ؼ�
										NotifyButton(screen_id,control_id,msg->param[1]);                  
										break;                                                             
								case kCtrlText:                                                     //�ı��ؼ�
										NotifyText(screen_id,control_id,msg->param);                       
										break;                                                             
								case kCtrlProgress:                                                 //�������ؼ�
										NotifyProgress(screen_id,control_id,value);                        
										break;                                                             
								case kCtrlSlider:                                                   //�������ؼ�
										NotifySlider(screen_id,control_id,value);                          
										break;                                                             
								case kCtrlMeter:                                                    //�Ǳ�ؼ�
										NotifyMeter(screen_id,control_id,value);                           
										break;                                                             
								case kCtrlMenu:                                                     //�˵��ؼ�
										NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);      
										break;                                                              
								case kCtrlSelector:                                                 //ѡ��ؼ�
										NotifySelector(screen_id,control_id,msg->param[0]);                
										break;                                                              
								case kCtrlRTC:                                                      //����ʱ�ؼ�
										NotifyTimer(screen_id,control_id);
										break;
								default:
										break;
								}
						} 
						break;  
				} 
		case NOTIFY_HandShake:         //����֪ͨ                                                     
				NOTIFYHandShake();
				break;
		default:
				break;
	}
}
/*! 
*  \brief  ����֪ͨ
*/
void NOTIFYHandShake()
{
// SetButtonValue(1,101,0); //��������MCU���ֳɹ���������Ļ1�Ĳ�����ť��
}

/*! 
*  \brief  �����л�֪ͨ
*  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
*  \param screen_id ��ǰ����ID
*/
void NotifyScreen(uint16_t screen_id)
{
 current_screen_id = screen_id;  //�ڹ��������п��������л�֪ͨ����¼��ǰ����ID  
}

/*! 
*  \brief  ���������¼���Ӧ
*  \param press 1���´�������0�ɿ�������
*  \param x ������
*  \param y ������
*/
void NotifyTouchXY(uint8_t press,uint16_t x,uint16_t y)
{ 
    //���������ΰ�ť���û����������������ҵ���ʹ�á�
}


/*! 
*  \brief  ��������
*/ 
void UpdateUI(void)
{
// Yout(0,2);//������
// CAN_Tx_Msg(CANmsg[current_control_id]);
 }
 /*! 
*  \brief  ��ť�ؼ�֪ͨ
*  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param state ��ť״̬��0����1����
*/

void NotifyButton(uint16_t screen_id, uint16_t control_id, uint8_t state)
{ 
	current_screen_id=screen_id;
	if(control_id==433){K(0,state);for(int i=1;i<17;i++)SetButtonValue(current_screen_id,i+8,state);}//������ȫ��ȫ��
	if(control_id==101 && state==1) status=1;//�����Զ�����
	else if(control_id==800) status=2;else status=0;//�����ѧ��ʾ
	if(control_id==808){ if(state)status=2;else status=0;}//�ϻ���������������ͣ
	if(control_id>50 && control_id<60)Fuel(control_id-50);  //ȼ�͵��赵
	if(control_id>60 && control_id<70)Cool(control_id-60);  //ˮ�µ��赵
	if(control_id>70 && control_id<80)Gas(control_id-70);   //��ѹ���赵
	if(control_id>80 && control_id<90)Fluid(control_id-80); //��ѹ���赵
	if(control_id==119)IGN(state);//���Ϩ��
	if(control_id>25 && control_id<=99)current_control_id=control_id;//�̻���ť
	if(screen_id==3)K(control_id,state);//�̵������ز���;
	else if(control_id>0 && control_id<=8)Yout(control_id,state);//IO����
	else if(control_id>8 && control_id<25) K(control_id-8,state);//�̵�������
	else return;
}//��ť��������

/*! 
*  \brief  �ı��ؼ�֪ͨ
*  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
*  \details  �ı��ؼ����������ַ�����ʽ�·���MCU������ı��ؼ������Ǹ���ֵ��
*  \details  ����Ҫ�ڴ˺����н��·��ַ�������ת�ظ���ֵ��
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param str �ı��ؼ�����
*/
void NotifyText(uint16_t screen_id, uint16_t control_id, uint8_t *str)
{
	sscanf((const char*)str,"%ld",&Value); //�ַ�ת����ʮ��������
  sscanf((const char*)str,"%lx",&Data);  //�ַ�ת����ʮ����������

	/*BUS_ID����*/
  if(control_id ==209) CANmsg[50].Id = Data;    //CAN_Id
	/*DATA[8]����*/
	if(control_id ==201) CANmsg[50].Data[0] = Data;	//DATA0
  if(control_id ==202) CANmsg[50].Data[1] = Data;	//DATA1
	if(control_id ==203) CANmsg[50].Data[2] = Data;	//DATA2
	if(control_id ==204) CANmsg[50].Data[3] = Data;	//DATA3
	if(control_id ==205) CANmsg[50].Data[4] = Data;	//DATA4
	if(control_id ==206) CANmsg[50].Data[5] = Data;	//DATA5
	if(control_id ==207) CANmsg[50].Data[6] = Data;	//DATA6
	if(control_id ==208) CANmsg[50].Data[7] = Data;	//DATA7
	
	if(control_id ==164) DataR(Value);//���ֵ���
	if(control_id ==168) {Hz=Value;  PWM(ch,Hz,Duty);}//PWMƵ��
	if(control_id ==169) {Duty=Value;PWM(ch,Hz,Duty);}//PWMռ�ձ�
	if(control_id ==710) LifeTime=Value;//�ϻ�ʱ������
	//PWM����
} //�ı��������                                                                               

/*!                                                                              
*  \brief  �������ؼ�֪ͨ                                                       
*  \details  ����GetControlValueʱ��ִ�д˺���                                  
*  \param screen_id ����ID                                                      
*  \param control_id �ؼ�ID                                                     
*  \param value ֵ                                                              
*/                                                                              
void NotifyProgress(uint16_t screen_id, uint16_t control_id, uint32_t value)           
{  
//if(screen_id==1 && control_id==1234)
//  BRIGHT=(uint8_t)value;//�������ı�����
}                                                                                

/*!                                                                              
*  \brief  �������ؼ�֪ͨ                                                       
*  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���                  
*  \param screen_id ����ID                                                      
*  \param control_id �ؼ�ID                                                     
*  \param value ֵ                                                              
*/                                                                              
void NotifySlider(uint16_t screen_id, uint16_t control_id, uint32_t value)             
{                                                             
    //TODO: ����û�����
}

/*! 
*  \brief  �Ǳ�ؼ�֪ͨ
*  \details  ����GetControlValueʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param value ֵ
*/
void NotifyMeter(uint16_t screen_id, uint16_t control_id, uint32_t value)
{
    //TODO: ����û�����
}

/*! 
*  \brief  �˵��ؼ�֪ͨ
*  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param item �˵�������
*  \param state ��ť״̬��0�ɿ���1����
*/
void NotifyMenu(uint16_t screen_id, uint16_t control_id, uint8_t item, uint8_t state)
{
	if(control_id ==112) SetScreen(6+item);//����ѡ��
	/*����2��CAN����,���������ʣ�֡��ʽ��Ԥ��id*/
	
	if(control_id ==212) //������ѡ��
	{
	if(item==0)CAN_Baud(500);//������500k
	if(item==1)CAN_Baud(250);//������250k
	}
	
	if(control_id ==215) //֡��ʽѡ��
	{
	if(item==0) Header.IDE=CAN_ID_STD;  //��׼֡
	if(item==1) Header.IDE=CAN_ID_EXT;  //��չ֡
	}

	if(control_id ==218) //CAN_IDԤѡ
	{
		if(item==0) {CANmsg[0].Id = 0x0CF00400;  SetTextValue(2,209,(uint8_t*)&"0CF00400");}
		if(item==1) {CANmsg[0].Id = 0x613;       SetTextValue(2,209,(uint8_t*)&"����613"); }
		if(item==2) {CANmsg[0].Id = 0x18FEEF00;  SetTextValue(2,209,(uint8_t*)&"18FEEF00");}
		if(item==3) {CANmsg[0].Id = 0x10F8108D;  SetTextValue(2,209,(uint8_t*)&"10F8108D");}
		if(item==4) {CANmsg[0].Id = 0x18FEEE00;  SetTextValue(2,209,(uint8_t*)&"18FEEE00");}
		if(item==5) {CANmsg[0].Id = 0x10F8109A;  SetTextValue(2,209,(uint8_t*)&"10F8109A");}
	}
	
	if(control_id ==162) //����ͨ��
	{
		if(item==0) {K(29,1);K(30,0);K(31,0);K(32,0);}//ȼ��
		if(item==1) {K(29,0);K(30,1);K(31,0);K(32,0);}//ˮ��
		if(item==2) {K(29,0);K(30,0);K(31,1);K(32,0);}//��ѹ
		if(item==3) {K(29,0);K(30,0);K(31,0);K(32,1);}//��ѹ
	}
	
	if(control_id ==166) //PWMͨ��
	{
		if(item==0) ch=1;//ת��
		if(item==1) ch=2;//����
	}
}//ѡ��˵�����

/*! 
*  \brief  ѡ��ؼ�֪ͨ
*  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*  \param item ��ǰѡ��
*/
void NotifySelector(uint16_t screen_id, uint16_t control_id, uint8_t  item)
{
 //TODO: ����û�����
}


/*! 
*  \brief  ��ʱ����ʱ֪ͨ����
*  \param screen_id ����ID
*  \param control_id �ؼ�ID
*/
void NotifyTimer(uint16_t screen_id, uint16_t control_id)
{
    //TODO: ����û�����
}

/*! 
*  \brief  ��ȡ�û�FLASH״̬����
*  \param status 0ʧ�ܣ�1�ɹ�
*  \param _data ��������
*  \param length ���ݳ���
*/
void NotifyReadFlash(uint8_t status,uint8_t *_data,uint16_t length)
{
    //TODO: ����û�����
}

/*! 
*  \brief  д�û�FLASH״̬����
*  \param status 0ʧ�ܣ�1�ɹ�
*/
void NotifyWriteFlash(uint8_t status)
{
    //TODO: ����û�����
}

/*! 
*  \brief  ��ȡRTCʱ�䣬ע�ⷵ�ص���BCD��
*  \param year �꣨BCD��
*  \param month �£�BCD��
*  \param week ���ڣ�BCD��
*  \param day �գ�BCD��
*  \param hour ʱ��BCD��
*  \param minute �֣�BCD��
*  \param second �루BCD��
*/
void NotifyReadRTC(uint8_t year,uint8_t month,uint8_t week,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
    //TODO: ����û����룬���Խ����ʴ������İ���
}

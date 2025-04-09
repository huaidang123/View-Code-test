/************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息-------------------------------------------
** 文件名称:   cmd_process.c
** 修改时间:   2018-05-18
** 文件说明:   读屏函数库
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------*/
#include "cmd_process.h"   //触摸屏功能模块
#include "usart.h"         //串口操作
#include "gpio.h"          //直接IO口
#include "tim.h"           //定时器PWM
#include "adc.h"           //模拟量的配置
#include "can.h"           //CAN报文收发
#include "operate.h"       //用户操作函数
#include "rs485.h"         //485扩展模块
#include "Vehicle.h"       //汽车硬控
#include <stdio.h>         //C语言输入输出函数,支持sscanf()

/*触摸屏下发指令的的操作函数，都在这里*/
uint16_t current_screen_id=1; //当前画面ID
uint16_t current_control_id=45;//当前控件ID
uint8_t  cmd_buffer[CMD_MAX_SIZE]; //触摸屏的指令缓存

long     Value=0;      //定义窗口十进制数据
long     Data=0x00;    //定义窗口十六进制数据

uint8_t  ch=0;        //PWM的通道
float    Hz=1500;     //PWM的频率
uint8_t  Duty=50;     //PWM的占空比
uint16_t LifeTime=100; //老化时间

extern uint8_t status; //触摸屏上改变帧格式
extern CAN_TxHeaderTypeDef Header; //触摸屏上改变帧格式
extern CanMsgTypeDef CANmsg[];     //总线报文,帧格式识别

/*! 
*  \brief 扫描来自屏幕的指令
*/
void Scan_TFT(void)
{
	static qsize size=0;
	size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE);    //从缓冲区中获取一条指令         
	if(size>0&&cmd_buffer[1]!=0x07)                    //接收到指令 ，及判断是否为开机提示
	{
		ProcessMessage((PCTRL_MSG)cmd_buffer, size);      //指令处理  
	}                                                                           
	else if(size>0&&cmd_buffer[1]==0x07)               //如果为指令0x07就软重置STM32  
	{                                                                           
		__disable_fault_irq();                                                   
		NVIC_SystemReset();                                                                                                                                          
	}
	
	// 特别注意
	// MCU不要频繁向串口屏发送数据，串口屏拥塞会丢失数据(缓冲区大小：标准型8K，基本型4.7K)
	// 1) 一般情况下，控制MCU向串口屏发送数据的周期大于100ms，就可以避免数据丢失的问题；
	// 2) 如果仍然有数据丢失的问题，请判断串口屏的BUSY引脚，为高时不能发送数据给串口屏。

	// TODO: 添加用户代码
	// 数据有更新时标志位发生改变，定时100毫秒刷新屏幕
	
//  if( HAL_GetTick()%100==0)UpdateUI();//更新界面
}
/*! 
*  \brief  消息处理流程
*  \param msg 待处理消息
*  \param size 消息长度
*/
void ProcessMessage( PCTRL_MSG msg, uint16_t size )
{
	uint8_t cmd_type = msg->cmd_type;                                               //指令类型
	uint8_t ctrl_msg = msg->ctrl_msg;                                               //消息的类型
	uint8_t control_type = msg->control_type;                                       //控件类型
	uint16_t screen_id = PTR2U16(&msg->screen_id);                                  //画面ID
	uint16_t control_id = PTR2U16(&msg->control_id);                                //控件ID
	uint32_t value = PTR2U32(msg->param);                                           //数值

	switch(cmd_type)
	{  
		case NOTIFY_TOUCH_PRESS:                                                        //触摸屏按下
		case NOTIFY_TOUCH_RELEASE:                                                      //触摸屏松开
				NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4)); 
				break;                                                                    
		case NOTIFY_WRITE_FLASH_OK:                                                     //写FLASH成功
				NotifyWriteFlash(1);                                                      
				break;                                                                    
		case NOTIFY_WRITE_FLASH_FAILD:                                                  //写FLASH失败
				NotifyWriteFlash(0);                                                      
				break;                                                                    
		case NOTIFY_READ_FLASH_OK:                                                      //读取FLASH成功
				NotifyReadFlash(1,cmd_buffer+2,size-6);                                     //去除帧头帧尾
				break;                                                                    
		case NOTIFY_READ_FLASH_FAILD:                                                   //读取FLASH失败
				NotifyReadFlash(0,0,0);                                                   
				break;                                                                    
		case NOTIFY_READ_RTC:                                                           //读取RTC时间
				NotifyReadRTC(cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7],cmd_buffer[8]);
				break;
		case NOTIFY_CONTROL:
				{
						if(ctrl_msg==MSG_GET_CURRENT_SCREEN)                                    //画面ID变化通知
						{
								NotifyScreen(screen_id);                                            //画面切换调动的函数
						}
						else
						{
								switch(control_type)
								{
								case kCtrlButton:                                                   //按钮控件
										NotifyButton(screen_id,control_id,msg->param[1]);                  
										break;                                                             
								case kCtrlText:                                                     //文本控件
										NotifyText(screen_id,control_id,msg->param);                       
										break;                                                             
								case kCtrlProgress:                                                 //进度条控件
										NotifyProgress(screen_id,control_id,value);                        
										break;                                                             
								case kCtrlSlider:                                                   //滑动条控件
										NotifySlider(screen_id,control_id,value);                          
										break;                                                             
								case kCtrlMeter:                                                    //仪表控件
										NotifyMeter(screen_id,control_id,value);                           
										break;                                                             
								case kCtrlMenu:                                                     //菜单控件
										NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);      
										break;                                                              
								case kCtrlSelector:                                                 //选择控件
										NotifySelector(screen_id,control_id,msg->param[0]);                
										break;                                                              
								case kCtrlRTC:                                                      //倒计时控件
										NotifyTimer(screen_id,control_id);
										break;
								default:
										break;
								}
						} 
						break;  
				} 
		case NOTIFY_HandShake:         //握手通知                                                     
				NOTIFYHandShake();
				break;
		default:
				break;
	}
}
/*! 
*  \brief  握手通知
*/
void NOTIFYHandShake()
{
// SetButtonValue(1,101,0); //触摸屏和MCU握手成功，按下屏幕1的步进按钮。
}

/*! 
*  \brief  画面切换通知
*  \details  当前画面改变时(或调用GetScreen)，执行此函数
*  \param screen_id 当前画面ID
*/
void NotifyScreen(uint16_t screen_id)
{
 current_screen_id = screen_id;  //在工程配置中开启画面切换通知，记录当前画面ID  
}

/*! 
*  \brief  触摸坐标事件响应
*  \param press 1按下触摸屏，0松开触摸屏
*  \param x 横坐标
*  \param y 纵坐标
*/
void NotifyTouchXY(uint8_t press,uint16_t x,uint16_t y)
{ 
    //可以做隐形按钮，用户看不到，仅供厂家调试使用。
}


/*! 
*  \brief  更新数据
*/ 
void UpdateUI(void)
{
// Yout(0,2);//呼吸灯
// CAN_Tx_Msg(CANmsg[current_control_id]);
 }
 /*! 
*  \brief  按钮控件通知
*  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param state 按钮状态：0弹起，1按下
*/

void NotifyButton(uint16_t screen_id, uint16_t control_id, uint8_t state)
{ 
	current_screen_id=screen_id;
	if(control_id==433){K(0,state);for(int i=1;i<17;i++)SetButtonValue(current_screen_id,i+8,state);}//报警灯全开全灭
	if(control_id==101 && state==1) status=1;//进入自动步进
	else if(control_id==800) status=2;else status=0;//进入教学演示
	if(control_id==808){ if(state)status=2;else status=0;}//老化不能用其它键暂停
	if(control_id>50 && control_id<60)Fuel(control_id-50);  //燃油电阻档
	if(control_id>60 && control_id<70)Cool(control_id-60);  //水温电阻档
	if(control_id>70 && control_id<80)Gas(control_id-70);   //气压电阻档
	if(control_id>80 && control_id<90)Fluid(control_id-80); //油压电阻档
	if(control_id==119)IGN(state);//点火熄火
	if(control_id>25 && control_id<=99)current_control_id=control_id;//固化按钮
	if(screen_id==3)K(control_id,state);//继电器开关测试;
	else if(control_id>0 && control_id<=8)Yout(control_id,state);//IO开关
	else if(control_id>8 && control_id<25) K(control_id-8,state);//继电器开关
	else return;
}//按钮操作结束

/*! 
*  \brief  文本控件通知
*  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
*  \details  文本控件的内容以字符串形式下发到MCU，如果文本控件内容是浮点值，
*  \details  则需要在此函数中将下发字符串重新转回浮点值。
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param str 文本控件内容
*/
void NotifyText(uint16_t screen_id, uint16_t control_id, uint8_t *str)
{
	sscanf((const char*)str,"%ld",&Value); //字符转换成十进制数据
  sscanf((const char*)str,"%lx",&Data);  //字符转换成十六进制数据

	/*BUS_ID输入*/
  if(control_id ==209) CANmsg[50].Id = Data;    //CAN_Id
	/*DATA[8]输入*/
	if(control_id ==201) CANmsg[50].Data[0] = Data;	//DATA0
  if(control_id ==202) CANmsg[50].Data[1] = Data;	//DATA1
	if(control_id ==203) CANmsg[50].Data[2] = Data;	//DATA2
	if(control_id ==204) CANmsg[50].Data[3] = Data;	//DATA3
	if(control_id ==205) CANmsg[50].Data[4] = Data;	//DATA4
	if(control_id ==206) CANmsg[50].Data[5] = Data;	//DATA5
	if(control_id ==207) CANmsg[50].Data[6] = Data;	//DATA6
	if(control_id ==208) CANmsg[50].Data[7] = Data;	//DATA7
	
	if(control_id ==164) DataR(Value);//数字电阻
	if(control_id ==168) {Hz=Value;  PWM(ch,Hz,Duty);}//PWM频率
	if(control_id ==169) {Duty=Value;PWM(ch,Hz,Duty);}//PWM占空比
	if(control_id ==710) LifeTime=Value;//老化时间设置
	//PWM更新
} //文本输入结束                                                                               

/*!                                                                              
*  \brief  进度条控件通知                                                       
*  \details  调用GetControlValue时，执行此函数                                  
*  \param screen_id 画面ID                                                      
*  \param control_id 控件ID                                                     
*  \param value 值                                                              
*/                                                                              
void NotifyProgress(uint16_t screen_id, uint16_t control_id, uint32_t value)           
{  
//if(screen_id==1 && control_id==1234)
//  BRIGHT=(uint8_t)value;//滚动条改变亮度
}                                                                                

/*!                                                                              
*  \brief  滑动条控件通知                                                       
*  \details  当滑动条改变(或调用GetControlValue)时，执行此函数                  
*  \param screen_id 画面ID                                                      
*  \param control_id 控件ID                                                     
*  \param value 值                                                              
*/                                                                              
void NotifySlider(uint16_t screen_id, uint16_t control_id, uint32_t value)             
{                                                             
    //TODO: 添加用户代码
}

/*! 
*  \brief  仪表控件通知
*  \details  调用GetControlValue时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param value 值
*/
void NotifyMeter(uint16_t screen_id, uint16_t control_id, uint32_t value)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  菜单控件通知
*  \details  当菜单项按下或松开时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param item 菜单项索引
*  \param state 按钮状态：0松开，1按下
*/
void NotifyMenu(uint16_t screen_id, uint16_t control_id, uint8_t item, uint8_t state)
{
	if(control_id ==112) SetScreen(6+item);//车型选择
	/*画面2，CAN设置,包含波特率，帧格式，预置id*/
	
	if(control_id ==212) //波特率选择
	{
	if(item==0)CAN_Baud(500);//波特率500k
	if(item==1)CAN_Baud(250);//波特率250k
	}
	
	if(control_id ==215) //帧格式选择
	{
	if(item==0) Header.IDE=CAN_ID_STD;  //标准帧
	if(item==1) Header.IDE=CAN_ID_EXT;  //扩展帧
	}

	if(control_id ==218) //CAN_ID预选
	{
		if(item==0) {CANmsg[0].Id = 0x0CF00400;  SetTextValue(2,209,(uint8_t*)&"0CF00400");}
		if(item==1) {CANmsg[0].Id = 0x613;       SetTextValue(2,209,(uint8_t*)&"北汽613"); }
		if(item==2) {CANmsg[0].Id = 0x18FEEF00;  SetTextValue(2,209,(uint8_t*)&"18FEEF00");}
		if(item==3) {CANmsg[0].Id = 0x10F8108D;  SetTextValue(2,209,(uint8_t*)&"10F8108D");}
		if(item==4) {CANmsg[0].Id = 0x18FEEE00;  SetTextValue(2,209,(uint8_t*)&"18FEEE00");}
		if(item==5) {CANmsg[0].Id = 0x10F8109A;  SetTextValue(2,209,(uint8_t*)&"10F8109A");}
	}
	
	if(control_id ==162) //电阻通道
	{
		if(item==0) {K(29,1);K(30,0);K(31,0);K(32,0);}//燃油
		if(item==1) {K(29,0);K(30,1);K(31,0);K(32,0);}//水温
		if(item==2) {K(29,0);K(30,0);K(31,1);K(32,0);}//气压
		if(item==3) {K(29,0);K(30,0);K(31,0);K(32,1);}//油压
	}
	
	if(control_id ==166) //PWM通道
	{
		if(item==0) ch=1;//转速
		if(item==1) ch=2;//车速
	}
}//选择菜单结束

/*! 
*  \brief  选择控件通知
*  \details  当选择控件变化时，执行此函数
*  \param screen_id 画面ID
*  \param control_id 控件ID
*  \param item 当前选项
*/
void NotifySelector(uint16_t screen_id, uint16_t control_id, uint8_t  item)
{
 //TODO: 添加用户代码
}


/*! 
*  \brief  定时器超时通知处理
*  \param screen_id 画面ID
*  \param control_id 控件ID
*/
void NotifyTimer(uint16_t screen_id, uint16_t control_id)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  读取用户FLASH状态返回
*  \param status 0失败，1成功
*  \param _data 返回数据
*  \param length 数据长度
*/
void NotifyReadFlash(uint8_t status,uint8_t *_data,uint16_t length)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  写用户FLASH状态返回
*  \param status 0失败，1成功
*/
void NotifyWriteFlash(uint8_t status)
{
    //TODO: 添加用户代码
}

/*! 
*  \brief  读取RTC时间，注意返回的是BCD码
*  \param year 年（BCD）
*  \param month 月（BCD）
*  \param week 星期（BCD）
*  \param day 日（BCD）
*  \param hour 时（BCD）
*  \param minute 分（BCD）
*  \param second 秒（BCD）
*/
void NotifyReadRTC(uint8_t year,uint8_t month,uint8_t week,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
    //TODO: 添加用户代码，可以借鉴大彩触摸屏的案例
}

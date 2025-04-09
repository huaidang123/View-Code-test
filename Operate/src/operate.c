/************************************用户操作********************************************
**余姚韩慧自动化设备有限公司 安徽隽世电子科技有限公司 安徽机电职业技术学院
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   operate.c
** 修改时间:   2022-04-08
** 文件说明:   用户操作函数库
** 技术支持    华工 Tel:13855361411（微信同号）
----------------------------------------------------------------------------------------*/
#include "operate.h"      //自定义操作方式
#include "Vehicle.h"      //车辆硬控参数
#include "usart.h"        //串口收发函数
#include "gpio.h"         //开关量操作
#include "can.h"          //汽车双绞总线
#include "rs485.h"        //工业差分总线
#include "tim.h"          //定时器PWM
#include "cmd_process.h"  //触摸屏下载指令

extern uint16_t  Step;           //检测步骤
extern CanMsgTypeDef CANmsg[];  //CAN报文数组
extern uint16_t current_screen_id; //当前画面ID
extern uint8_t status;//触摸屏、电位器、步进、耐久选择0,1,2,3
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

void Study(void)//教学演示
{
	int i;
	IGN(1);
	Fuel(5);
	for(i=7;i<17;i++)
	{
	SetScreen(i);//切换页面
	HAL_Delay(300);//停留一秒
	}
	SetScreen(12);//北汽电动
	for(i=1;i<9;i++)
  {
	Yout(i,1);//IO控制检测
	SetButtonValue(12,i,1);	
	HAL_Delay(500);//停留500毫秒
	}
	SetScreen(2);//CAN
	SetTextValue(2,129,(uint8_t*)&"油机转速");//转速
	SetTextValue(2,209,(uint8_t*)&"0CF00400");
	for(i=0;i<0x5dc0;i+=5)
  {
	 CANmsg[50].Data[4]=(i>>8)&0xff;CANmsg[50].Data[3]=i&0xff;
   CAN_Tx_Msg(CANmsg[50]);
	 SetTextInt32(2,205,(i>>8)&0xff,0,0);
	 SetTextInt32(2,204,i&0xff,0,0);
	}
	SetTextValue(2,129,(uint8_t*)&"冷却水温");//水温
	SetTextValue(2,209,(uint8_t*)&"18FEEE00");
	for(i=0x50;i<0xa0;i++)
  {
	 CANmsg[70].Data[0]=i;
   CAN_Tx_Msg(CANmsg[70]);
	 HAL_Delay(100);
   SetTextInt32(2,201,i,0,0);
	}
	SetTextValue(2,129,(uint8_t*)&"机油压力");//机油
	SetTextValue(2,209,(uint8_t*)&"18FEEF00");
	for(i=0x00;i<0xfa;i++)
  {
	 CANmsg[80].Data[3]=i;
   CAN_Tx_Msg(CANmsg[80]);
	SetTextInt32(2,204,i,0,0);
	 HAL_Delay(20);
	}
  SetScreen(3);//报警灯
	Cool(5);
	for(i=1;i<17;i++)
  {
  K(i,1);
	SetButtonValue(3,i,1);
	HAL_Delay(500);//停留1秒	
	}
	SetScreen(0);//动画
	for(i=1;i<9;i++)SetButtonValue(12,i,0);
	SetScreen(1);//首页
	for(i=0;i<17;i++)SetButtonValue(3,i,0);
	SetButtonValue(1,800,0);
	IGN(0);
	K(0,0);	
}//切换所有的界面，功能演示

void Test(uint8_t Model,uint8_t num) //单步检测,Model是型号，step是单步动作
{int i;
if(num==0){SetTextValue(Model,256,(uint8_t*)&"归零");\
           SetScreen(Model);IGN(0);}//初始化，断电更换仪表
if(num==1){SetTextValue(Model,256,(uint8_t*)&"点火");IGN(1);\
	         for(i=0;i<100;i++){SetButtonValue(Model,i,0);HAL_Delay(10);SetButtonValue(Model,119,i%2);}
           }//给仪表自检留出时间
if(num==2){SetTextValue(Model,256,(uint8_t*)&"灯检");\
           K(0,1);HAL_Delay(1000);K(0,0);}//快速检灯
if(num==3){SetTextValue(Model,256,(uint8_t*)&"频率");\
	         for(i=0;i<10;i++){RPM(i*100);SetButtonValue(Model,40+i,1);HAL_Delay(300);}
           }//频率转速
if(num==4){SetTextValue(Model,256,(uint8_t*)&"总线");\
	         for(i=0;i<100;i++){CANmsg[50].Data[4]=i;CAN_Tx_Msg(CANmsg[50]);HAL_Delay(20);\
          SetButtonValue(Model,40+i/10,0);}}//CAN转速
if(num==5){SetTextValue(Model,256,(uint8_t*)&"燃油");
	         for(i=1;i<10;i+=2){Fuel(i); SetButtonValue(Model,50+i,1);\
	         CANking(CANmsg[50+i]);}}//电阻1，陕汽的燃油改为发送报文           
if(num==6){SetTextValue(Model,256,(uint8_t*)&"水温");
	        for(i=1;i<10;i+=2){Cool(i); SetButtonValue(Model,60+i,1);\
           CANking(CANmsg[60+i]);}}//电阻2，水温报文
if(num==7){SetTextValue(Model,256,(uint8_t*)&"油压");
	         for(i=1;i<10;i+=2){Gas(i); SetButtonValue(Model,70+i,1); \
           CANking(CANmsg[70+i]);}}//电阻3，机油压力报文
if(num==8){SetTextValue(Model,256,(uint8_t*)&"气压");
	        for(i=1;i<10;i+=2){Fluid(i);HAL_Delay(100);SetButtonValue(Model,80+i,1);\
//          CANking(CANmsg[80+i]);
          }}//气压是模拟量
if(num>8 && num<25){SetTextInt32(Model,256,num,0,0);K(num-8,1);SetButtonValue(Model,num,1);\
          HAL_Delay(500);K(num-7,0); CANking(CANmsg[num]);\
           SetButtonValue(Model,num+1,0);}//下一个灯熄灭，是后退检测的需要         
if(num>24 && num<30){SetTextInt32(Model,256,num,0,0);HAL_Delay(50);
	        CANking(CANmsg[num]);SetButtonValue(Model,num,1);SetButtonValue(Model,num+1,0);
          }//排放软件灯，司机提醒无效
if(num==30){IGN(0);K(0,0);SetButtonValue(Model,101,0);Step=0;status=0;\
           for(i=0;i<100;i++)//步进按钮弹起,检测结束，报告OK
           {if(i%2)SetTextValue(Model,256,(uint8_t*)&"OK");else SetTextValue(Model,256,(uint8_t*)&"良好");}}
else return;
}

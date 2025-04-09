#ifndef __OPERATE_H
#define __OPERATE_H
#endif
#include "main.h"    //嵌入式系统配置

/*! 
*  \brief  自动检测步骤
*  \param  油门、按键、触摸屏，调用检测步骤
*/
void AutoCheck(void); //全自动检测
void Test(uint8_t Model,uint8_t num);//仪表型号,检测步骤
void Study(void);//教学演示
/************************自定义操作*****END OF FILE****/

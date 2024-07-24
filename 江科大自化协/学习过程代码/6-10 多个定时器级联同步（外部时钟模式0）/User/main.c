#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "TIM2_master.h"
#include "TIM3_slave_master.h"
#include "TIM1_slave.h"
#include "IC.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	
	TIM2_PWM_Init();	//TIM2-CH2-PA1。频率10kHz、占空比50%
	TIM3_PWM_Init();	//TIM3-CH1-PA6。频率2kHz、占空比20%
	//TIM1_PWM_Init();	//TIM1-CH1-PA8。频率1kHz、占空比50%
	
	// 定时器初始化配置都完成后，连贯启动计数器，这样是否可以尽量减小相位差?
	// 是否需要在main()中执行TIM_Cmd()函数，这样保证级联的启动连贯，不然分在3个初始化函数中，不够连贯。
	// 实测，没区别
	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM3, ENABLE);	
	//TIM_Cmd(TIM1, ENABLE);	
	
	IC_Init();		    //输入捕获初始化，测量TIM2和TIM3级联（注释掉TIM1_PWM_Init()和TIM_Cmd()），时用TIM1-CH1-PA8输入捕获。
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Freq:00000Hz");		//1行1列显示字符串Freq:00000Hz
	OLED_ShowString(2, 1, "Duty:00%");			//2行1列显示字符串Duty:00%
	
	while (1)
	{
		OLED_ShowNum(1, 6, IC_GetFreq(), 5);	//不断刷新显示输入捕获测得的频率
		OLED_ShowNum(2, 6, IC_GetDuty(), 2);	//不断刷新显示输入捕获测得的占空比
	}
}

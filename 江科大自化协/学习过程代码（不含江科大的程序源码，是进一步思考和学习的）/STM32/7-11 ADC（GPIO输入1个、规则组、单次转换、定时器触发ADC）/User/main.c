#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "AD.h"
#include "TIM1_CC1.h"
#include "TIM3_TRGO.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();			//OLED初始化
	LED_Init(); 			//LED0（贴片LED）初始化
	AD_Init();				//AD初始化
	
	TIM3_TRGO_Init();	
	/*
	      如果用TIM3_TRGO，需要配置主模式输出TRGO，但是不需要配置输出比较OC结构体；
	
	      需要在AD.c中设置结构体的成员 ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;		//（只针对规则组）外部触发源，使用TIM3的TRGO
	
		  TIM3的更新周期是1S；在DMA1_Channel1_IRQHandler中翻转LED的电平，看出来确实是1s。
		  而且数据变动也是1s一次。即使OLED是100s刷新一次
	*/		
	
	// TIM1_CC1_Init();
	/*    
	      如果用TIM1_CC1，需要配置输出比较OC结构体，但是不需要配置主模式输出TRGO；
	
	      需要在AD.c中设置结构体的成员 ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;		//（只针对规则组）外部触发源，使用TIM1的CC1	
	
		  TIM2的更新周期是5S，那么输出比较触发的频率也是5S；在ADC1_2_IRQHandler中翻转LED的电平，看出来确实是5s。
		  而且数据变动也是5s一次。即使OLED是100s刷新一次
	      
		  此外，我们只需要输出比较事件，来触发ADC转换，不需要实际输出PWM波形
	      因此，在TIM1_CC1_Init()中，我们没有配置PA8	  	  
	*/	
	
	
	LED0_ON();
		
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "AD0:");
	
	
	while (1)
	{	
		//LED0_Turn();
		
		OLED_ShowNum(1, 5, AD_Value, 4);		//显示转换结果
		
		Delay_ms(100);			//延时100ms，手动增加一些转换的间隔时间
	}
}

/**
  * 函    数：ADC1_2中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void ADC1_2_IRQHandler(void)
{
	if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)           //判断是否是ADC1中的EOC（规则组或注入组转换结束）触发的中断
	{
		//ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);			//清除ADC1中的EOC（规则组或注入组转换结束）事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
		
		LED0_Turn();  // 翻转LED，指示进入中断的频率
		
		AD_Value = ADC_GetConversionValue(ADC1);    // 读取ADC_DR寄存器
		
	    // 清除EOC、JEOC标志位
	    // ADC_ClearFlag(ADC1, ADC_FLAG_JEOC); 	
	    // ADC_ClearFlag(ADC1, ADC_FLAG_EOC); 	    // 读取了ADC_DR寄存器，会由硬件清除EOC标志位
	}
}

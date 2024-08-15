#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "AD.h"
#include "EXTI_15.h"

uint16_t adc_it_num;   // 记录进入ADC全局中断的次数

int main(void)
{
	/*模块初始化*/
	OLED_Init();			//OLED初始化
	LED_Init(); 			//LED0（贴片LED）初始化
	AD_Init();				//AD初始化
	
	EXTI_15_Init();	
	//ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4); //（只针对注入组）外部触发源，使用EXTI的15号线EXTI_15	
	/*
	      如果用EXTI_15，需要配置EXTI_15外部中断线为事件模式（注意，不是中断模式，这里EXTI_15仅作为事件，来触发ADC）
	
	      需要在AD.c中调用，或在main.c中调用ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4);
	
	      由于规则组的EXTI触发源只有EXTI_15线，STM32F103C8T6最小系统板上有PA15和PB15的引脚，所以将外部中断EXTI_15线映射到PA15和PB15都可以
	      为了提供上升沿，将User_Key连接到PB15（PA15）。这样，按下Key时，就会给PB15（PA15）一个上升沿，从而顺利触发一次ADC。
	      一轮扫描结束后，DMA也搬运结束，就会触发DMA中断。在DMA1_Channel1_IRQHandler中翻转LED的电平，看出来确实与Key同步。
	*/	
	
	
	LED0_ON();

	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "AD0:");
	OLED_ShowString(2, 1, "AD1:");
	OLED_ShowString(3, 1, "AD2:");
	OLED_ShowString(4, 1, "AD3:");
	
	
	while (1)
	{	
		//LED0_Turn();
		
		OLED_ShowNum(1, 5, AD_Value[0], 4);		//显示转换结果第0个数据
		OLED_ShowNum(2, 5, AD_Value[1], 4);		//显示转换结果第1个数据	
		OLED_ShowNum(3, 5, AD_Value[2], 4);		//显示转换结果第0个数据
		OLED_ShowNum(4, 5, AD_Value[3], 4);		//显示转换结果第1个数据	
		
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
	/*
	  如果在AD.c中，有ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);， 则使用if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
	  如果在AD.c中，有ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);，则使用if (ADC_GetITStatus(ADC1, ADC_IT_JEOC) == SET)	
	  当然，由于我们在AD.c中，JEOC和EOC的中断都使能了，都可以触发ADC全局中断ADC1_2_IRQHandler
	  这里，用哪个判断都可以
	*/
	//if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)         //判断是否是ADC1中的EOC（规则组或注入组转换结束）触发的中断	
	if (ADC_GetITStatus(ADC1, ADC_IT_JEOC) == SET)          //判断是否是ADC1中的JEOC（注入组转换结束）触发的中断
	{
		//ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
		ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);			//清除ADC1中的JEOC注入组转换结束事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
		
		LED0_Turn();  // 翻转LED，指示进入中断的频率
		/*
		  TIM2的更新周期是1S，那么输出比较触发的频率也是1S，
		  在ADC1_2_IRQHandler中翻转LED的电平，看出来确实是1s。
		  而且数据变动也是1s一次。即使OLED是100ms刷新一次
		*/
		
		
		adc_it_num++;   // 记录进入ADC全局中断的次数
		
		
		/* 读取ADC注入组的数据寄存器JDRx */
		AD_Value[0] = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1); // 读取ADC1_JDR1
		AD_Value[1] = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2); // 读取ADC1_JDR2	
		AD_Value[2] = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_3); // 读取ADC1_JDR3
		AD_Value[3] = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_4); // 读取ADC1_JDR4	

	    /* 清除EOC、JEOC标志位	*/
	    ADC_ClearFlag(ADC1, ADC_FLAG_JEOC); 	
	    ADC_ClearFlag(ADC1, ADC_FLAG_EOC); 	 	
	}
}

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "AD.h"
#include "LED.h"

uint16_t WDE_FLAG_data;

int main(void)
{
	/*模块初始化*/
	OLED_Init();			//OLED初始化
	LED_Init(); 			//LED0（贴片LED）初始化
	AD_Init();				//AD初始化
	
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "AWD_HT:");
	OLED_ShowString(2, 1, "ADC[0]:");
	OLED_ShowString(3, 1, "ADW_LT:");
	OLED_ShowString(4, 1, "Normal.");

	
	while (1)
	{	
		Delay_ms(100);			//延时100ms，手动增加一些转换的间隔时间
		
		OLED_ShowNum(1, 8, ADC_WATCHDOG_HT, 4);		//显示ADC模拟看门狗的上限，全局变量ADC_WATCHDOG_HT在AD.c中定义
		OLED_ShowNum(2, 8, AD_Value[0], 4);		    //显示转换结果第0个数据
		OLED_ShowNum(3, 8, ADC_WATCHDOG_LT, 4);		//显示ADC模拟看门狗的下限，全局变量ADC_WATCHDOG_LT在AD.c中定义	

        /*
		   ADC每次转换完成，模拟看门狗都会检测。如果数值超限，就触发看门狗中断
		   如果一直保持超限，那么看门狗中断，和AD转换结束中断时同样效果的
		*/
		if(WDE_FLAG_data == 1)
		{
			WDE_FLAG_data = 0;
			LED0_OFF();
	        OLED_ShowString(4, 1, "Warning");			
		} else {
	        OLED_ShowString(4, 1, "Normal.");			
		}
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
	  如果在AD.c中，有ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);， 则使用if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
	*/
	if (ADC_GetITStatus(ADC1, ADC_IT_AWD) == SET)          //判断是否是ADC1中的AWD（转换结果超出模拟看门狗的上下限）触发的中断
	{
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);			//清除ADC1中的AWD模拟看门狗监测到超限事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
		
		LED0_Turn();  // 翻转LED，指示进入中断的频率	
		WDE_FLAG_data = 1;
	}
}

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "AD.h"

float temperature;
float vref_value;

int main(void)
{
	/*模块初始化*/
	OLED_Init();			//OLED初始化
	LED_Init(); 			//LED0（贴片LED）初始化
	AD_Init();				//AD初始化
	
	LED0_OFF();
		
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "AD0:");
	OLED_ShowString(2, 1, "Temp:");
	OLED_ShowString(3, 1, "AD1:");
	OLED_ShowString(4, 1, "Vref:");
	
	
	while (1)
	{	
		AD_StartConv();			//软件触发AD转换一次
		
		OLED_ShowNum(1, 5, AD_Value[0], 4);		//显示转换结果第0个数据
		OLED_ShowNum(3, 5, AD_Value[1], 4);		//显示转换结果第1个数据			
		
		Delay_ms(100);			//延时100ms，手动增加一些转换的间隔时间
		
		/* 内部温度传感器 */
		temperature = (1.43 - AD_Value[0]*3.3/4096) * 1000 / 4.3 + 25;
		/*
		  看STM32F10xxx英文参考手册（11.10 Temperature sensor）
		               Temperature (in °C) = {(V25 - VSENSE) / Avg_Slope} + 25.
		               Vsense：ADC采集得到的值，对应电压值（单位：V）
		  看STM32F103x8B英文数据手册（5.3.19 Temperature sensor characteristics）
		               V25：Voltage at 25 °C，即1.43V。Vsense在25°C时的数值
		               Avg_Slope：4.3 mV/°C，换算为4.3/1000 V/°C。温度与Vsense曲线的平均斜率（单位为mV/°C 或 V/°C）
		               1 / Avg_Slope：1000 / 4.3 °C/V。
		*/
		
		/* 内部参考电压Vrefint */
		vref_value = (AD_Value[1] * 3.3 / 4096);
		
		OLED_ShowNum(2, 6, (uint16_t)temperature, 4);		//显示内部温度传感器（整数部分）
		OLED_ShowNum(4, 6, (uint16_t)vref_value, 4);		//显示内部参考电压值（整数部分）		
		
		LED0_OFF();  // AD_StartConv();	内部会点亮LED0，这样就会有闪烁的效果
		
		Delay_ms(100);			//延时100ms，手动增加一些转换的间隔时间
	}
}

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "AD.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();			//OLED初始化
	LED_Init(); 			//LED0（贴片LED）初始化
	AD_Init();				//AD初始化
	
	LED0_OFF();
		
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "AD0:");
	OLED_ShowString(2, 1, "AD1:");
	OLED_ShowString(3, 1, "AD2:");
	OLED_ShowString(4, 1, "AD3:");
	//OLED_ShowString(5, 1, "AD4:"); // OLED没有第5、6、7行
	//OLED_ShowString(6, 1, "AD5:");
	//OLED_ShowString(7, 1, "AD6:");
		
	
	while (1)
	{	
		AD_StartConv();			//软件触发AD转换一次
		
		OLED_ShowNum(1, 5, AD_Value[0], 4);		//显示转换结果第0个数据
		OLED_ShowNum(2, 5, AD_Value[1], 4);		//显示转换结果第1个数据
		OLED_ShowNum(3, 5, AD_Value[2], 4);		//显示转换结果第2个数据
		OLED_ShowNum(4, 5, AD_Value[3], 4);		//显示转换结果第3个数据		
		//OLED_ShowNum(5, 5, AD_Value[4], 4);		//显示转换结果第4个数据
		//OLED_ShowNum(6, 5, AD_Value[5], 4);		//显示转换结果第5个数据
		//OLED_ShowNum(7, 5, AD_Value[6], 4);		//显示转换结果第6个数据				
		
		Delay_ms(100);			//延时100ms，手动增加一些转换的间隔时间
		
		LED0_OFF();  // AD_StartConv();	内部会点亮LED0，这样就会有闪烁的效果
		
		Delay_ms(100);			//延时100ms，手动增加一些转换的间隔时间
	}
}

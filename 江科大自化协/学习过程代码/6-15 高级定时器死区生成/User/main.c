#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "TIM1_deadtime_break.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();		            // OLED初始化
	
	TIM1_deadtime_break_0_Init();	// TIM1死区插入
	
	//TIM1_deadtime_break_1_Init();	// CH1的正脉冲宽度24us > CH1N负脉冲宽度16us > 死区时间8.4us	
	
	//TIM1_deadtime_break_2_Init();	// CH1的正脉冲宽度24us > 死区时间16.9us > CH1N负脉冲宽度16us
	
	//TIM1_deadtime_break_3_Init();	// 死区时间8.4us < CH1的正脉冲宽度16us < CH1N负脉冲宽度24us
	
	//TIM1_deadtime_break_4_Init();	// CH1的正脉冲宽度16us < 死区时间16.9us < CH1N负脉冲宽度24us
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Outputing");		//1行1列显示字符串Outputing
	
	while (1){
	}
}

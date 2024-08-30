#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "SinglePWM_Dma.h"
#include "MultiPWM_Timer_Dma_Mode.h"

int main(void)
{
	/*模块初始化*/
	OLED_Init();		    //OLED初始化
	
	
	// ----------------- PWM输出初始化 ---------------
	// DMA1的CH2、CH3、CH6、CH4、CH5分别对应TIM1的CH1、CH2、CH3、CH4、UP等DMA请求
	
	// 只控制一个通道的PWM输出，使用普通DMA模式，单一触发源搬运单一地址值
	//SinglePWM_DMA_Init();	

	// 控制多个通道的PWM输出，使用定时器DMA模式，单一触发源搬运多个地址值（地址范围内的寄存器都会被访问）
	MultiPWM_Timer_DMA_Mode_Init();		

	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Outputing");		//1行1列显示字符串Outputing
		
	while (1){
	}
}

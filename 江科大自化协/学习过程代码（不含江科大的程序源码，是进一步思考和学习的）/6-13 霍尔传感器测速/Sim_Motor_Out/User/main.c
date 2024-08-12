#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "SimMotorOut.h"

uint32_t gSimStep;	

int main(void)
{
	/*模块初始化*/
	OLED_Init();		         //OLED初始化
	Sim_Motor_Output_Init();     //定时中断初始化
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Outputing");			//1行1列显示字符串Outputing
	
	while (1){
	}
}

/**
  * 函    数：TIM2中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)		//判断是否是TIM2的更新事件触发的中断
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);			//清除TIM2更新事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
		Sim_Motor_Prepare_Comutation();
		
	}
}

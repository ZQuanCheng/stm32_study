#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"
#include "HALL.h"

uint32_t Freq;        // 霍尔传感器ABC相的频率
uint16_t CNT_CCR;     // CCR锁存值

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	Timer_Init();		//定时中断初始化
	Hall_Init();		//霍尔传感器接口初始化
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "CCR:");		    //1行1列显示字符串CCR:	
	OLED_ShowString(2, 1, "Speed:");		//1行1列显示字符串Speed:
	
	while (1)
	{
		OLED_ShowSignedNum(1, 5, CNT_CCR, 5);	//不断刷新显示CCR锁存的最新值		
		OLED_ShowSignedNum(2, 7, Freq, 8);	    //不断刷新显示测得的霍尔传感器ABC相最新频率
	}
}

/**
  * 函    数：TIM3中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)		//判断是否是TIM3的更新事件触发的中断
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);			//清除TIM3更新事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
		
		CNT_CCR = Hall_Get();                 // 每隔固定时间段，读取一次TIM2的CCR锁存值
		
		// 我们如何确定转速？从《STM32参考手册13.3.18》的图93（或者《GD32参考手册16.1.4.14》的图16-21）可知，
		// 对于一个A相通道，其输入周期的1/6为CCR的锁存值。
		// 我们读出来CCR的锁存值，除以CNT计数频率CK_CNT，得到一个时间值；乘以6，就可以得到A相的周期时间，倒数就是频率。频率可以换算为转速。
		Freq = 1000000 / (CNT_CCR * 6);       // 计数时钟频率为72MHz / (72-1+1) = 1MHz。
	}
}

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"
#include "Encoder.h"

int32_t Speed;			 // 定义速度变量

// 正转CNT从0自增。但是反转CNT从0减到65535、65534、...；转换为int16_t后为0、-1、-2、...
uint16_t Incremental;    // 增量值
int16_t Dir_incremental; // 有符号增量值

int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	Timer_Init();		//定时器初始化
	Encoder_Init();		//编码器初始化
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Incre:");		//1行1列显示字符串Speed:	
	OLED_ShowString(2, 1, "Speed:");		//1行1列显示字符串Speed:
	
	while (1)
	{
		OLED_ShowSignedNum(1, 7, Dir_incremental, 5);	//不断刷新显示编码器测得的最新增量值		
		OLED_ShowSignedNum(2, 7, Speed, 8);	            //不断刷新显示编码器测得的最新速度
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
		
		Incremental = Encoder_Get();	    // 每隔固定时间段读取一次编码器计数增量值，即为速度值
		Dir_incremental = Incremental;      // int16_t转为int16_t，这样根据正负，可以判断转动的方向。
		Speed = Dir_incremental * 1;        // 根据定时中断的时间，换算速度值。定时中断1s一次（1Hz）。
	}
}

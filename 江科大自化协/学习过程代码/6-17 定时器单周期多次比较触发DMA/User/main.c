#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "SinglePWM_Dma_multitrigger.h"
  
uint32_t TIM1_UP_num = 0;  // 统计进入更新中断的次数
uint32_t TIM1_CC_num = 0;  // 统计进入比较中断的次数

int main(void)
{
	/*模块初始化*/
	OLED_Init();		    //OLED初始化
	
	
	// ----------------- PWM输出初始化 ---------------
	// DMA1的CH2、CH3、CH6、CH4、CH5分别对应TIM1的CH1、CH2、CH3、CH4、UP等DMA请求
		
    // DMA模式，实现在TIM1单个周期内多次比较触发
	SinglePWM_DMA_MultiTRIG_Init();
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Outputing");		//1行1列显示字符串Outputing
		
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);				 //将PB15引脚初始化为推挽输出
	/*设置GPIO初始化后的默认电平*/
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);				     //设置PB15引脚为高电平	
	
	while (1){
//		if((TIM1_CC_num > (TIM1_UP_num * 4)) || (TIM1_CC_num == (TIM1_UP_num * 4))){
//			GPIO_SetBits(GPIOB, GPIO_Pin_15);
//		}
	}
}

/**
  * 函    数：TIM1更新中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)		//判断是否是TIM1的更新事件触发的中断
	{
		//TIM1_UP_num++;
		
		// 拉低PB15电平
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);

		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);			//清除TIM1更新事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
	}
}

/**
  * 函    数：TIM1捕获/比较中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void TIM1_CC_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_CC1) == SET)		   //判断是否是TIM1-通道1的捕获/比较事件触发的中断
	{
		//TIM1_CC_num++;
		//GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		
		// 翻转PB15电平
		if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15) == 0)    //获取输出寄存器的状态，如果当前引脚输出低电平
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_15);					//则设置PB15引脚为高电平
		}
		else													//否则，即当前引脚输出高电平
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_15);					//则设置PB15引脚为低电平
		}

		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);			//清除TIM1-通道1的捕获/比较事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
	}
}

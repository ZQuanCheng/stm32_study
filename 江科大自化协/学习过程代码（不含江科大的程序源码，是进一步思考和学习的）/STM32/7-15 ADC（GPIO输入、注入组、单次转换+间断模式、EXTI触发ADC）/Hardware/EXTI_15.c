#include "stm32f10x.h"                  // Device header

/**
  * 函    数：外部中断EXTI_15初始化
  * 参    数：无
  * 返 回 值：无
  */
void EXTI_15_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		//开启AFIO的时钟，外部中断必须开启AFIO的时钟
	
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//将PB15引脚初始化为下拉输入
	// 如果对应的GPIO配置为IPD下拉模式，外部中断线就配置为上升沿触发EXTI_Trigger_Rising
	// 如果对应的GPIO配置为IPU上拉模式，外部中断线就配置为下降沿触发EXTI_Trigger_Falling	
	
	
	/*AFIO选择中断引脚*/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);//将外部中断的15号线映射到GPIOB，即选择PB15为外部中断引脚
	
	
	/*EXTI初始化*/
	EXTI_InitTypeDef EXTI_InitStructure;						//定义结构体变量
	EXTI_InitStructure.EXTI_Line = EXTI_Line15;					//选择配置外部中断的15号线
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//指定外部中断线使能
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;			    //指定外部中断线为事件模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;		//指定外部中断线为上升沿触发
	EXTI_Init(&EXTI_InitStructure);								//将结构体变量交给EXTI_Init，配置EXTI外设
	// 注意，不是中断模式，这里EXTI_15仅作为事件，来触发ADC
	// 如果对应的GPIO配置为IPD下拉模式，外部中断线就配置为上升沿触发EXTI_Trigger_Rising
	// 如果对应的GPIO配置为IPU上拉模式，外部中断线就配置为下降沿触发EXTI_Trigger_Falling		
	
	
	
	
	
	// ------------------------------------ 既然EXTI_15不是中断模式，就不用配置NVIC了 ---------------------------------------
	
	/*NVIC中断分组*/
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	//NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	//NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;		//选择配置NVIC的EXTI15_10线
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//指定NVIC线路的抢占优先级为1
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	//NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	// -----------------------------------------------------------------------------------------------------------
		
	
	
	
	
	/*清楚外部中断15号线的标志位*/
	//EXTI_ClearITPendingBit(EXTI_Line15);      // 清除外部中断15号线的中断标志位
	EXTI_ClearFlag(EXTI_Line15);              // 清除外部中断15号线的标志位	
}

/**
  * 函    数：EXTI15_10外部中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
/*
void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line15) == SET)		//判断是否是外部中断15号线触发的中断
	{
		//如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15) == 0)
		{
			操作;	
		}
		EXTI_ClearITPendingBit(EXTI_Line15);		//清除外部中断15号线的中断标志位
													//中断标志位必须清除
													//否则中断将连续不断地触发，导致主程序卡死
	}
}
*/

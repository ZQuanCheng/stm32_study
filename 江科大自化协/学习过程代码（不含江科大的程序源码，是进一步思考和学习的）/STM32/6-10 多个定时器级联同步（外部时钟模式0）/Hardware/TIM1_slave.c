#include "stm32f10x.h"                  // Device header

/**
  * 函    数：TIM1输出PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
void TIM1_PWM_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟。TIM1是高级定时器，接在APB2总线上。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟，TIM1-CH1-PA8
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//将PA8引脚初始化为复用推挽输出	
																	//受外设控制的引脚，均需要配置为复用模式		

	/*复位外设TIM1*/	
	TIM_DeInit(TIM1);
	
	/*配置时钟源*/
	//输入时钟源为ITR2（TIM3的TRGO）	
	//TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	// 目标输出：一个频率1kHz、占空比50%，分辨率为1%的PWM波形。对应的 ARR=2-1=1、PSC=1-1=0、CCR=1。
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				  //定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	// 定时频率Freq = 计数器的溢出频率CK_CNT_OV = ITR2 / (PSC + 1) / (ARR + 1) = 2kHz / (1-1+1) / (2-1+1) = 1kHz。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;				  //预分频器，即PSC的值。这里选择不分频
	TIM_TimeBaseInitStructure.TIM_Period = 2 - 1;					  //计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;              //重复计数器，高级定时器才会用到
	// TIM初始化
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM1的时基单元
	// TIM_TimeBaseInit函数末尾，手动产生了更新事件：需要清除定时器更新标志位。若不清除此标志位，则开启中断后，会立刻进入一次中断
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update); 
	
	/*输出比较初始化*/
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);							//结构体初始化，若结构体没有完整赋值
																	//则最好执行此函数，给结构体所有成员都赋一个默认值
																	//避免结构体初值不确定的问题
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//  通道输出状态：输出使能
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Enable;	//  互补通道输出状态：输出使能。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_Pulse = 1;								    //  通道CCR值。占空比duty = CCR / (ARR+1) = 1 / (2-1+1) = 50%
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		    //  通道输出极性，选择为高，若选择极性为低，则输出高低电平取反。
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;	    //  互补通道输出极性，选择为高，若选择极性为低，则输出高低电平取反。	只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCNIdleState_Reset;    //  空闲状态下通道输出：低电平。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;   //  空闲状态下通道输出：低电平。只有高级定时器需要配置	
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);						//将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	
	/*预装功能*/
	// 用来配置CCR寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);	// 这里是Disable，不使用CCR1上的影子寄存器

    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
	
	
	/*TIM1：选择从模式、TRGI输入的触发源*/
	TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_External1);		     // 从模式：选择“外部时钟模式1”															
	TIM_SelectInputTrigger(TIM1, TIM_TS_ITR2);					 // TRGI输入的触发源：选择ITR2	

    /*TIM1：主/从模式使能、TRGO输出的触发模式*/
	// TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable); // 主从/模式：使能
    // TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);        // TRGO输出的触发模式：更新事件。换成TIM_TRGOSource_OC1Ref是否可以

	
	/*TIM使能，启动计数器*/
	// TIM_Cmd(TIM1, ENABLE);			//使能TIM2，定时器开始运行
}

#include "stm32f10x.h"                  // Device header

/**
  * 函    数：TIM3输出PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
void TIM3_PWM_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);			//开启TIM3的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟，TIM3-CH1-PA6
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//将PA6引脚初始化为复用推挽输出	
																	//受外设控制的引脚，均需要配置为复用模式		

	/*复位外设TIM3*/	
	TIM_DeInit(TIM3);
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM3);		//选择TIM3为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	// 目标输出：一个频率1kHz、占空比50%，分辨率为1%的PWM波形。对应的 ARR=100-1=99、PSC=144-1=143、CCR=25。
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				  //定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;       //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	// 定时频率Freq = 计数器的溢出频率CK_CNT_OV = CK_PSC / (PSC + 1) / (ARR + 1) = 72MHz / (144-1+1) / (100-1+1) = 5kHz。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; //计数器模式，选择向下计数
	TIM_TimeBaseInitStructure.TIM_Prescaler = 144 - 1;				  //预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;					  //计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;              //重复计数器，高级定时器才会用到
	// TIM初始化
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM3的时基单元
	// TIM_TimeBaseInit函数末尾，手动产生了更新事件：需要清除定时器更新标志位。若不清除此标志位，则开启中断后，会立刻进入一次中断
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 
	
	/*输出比较初始化*/
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);							//结构体初始化，若结构体没有完整赋值
																	//则最好执行此函数，给结构体所有成员都赋一个默认值
																	//避免结构体初值不确定的问题
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				//输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	//  通道输出状态：输出使能
	// TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Enable;	//  互补通道输出状态：输出使能。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_Pulse = 25;								    //  通道CCR值。占空比duty = CCR / (ARR+1) = 25 / (100-1+1) = 25%
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		    //  通道输出极性，选择为高，若选择极性为低，则输出高低电平取反。
	// TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;	    //  互补通道输出极性，选择为高，若选择极性为低，则输出高低电平取反。	只有高级定时器需要配置
	// TIM_OCInitStructure.TIM_OCIdleState = TIM_OCNIdleState_Reset;    //  空闲状态下通道输出：低电平。只有高级定时器需要配置
	// TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;   //  空闲状态下通道输出：低电平。只有高级定时器需要配置	
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);						//将结构体变量交给TIM_OC1Init，配置TIM3的输出比较通道1
	
	
	/*预装功能*/
	// 用来配置CCR寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);	// 这里是Disable，不使用CCR1上的影子寄存器

	
	/*TIM使能，启动计数器*/
	TIM_Cmd(TIM3, ENABLE);			//使能TIM2，定时器开始运行
}

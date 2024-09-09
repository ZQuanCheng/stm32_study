#include "stm32f10x.h"                  // Device header

/**
  * 函    数：编码器初始化
  * 参    数：无
  * 返 回 值：无
  */
void Encoder_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);			//开启TIM3的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							//将PA6和PA7引脚初始化为上拉输入

	
	/*复位外设TIM3*/
    TIM_DeInit(TIM3);	
	
	
	/*配置时钟源*/
	// TIM_InternalClockConfig(TIM3);		// 不需要配置时钟源选择，因为编码器接口会托管时钟。
			                                // 编码器接口是一个带方向控制的外部时钟，计数时钟是外部的正交编码器信号
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;               //计数周期，即ARR的值。设置为65535，最大量程。利用补码的特性，方便换算为负数。
	TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;                //预分频器，即PSC的值。一般选择不分频。编码器信号作为时钟，驱动计数器。
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM3的时基单元
	// TIM_TimeBaseInit函数末尾，手动产生了更新事件；需要清除定时器更新标志位。若不清除此标志位，则开启中断后，会立刻进入一次中断。
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	// 如果需要单独清除一个通道的中断标志位，可以这样：
	// TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);	
	
	
	/*输入捕获初始化*/
	// 编码器接口只使用输入捕获的滤波器、边沿检测/极性选择。后面的是否交叉/直通、预分频器、CCR等与编码器接口无关。
	// 此外，极性在 TIM_EncoderInterfaceConfig() 中的参数配置，不用在输入捕获结构体中配置。不过，重复配置也没事
	TIM_ICInitTypeDef TIM_ICInitStructure;							//定义结构体变量
	TIM_ICStructInit(&TIM_ICInitStructure);							//结构体初始化，若结构体没有完整赋值
																	//则最好执行此函数，给结构体所有成员都赋一个默认值
																	//避免结构体初值不确定的问题
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;				      //选择配置定时器通道1
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	          //极性：选择为上升沿触发捕获。后续TIM_EncoderInterfaceConfig也会设置，但是作用不一样
	//TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;     //输入信号：选择直通，不交叉。
	//TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;				  //捕获预分频：选择不分频，每次信号都触发捕获
	TIM_ICInitStructure.TIM_ICFilter = 0xF;							      //输入滤波器参数：可以过滤信号抖动
	TIM_ICInit(TIM3, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道
	
	// 结构体已经写入到TIM3的硬件寄存器了。接下来，可以换个参数，还可以继续使用该结构体。
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;				      //选择配置定时器通道2
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	          //极性：选择为上升沿触发捕获。后续TIM_EncoderInterfaceConfig也会设置，但是作用不一样
	//TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;     //输入信号：选择直通，不交叉。
	//TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;				  //捕获预分频：选择不分频，每次信号都触发捕获	
	TIM_ICInitStructure.TIM_ICFilter = 0xF;							      //输入滤波器参数，可以过滤信号抖动
	TIM_ICInit(TIM3, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道
	
	
	/*正交编码器接口配置*/
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
																	//配置编码器模式以及两个输入通道是否反相
																	//注意此时参数的Rising和Falling已经不代表上升沿和下降沿了，而是代表是否反相
																	//此函数必须在输入捕获初始化之后进行，否则输入捕获的配置会覆盖此函数的部分配置
     
	 
    /*自动预装功能：使能*/
	// TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  // 使能或者失能TIMx在CCR1上的预装载寄存器
	                                                      // 仅用于输出比较


	/*TIM使能*/
	TIM_Cmd(TIM3, ENABLE);			//使能TIM3，定时器开始运行
}

/**
  * 函    数：获取编码器的增量值
  * 参    数：无
  * 返 回 值：自上次调用此函数后，编码器的增量值
  */
uint16_t Encoder_Get(void)
{
	/*使用Temp变量作为中继，目的是返回CNT后将其清零*/
	uint16_t Temp;
	Temp = TIM_GetCounter(TIM3);        // 返回uint16_t类型
	                                    // 如果右转编码器（正转），CNT从0自增到10。然后一直左转（反转），可以看到，CNT从10减到0后，会突变为65535。
	
	TIM_SetCounter(TIM3, 0);    // 返回将CNT的值后，将CNT清零
	return Temp;
}

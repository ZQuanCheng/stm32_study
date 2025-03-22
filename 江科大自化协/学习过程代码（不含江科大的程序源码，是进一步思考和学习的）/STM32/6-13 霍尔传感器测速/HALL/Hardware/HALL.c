#include "stm32f10x.h"                  // Device header

/**
  * 函    数：霍尔传感器接口初始化
  * 参    数：无
  * 返 回 值：无
  */
void Hall_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                // GPIO_Mode_IPU
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							     //将PA0、PA1、PA2引脚初始化为浮空输入，也可以为上拉输入

	
	/*复位外设TIM2*/
    TIM_DeInit(TIM2);	
	
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM2);		// 选择TIM2为内部时钟，若不调用此函数，TIM的输入也默认为内部时钟源
			                                
	
	/*时基单元初始化*/
    // 计数器CNT必须通过T1的变化清零（T1到TI1F_ED到TRC到TRGO），不能让CNT到达ARR自动清零。为了保证，就让ARR=65536-1最大值。
	// 此外，预分频器PSC的值也需要尽量大，保证计数器周期长于传感器上的两次变化的时间间隔，即定时频率 < BLDC电机的霍尔传感器信号的换相频率。
    // 但是计数时钟CK_CNT = CK_PCS / (PSC+1)的频率也不能太低，你得保证CNT能变化一些，不能传感器信号变化到来时，CNT还是0。
	// 总结：
	// >>>>>> 计数器CNT的计数时钟频率CK_CNT=CK_PCS/(PSC+1)，要大于（高于）BLDC电机的换相频率；
	// >>>>>> 定时频率=计数器的溢出频率CK_CNT_OVT，要小于（低于）BLDC电机的换相频率；
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PCS / (PSC+1) / (ARR+1) = 72MHz / (72-1+1) / (65536-1+1) = 10~20Hz，即计数周期为65.536ms，大于传感器上的两次变化的时间间隔1/6ms。
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;               //计数周期，即ARR的值。设置为65535，最大量程。
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;               //预分频器，即PSC的值。最大65535U。计数时钟频率CK_CNT为72MHz / (72-1+1) = 1MHz。计数时钟频率 大于 换相频率。
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM3的时基单元
	// TIM_TimeBaseInit函数末尾，手动产生了更新事件；需要清除定时器更新标志位。若不清除此标志位，则开启中断后，会立刻进入一次中断。
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	// 如果需要单独清除一个通道的中断标志位，可以这样：
	// TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);	
	
	
	/*输入捕获初始化*/
	// 编码器接口只使用输入捕获的滤波器、边沿检测/极性选择。后面的是否交叉/直通、预分频器、CCR等与编码器接口无关。
	// 此外，极性在 TIM_EncoderInterfaceConfig() 中的参数配置，不用在输入捕获结构体中配置。不过，重复配置也没事
	TIM_ICInitTypeDef TIM_ICInitStructure;							//定义结构体变量
	TIM_ICStructInit(&TIM_ICInitStructure);							//结构体初始化，若结构体没有完整赋值
																	//则最好执行此函数，给结构体所有成员都赋一个默认值
																	//避免结构体初值不确定的问题
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;				      //选择配置定时器通道1
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	          //极性：选择为上升沿触发捕获。                    注意：边沿检测参数无效，不影响霍尔传感器接口的TI1F_ED。
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_TRC;            //输入信号：选择TRC。TI1的跳变沿TI1F_ED会接通TRC。
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;				  //捕获预分频：选择不分频，每次信号都触发捕获
	TIM_ICInitStructure.TIM_ICFilter = 0xF;							      //输入滤波器参数：可以过滤信号抖动。              注意：输入滤波参数无效，不影响霍尔传感器接口的TI1F_ED。
	TIM_ICInit(TIM2, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM2的输入捕获通道
    // 三个输入通过异或门，只反映在输入捕获通道1上。
	
	
	/*TIMx霍尔传感器接口配置：使能*/
    TIM_SelectHallSensor(TIM2, ENABLE);        // 选择TIMx_CH1、CH2、CH3引脚异或的结果作为通道1的触发输入TI1
	 
	 
	/*TIM2：选择从模式、TRGI输入的触发源*/
	TIM_SelectInputTrigger(TIM2, TIM_TS_TI1F_ED);				// TRGI输入的触发源：选择TI1F_ED	
	TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);		        // 从模式：选择“复位模式Reset”。即TI1产生边沿跳变TI1F_ED时，会触发CNT复位清零。													
		
		
	// 我们如何确定转速？从《STM32参考手册13.3.18》的图93（或者《GD32参考手册16.1.4.14》的图16-21）可知，
	// 对于一个A相通道，其输入周期的1/6为CCR的锁存值。
	// 我们读出来CCR的锁存值，除以CNT计数频率CK_CNT，得到一个时间值；然后，乘以6，就可以得到A相的周期时间；取倒数，就是频率。频率可以换算为转速。







    /* ------------------------如果需要接口定时器TIM2（主模式），通过驱动高级定时器TIM1（从模式）控制BLDC电机，需要进行以下设置 -------------------------------- */

    /*
	// 如果想要TTM2驱动高级定时器TIMl来控制BLDC电机，那么就需设置通道2为PWM输出模式2，从而使得OC2Ref有一个脉冲通过TRGO输出。
	// 这里我们只是输入测量，不涉及这个，有没有无所谓。
	
	// 输出比较初始化
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);							//结构体初始化，若结构体没有完整赋值
																	//则最好执行此函数，给结构体所有成员都赋一个默认值
																	//避免结构体初值不确定的问题
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;				    // 输出比较模式：选择PWM模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	    // 通道输出状态：输出使能	
	//TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;	// 互补通道输出状态：输出使能。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_Pulse = 1;									// CCR值寄存器值：不能超过（ARR+1）的值。且在0x0000~0xFFFF之间，即0~65535
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		    // 通道输出极性：选择为高，若选择极性为低，则输出高低电平取反
	//TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;	    // 互补通道输出极性：同上。只有高级定时器需要配置
	//TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;      // 空闲状态下通道输出：低电平。只有高级定时器需要配置
    //TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;    // 空闲状态下互补通道输出：低电平。只有高级定时器需要配置
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);						//将结构体变量交给TIM_OC1Init，配置TIM2的输出比较通道2
         // PWM模式2：当CNT < CCR时，低电平；当CNT > CCR时，高电平。与PWM模式1极性相反。
		 // 这样，通过设置CCR2，可以控制TRGO的上升沿何时到来，从而驱动高级定时器TIM1进行输出，控制TIM1输出的PWM波的启动时间。
		 // 为了保证在换相前（异或门有上升沿）可以触发TRGO，尽量减小CCR的值。
		 // 这里CCR = 1，即只要CNT有计数，就可以通过OC2Ref触发TRGO，从而驱动TIM1，控制BLDC电机。


    // 还需要配置通道2的中断
	// 中断输出配置
	TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);					    //使能TIM2_CH2的更新中断
	
	// NVIC中断分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	// NVIC配置
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				//选择配置NVIC的TIM2线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
    // 处理OC2Ref上升沿的中断子程序，需要进行一些处理，才能正确驱动TIM1，从而控制电机。
	// 见《STM32参考手册13.3.18》


    // 使能TIMx(x=1,8)的主输出
	// TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要执行此函数，将所有的输出通道使能或者失能
    // TIM2不需要执行此函数


    // TIM2：主/从模式使能、TRGO输出的触发模式
	TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable); // 通过TRGO输出脉冲，可以驱动TIM1，从而使TIM1的6个互补通道输出，控制BLDC电机
    TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_OC2Ref);        // OC2Ref用于TRGO，这样可以通过TRGO来控制其他外设和定时器。

    */
    /* -------------------------------------------------------------------------------------------------------------------------------------------------------- */



	/*TIM使能*/
	TIM_Cmd(TIM2, ENABLE);			//使能TIM2，定时器开始运行
}

/**
  * 函    数：获取Hall传感器驱动的CCR锁存值
  * 参    数：无
  * 返 回 值：CCR锁存值
  */
uint16_t Hall_Get(void)
{
	// 我们如何确定转速？从《STM32参考手册13.3.18》的图93（或者《GD32参考手册16.1.4.14》的图16-21）可知，
	// 对于一个A相通道，其输入周期的1/6为CCR的锁存值。
	// 我们读出来CCR的锁存值，除以CNT计数频率CK_CNT，得到一个时间值；然后，乘以6，就可以得到A相的周期时间；取倒数，就是频率。频率可以换算为转速。

	/*返回CCR锁存的CNT值*/
	uint16_t Temp;
	Temp = TIM_GetCapture1(TIM2);  // 返回uint16_t类型	                                  
	// TIM_SetCounter(TIM2, 0);    // 不能手动清零CNT，必须是等TI1出现跳变沿TI1F_ED，自动将CNT清零。
	
	return Temp;
}

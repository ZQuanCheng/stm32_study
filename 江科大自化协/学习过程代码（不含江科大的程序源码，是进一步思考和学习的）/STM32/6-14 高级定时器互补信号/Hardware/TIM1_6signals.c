#include "stm32f10x.h"                  // Device header

/**
  * 函    数：TIM1，3通道互补输出初始化
  * 参    数：无
  * 返 回 值：无
  */
void TIM1_6signals_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//开启GPIOB的时钟	
		
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							                 //将PA8、PA9、PA10引脚初始化为复用推挽输出	
	// 结构体已经写入到GPIOA的硬件寄存器了。接下来，可以换个参数，还可以继续使用该结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);							                 //将PB13、PB14、PB15引脚初始化为复用推挽输出	
			

/* -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	生成3个具有3种不同占空比的互补PWM信号。
	TIM1时钟（TIM1CLK）固定为系统核心时钟，TIM1的预分频器等于71，因此使用的TIM1计数器时钟为1MHz。
		 
	这三个占空比的计算如下所示：
	通道1的占空比设置为25%，因此通道1N的占空比为75%。
	通道2的占空比设置为50%，因此通道2N的占空比为50%。	
	通道3的占空比设置为75%，因此通道3N的占空比为25%。
	 ------------------------------------------------------------------------------------- */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (1000-1+1) = 1kHz，即计数周期为1ms。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;				//计数周期，即ARR的值。最大65535U。
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;				//预分频器，即PSC的值。最大65535U。计数时钟频率CK_CNT为72MHz/(72-1+1)=1MHz。
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM1的时基单元
	// TIM_TimeBaseInit函数末尾，手动产生了更新事件：需要清除定时器更新标志位。若不清除此标志位，则开启中断后，会立刻进入一次中断
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update); 		
	// 单独清除一个通道的中断标志位
	//TIM_ClearITPendingBit(TIM1, TIM_IT_CC1); 			
		
		
	/*输出比较初始化*/
	TIM_OCInitTypeDef TIM_OCInitStructure;							//定义结构体变量
	TIM_OCStructInit(&TIM_OCInitStructure);							//结构体初始化，若结构体没有完整赋值
																	//则最好执行此函数，给结构体所有成员都赋一个默认值
																	//避免结构体初值不确定的问题
	// PWM模式1：当CNT < CCR时，高电平；CNT > CCR时，低电平。与PWM模式2极性相反。
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;				    // 输出比较模式，选择PWM模式1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	  	// 通道输出状态：输出使能
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;	    // 互补通道输出状态：输出使能。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		 	// 通道输出极性，选择为高，若选择极性为低，则输出高低电平取反。
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;	    	// 互补通道输出极性，同上。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;   	    // 空闲状态下通道输出：低电平。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;  	// 空闲状态下互补通道输出：同上。只有高级定时器需要配置	
	// 配置通道1
	TIM_OCInitStructure.TIM_Pulse = 250;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 250 / (1000-1+1) = 25%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	// 配置通道2
	TIM_OCInitStructure.TIM_Pulse = 500;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 500 / (1000-1+1) = 50%
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC2Init，配置TIM1的输出比较通道2
	// 配置通道3
	TIM_OCInitStructure.TIM_Pulse = 750;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 750 / (1000-1+1) = 75%
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC3Init，配置TIM1的输出比较通道3
			
		
    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
			
	
	/*CCR预装功能*/
	// 用于配置CCR捕获/比较寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是CHxAVL寄存器，代码为timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// STM32中是下列代码
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在CCR1上的捕获/比较寄存器
	                                                    // 仅用于输出比较
	// 当输出PWM波控制电机时，通过反馈值和PID算法，得到下个周期的控制参数。那么我们就Disable，等更新事件后再写入寄存器。
	// 当输出给ADC采样的时候，一个周期内要多次采样。这次采完，下次采样值就会更高或更低，这时候要Enable，使得修改值立即写入。
	
		
	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		
		
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}

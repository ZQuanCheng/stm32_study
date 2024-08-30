#include "stm32f10x.h"                  // Device header

/**
  * 函    数：TIM1，带死区插入的互补输出
  * 参    数：无
  * 返 回 值：无
  */
void TIM1_deadtime_break_0_Init(void)
{
	// 高级定时器TIM1的3个互补的信号，从3个通道-6个引脚中输出。
	// TIM1_CH1  - PA8、    CH2  - PA9、    CH3  - PA10
	// TIM1_CH1N - PB13、   CH2N - PB14、   CH3N - PB15	
	// 此外，还有TIM1_CH4-PA11、TIM1_ETR-PA12、TIM1_BKIN-PB12（死区插入会用到）

	
	// 本程序演示死区生成，只需要一对互补CH1和CH1N，以及一个死区插入BRKIN 
	
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//开启GPIOB的时钟	
		
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                 // 如果是所有通道，这里需要GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				      // 将CH1-PA8引脚初始化为复用推挽输出	
	// 结构体已经写入到GPIOA的硬件寄存器了。接下来，可以换个参数，还可以继续使用该结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;  // 如果是所有通道，这里需要GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					  // 将BKIN-PB12、CH1N-PB13引脚初始化为复用推挽输出	
			

/* -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	生成1个互补的PWM信号。
	TIM1时钟（TIM1CLK）固定为系统核心时钟，TIM1的预分频器等于71，因此使用的TIM1计数器时钟为1MHz。
		 
	占空比的计算如下所示：
	通道1的占空比设置为50%，因此通道1N的占空比为50%。
    
	插入一个死区时间等于 ((32+6)*16*4)/systemcoreclock系统核心时钟 = 33.8us微秒
		>>> 首先，根据刹车和死区寄存器(TIMx_BDTR)的UTG[7:0]
		>>>>>> 刹车和死区结构体参数-死区时间TIM_DeadTime = 0xE6(11100110)，对应UTG[7:5]=111。有DT = (32+DTG[4:0]) * Tdtg，Tdtg = 16 * tDTS。
	
		>>> 又有，根据控制寄存器(TIMx_CR1)的CKD[1:0]
		>>>>>> 时基单元结构体参数-时钟分频因子TIM_ClockDivision = TIM_CKD_DIV4，对应CKD[1:0]=10。有tDTS = 4 * tCK_INT = 4 / fCK_INT。
		
		>>> 最终，DT = (32+DTG[4:0]) * 16 * 4 / fCK_INT = (32+6)*64 = 2432 / 72000000 = 0.0000338s = 0.0338ms = 33.8us
		>>>>>> 死区时间为33.8us微秒
	
    此外，刹车和死区结构体的其他参数配置：死区特性高电平有效；自动输出使能；使能锁定级别2。
										（配置死区特性，低电平有效（与地线相连））
   ------------------------------------------------------------------------------------- */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV4;     //时钟分频，影响tDTS，进而影响死区时间DT。
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
	TIM_OCInitStructure.TIM_Pulse = 500;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 500 / (1000-1+1) = 50%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	
	
	/*输出比较：CCR预装功能*/
	// 用于配置CCR捕获/比较寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是CHxAVL寄存器，代码为timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// STM32中是下列代码
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在CCR1上的捕获/比较寄存器
	                                                    // 仅用于输出比较
	// 当输出PWM波控制电机时，通过反馈值和PID算法，得到下个周期的控制参数。那么我们就Disable，等更新事件后再写入寄存器。
	// 当输出给ADC采样的时候，一个周期内要多次采样。这次采完，下次采样值就会更高或更低，这时候要Enable，使得修改值立即写入。
	

    /*死区初始化*/
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;          //刹车和死区结构体
	TIM_BDTRStructInit(&TIM_BDTRInitStructure);         //结构体初始化，若结构体没有完整赋值
														//则最好执行此函数，给结构体所有成员都赋一个默认值
														//避免结构体初值不确定的问题
    // 由于后续的TIM_CtrlPWMOutputs(TIM1, ENABLE);会将TIM1_BDTR寄存器的MOE位置1，
	// 所以这里只有runoffstate参数配置有效，ideloffstate参数配置无效。
	// 《STM32参考手册》没有整理好的表格，我们可以看《GD32参考手册16.1.4-死区时间插入 表16-2》：MOE对应POEN，OSSR对应ROS，OSSI对应IOS。
	// 可知，若MOE=0，则OSSR=0/1无所谓，只看OSSI；若MOE=1，则OSSI=0/1无所谓，只看OSSR。
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;               // 运行模式下“关闭状态”选择。MOE置1时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;               // 空闲模式下“关闭状态”选择。MOE置0时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_2;                     // 锁定设置。默认是TIM_LOCKLevel_OFF无写保护，这里修改位锁定级别2。
	TIM_BDTRInitStructure.TIM_DeadTime = 0xE6;                                 // 死区时间0x00~0xFF，即0~255。默认是0x00，这里修改为0xE6。
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;                        // 中止使能。默认是Disable失能，这里需要修改为Enable使能。
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;          // 中止信号极性。默认是Low低电平有效，这里需要修改为High高电平有效。
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;    // 自动输出使能。默认是Disable，这里需要修改为Enable使能。
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);  // 将结构体变量交给刹车和死区功能配置函数，初始化TIM1。


    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
		

	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		
		
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}



void TIM1_deadtime_break_1_Init(void)
{
	// 高级定时器TIM1的3个互补的信号，从3个通道-6个引脚中输出。
	// TIM1_CH1  - PA8、    CH2  - PA9、    CH3  - PA10
	// TIM1_CH1N - PB13、   CH2N - PB14、   CH3N - PB15	
	// 此外，还有TIM1_CH4-PA11、TIM1_ETR-PA12、TIM1_BKIN-PB12（死区插入会用到）

	
	// 本程序演示死区生成，只需要一对互补CH1和CH1N，以及一个死区插入BRKIN 
	
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//开启GPIOB的时钟	
		
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                 // 如果是所有通道，这里需要GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				      // 将CH1-PA8引脚初始化为复用推挽输出	
	// 结构体已经写入到GPIOA的硬件寄存器了。接下来，可以换个参数，还可以继续使用该结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;  // 如果是所有通道，这里需要GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					  // 将BKIN-PB12、CH1N-PB13引脚初始化为复用推挽输出	
			

/* -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	生成1个互补的PWM信号。
	
	TIM1时钟（TIM1CLK）固定为系统核心时钟，TIM1的预分频器等于71，因此使用的TIM1计数器时钟为1MHz。
    定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	
	占空比的计算如下所示：
	通道1的占空比设置为60%，因此通道1N的占空比为40%。	
    
	插入一个死区时间等于 ((32+6)*16*1)/systemcoreclock系统核心时钟 = 8.4us微秒
		>>> 首先，根据刹车和死区寄存器(TIMx_BDTR)的UTG[7:0]
		>>>>>> 刹车和死区结构体参数-死区时间TIM_DeadTime = 0xE6(11100110)，对应UTG[7:5]=111。有DT = (32+DTG[4:0]) * Tdtg，Tdtg = 16 * tDTS。
	
		>>> 又有，根据控制寄存器(TIMx_CR1)的CKD[1:0]
		>>>>>> 时基单元结构体参数-时钟分频因子TIM_ClockDivision = TIM_CKD_DIV1，对应CKD[1:0]=00。有tDTS = 1 * tCK_INT = 1 / fCK_INT。
		
		>>> 最终，DT = (32+DTG[4:0]) * 16 * 1 / fCK_INT = (32+6)*16 = 608 / 72000000 = 0.0000084s = 0.0084ms = 8.4us
		>>>>>> 死区时间为8.4us微秒
	
    此外，刹车和死区结构体的其他参数配置：死区特性高电平有效；自动输出使能；使能锁定级别2。
										（配置死区特性，低电平有效（与地线相连））

   CH1的正脉冲宽度24us > CH1N负脉冲宽度16us > 死区时间8.4us
   ------------------------------------------------------------------------------------- */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，影响tDTS，进而影响死区时间DT。
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 40 - 1;				    //计数周期，即ARR的值。最大65535U。
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
	TIM_OCInitStructure.TIM_Pulse = 24;				    // CCR寄存器值。占空比duty = CCR / (ARR+1) = 24 / (40-1+1) = 60%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	
	
	/*输出比较：CCR预装功能*/
	// 用于配置CCR捕获/比较寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是CHxAVL寄存器，代码为timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// STM32中是下列代码
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在CCR1上的捕获/比较寄存器
	                                                    // 仅用于输出比较
	// 当输出PWM波控制电机时，通过反馈值和PID算法，得到下个周期的控制参数。那么我们就Disable，等更新事件后再写入寄存器。
	// 当输出给ADC采样的时候，一个周期内要多次采样。这次采完，下次采样值就会更高或更低，这时候要Enable，使得修改值立即写入。
	

    /*死区初始化*/
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;          //刹车和死区结构体
	TIM_BDTRStructInit(&TIM_BDTRInitStructure);         //结构体初始化，若结构体没有完整赋值
														//则最好执行此函数，给结构体所有成员都赋一个默认值
														//避免结构体初值不确定的问题
    // 由于后续的TIM_CtrlPWMOutputs(TIM1, ENABLE);会将TIM1_BDTR寄存器的MOE位置1，
	// 所以这里只有runoffstate参数配置有效，ideloffstate参数配置无效。
	// 《STM32参考手册》没有整理好的表格，我们可以看《GD32参考手册16.1.4-死区时间插入 表16-2》：MOE对应POEN，OSSR对应ROS，OSSI对应IOS。
	// 可知，若MOE=0，则OSSR=0/1无所谓，只看OSSI；若MOE=1，则OSSI=0/1无所谓，只看OSSR。
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;               // 运行模式下“关闭状态”选择。MOE置1时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;               // 空闲模式下“关闭状态”选择。MOE置0时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_2;                     // 锁定设置。默认是TIM_LOCKLevel_OFF无写保护，这里修改位锁定级别2。
	TIM_BDTRInitStructure.TIM_DeadTime = 0xE6;                                 // 死区时间0x00~0xFF，即0~255。默认是0x00，这里修改为0xE6。
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;                        // 中止使能。默认是Disable失能，这里需要修改为Enable使能。
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;          // 中止信号极性。默认是Low低电平有效，这里需要修改为High高电平有效。
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;    // 自动输出使能。默认是Disable，这里需要修改为Enable使能。
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);  // 将结构体变量交给刹车和死区功能配置函数，初始化TIM1。


    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
		

	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		
		
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}





void TIM1_deadtime_break_2_Init(void)
{
	// 高级定时器TIM1的3个互补的信号，从3个通道-6个引脚中输出。
	// TIM1_CH1  - PA8、    CH2  - PA9、    CH3  - PA10
	// TIM1_CH1N - PB13、   CH2N - PB14、   CH3N - PB15	
	// 此外，还有TIM1_CH4-PA11、TIM1_ETR-PA12、TIM1_BKIN-PB12（死区插入会用到）

	
	// 本程序演示死区生成，只需要一对互补CH1和CH1N，以及一个死区插入BRKIN 
	
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//开启GPIOB的时钟	
		
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                 // 如果是所有通道，这里需要GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				      // 将CH1-PA8引脚初始化为复用推挽输出	
	// 结构体已经写入到GPIOA的硬件寄存器了。接下来，可以换个参数，还可以继续使用该结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;  // 如果是所有通道，这里需要GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					  // 将BKIN-PB12、CH1N-PB13引脚初始化为复用推挽输出	
			

/* -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	生成1个互补的PWM信号。
	
	TIM1时钟（TIM1CLK）固定为系统核心时钟，TIM1的预分频器等于71，因此使用的TIM1计数器时钟为1MHz。
    定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	
	占空比的计算如下所示：
	通道1的占空比设置为60%，因此通道1N的占空比为40%。	
    
	插入一个死区时间等于 ((32+6)*16*2)/systemcoreclock系统核心时钟 = 16.9us微秒
		>>> 首先，根据刹车和死区寄存器(TIMx_BDTR)的UTG[7:0]
		>>>>>> 刹车和死区结构体参数-死区时间TIM_DeadTime = 0xE6(11100110)，对应UTG[7:5]=111。有DT = (32+DTG[4:0]) * Tdtg，Tdtg = 16 * tDTS。
	
		>>> 又有，根据控制寄存器(TIMx_CR1)的CKD[1:0]
		>>>>>> 时基单元结构体参数-时钟分频因子TIM_ClockDivision = TIM_CKD_DIV2，对应CKD[1:0]=01。有tDTS = 2 * tCK_INT = 2 / fCK_INT。
		
		>>> 最终，DT = (32+DTG[4:0]) * 16 * 2 / fCK_INT = (32+6)*32 = 1216 / 72000000 = 0.0000169s = 0.0169ms = 16.9us
		>>>>>> 死区时间为16.9us微秒
	
    此外，刹车和死区结构体的其他参数配置：死区特性高电平有效；自动输出使能；使能锁定级别2。
										（配置死区特性，低电平有效（与地线相连））

   CH1的正脉冲宽度24us > 死区时间16.9us > CH1N负脉冲宽度16us
   ------------------------------------------------------------------------------------- */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV2;     //时钟分频，影响tDTS，进而影响死区时间DT。
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 40 - 1;				    //计数周期，即ARR的值。最大65535U。
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
	TIM_OCInitStructure.TIM_Pulse = 24;				    // CCR寄存器值。占空比duty = CCR / (ARR+1) = 24 / (40-1+1) = 60%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	
	
	/*输出比较：CCR预装功能*/
	// 用于配置CCR捕获/比较寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是CHxAVL寄存器，代码为timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// STM32中是下列代码
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在CCR1上的捕获/比较寄存器
	                                                    // 仅用于输出比较
	// 当输出PWM波控制电机时，通过反馈值和PID算法，得到下个周期的控制参数。那么我们就Disable，等更新事件后再写入寄存器。
	// 当输出给ADC采样的时候，一个周期内要多次采样。这次采完，下次采样值就会更高或更低，这时候要Enable，使得修改值立即写入。
	

    /*死区初始化*/
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;          //刹车和死区结构体
	TIM_BDTRStructInit(&TIM_BDTRInitStructure);         //结构体初始化，若结构体没有完整赋值
														//则最好执行此函数，给结构体所有成员都赋一个默认值
														//避免结构体初值不确定的问题
    // 由于后续的TIM_CtrlPWMOutputs(TIM1, ENABLE);会将TIM1_BDTR寄存器的MOE位置1，
	// 所以这里只有runoffstate参数配置有效，ideloffstate参数配置无效。
	// 《STM32参考手册》没有整理好的表格，我们可以看《GD32参考手册16.1.4-死区时间插入 表16-2》：MOE对应POEN，OSSR对应ROS，OSSI对应IOS。
	// 可知，若MOE=0，则OSSR=0/1无所谓，只看OSSI；若MOE=1，则OSSI=0/1无所谓，只看OSSR。
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;               // 运行模式下“关闭状态”选择。MOE置1时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;               // 空闲模式下“关闭状态”选择。MOE置0时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_2;                     // 锁定设置。默认是TIM_LOCKLevel_OFF无写保护，这里修改位锁定级别2。
	TIM_BDTRInitStructure.TIM_DeadTime = 0xE6;                                 // 死区时间0x00~0xFF，即0~255。默认是0x00，这里修改为0xE6。
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;                        // 中止使能。默认是Disable失能，这里需要修改为Enable使能。
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;          // 中止信号极性。默认是Low低电平有效，这里需要修改为High高电平有效。
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;    // 自动输出使能。默认是Disable，这里需要修改为Enable使能。
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);  // 将结构体变量交给刹车和死区功能配置函数，初始化TIM1。


    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
		

	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		
		
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}






void TIM1_deadtime_break_3_Init(void)
{
	// 高级定时器TIM1的3个互补的信号，从3个通道-6个引脚中输出。
	// TIM1_CH1  - PA8、    CH2  - PA9、    CH3  - PA10
	// TIM1_CH1N - PB13、   CH2N - PB14、   CH3N - PB15	
	// 此外，还有TIM1_CH4-PA11、TIM1_ETR-PA12、TIM1_BKIN-PB12（死区插入会用到）

	
	// 本程序演示死区生成，只需要一对互补CH1和CH1N，以及一个死区插入BRKIN 
	
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//开启GPIOB的时钟	
		
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                 // 如果是所有通道，这里需要GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				      // 将CH1-PA8引脚初始化为复用推挽输出	
	// 结构体已经写入到GPIOA的硬件寄存器了。接下来，可以换个参数，还可以继续使用该结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;  // 如果是所有通道，这里需要GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					  // 将BKIN-PB12、CH1N-PB13引脚初始化为复用推挽输出	
			

/* -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	生成1个互补的PWM信号。
	
	TIM1时钟（TIM1CLK）固定为系统核心时钟，TIM1的预分频器等于71，因此使用的TIM1计数器时钟为1MHz。
    定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	
	占空比的计算如下所示：
	通道1的占空比设置为40%，因此通道1N的占空比为60%。	
    
	插入一个死区时间等于 ((32+6)*16*1)/systemcoreclock系统核心时钟 = 8.4us微秒
		>>> 首先，根据刹车和死区寄存器(TIMx_BDTR)的UTG[7:0]
		>>>>>> 刹车和死区结构体参数-死区时间TIM_DeadTime = 0xE6(11100110)，对应UTG[7:5]=111。有DT = (32+DTG[4:0]) * Tdtg，Tdtg = 16 * tDTS。
	
		>>> 又有，根据控制寄存器(TIMx_CR1)的CKD[1:0]
		>>>>>> 时基单元结构体参数-时钟分频因子TIM_ClockDivision = TIM_CKD_DIV1，对应CKD[1:0]=00。有tDTS = 1 * tCK_INT = 1 / fCK_INT。
		
		>>> 最终，DT = (32+DTG[4:0]) * 16 * 1 / fCK_INT = (32+6)*16 = 608 / 72000000 = 0.0000084s = 0.0084ms = 8.4us
		>>>>>> 死区时间为8.4us微秒
	
    此外，刹车和死区结构体的其他参数配置：死区特性高电平有效；自动输出使能；使能锁定级别2。
										（配置死区特性，低电平有效（与地线相连））

    死区时间8.4us < CH1的正脉冲宽度16us < CH1N负脉冲宽度24us
   ------------------------------------------------------------------------------------- */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，影响tDTS，进而影响死区时间DT。
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 40 - 1;				    //计数周期，即ARR的值。最大65535U。
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
	TIM_OCInitStructure.TIM_Pulse = 16;				    // CCR寄存器值。占空比duty = CCR / (ARR+1) = 16 / (40-1+1) = 40%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	
	
	/*输出比较：CCR预装功能*/
	// 用于配置CCR捕获/比较寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是CHxAVL寄存器，代码为timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// STM32中是下列代码
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在CCR1上的捕获/比较寄存器
	                                                    // 仅用于输出比较
	// 当输出PWM波控制电机时，通过反馈值和PID算法，得到下个周期的控制参数。那么我们就Disable，等更新事件后再写入寄存器。
	// 当输出给ADC采样的时候，一个周期内要多次采样。这次采完，下次采样值就会更高或更低，这时候要Enable，使得修改值立即写入。
	

    /*死区初始化*/
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;          //刹车和死区结构体
	TIM_BDTRStructInit(&TIM_BDTRInitStructure);         //结构体初始化，若结构体没有完整赋值
														//则最好执行此函数，给结构体所有成员都赋一个默认值
														//避免结构体初值不确定的问题
    // 由于后续的TIM_CtrlPWMOutputs(TIM1, ENABLE);会将TIM1_BDTR寄存器的MOE位置1，
	// 所以这里只有runoffstate参数配置有效，ideloffstate参数配置无效。
	// 《STM32参考手册》没有整理好的表格，我们可以看《GD32参考手册16.1.4-死区时间插入 表16-2》：MOE对应POEN，OSSR对应ROS，OSSI对应IOS。
	// 可知，若MOE=0，则OSSR=0/1无所谓，只看OSSI；若MOE=1，则OSSI=0/1无所谓，只看OSSR。
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;               // 运行模式下“关闭状态”选择。MOE置1时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;               // 空闲模式下“关闭状态”选择。MOE置0时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_2;                     // 锁定设置。默认是TIM_LOCKLevel_OFF无写保护，这里修改位锁定级别2。
	TIM_BDTRInitStructure.TIM_DeadTime = 0xE6;                                 // 死区时间0x00~0xFF，即0~255。默认是0x00，这里修改为0xE6。
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;                        // 中止使能。默认是Disable失能，这里需要修改为Enable使能。
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;          // 中止信号极性。默认是Low低电平有效，这里需要修改为High高电平有效。
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;    // 自动输出使能。默认是Disable，这里需要修改为Enable使能。
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);  // 将结构体变量交给刹车和死区功能配置函数，初始化TIM1。


    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
		

	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		
		
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}



void TIM1_deadtime_break_4_Init(void)
{
	// 高级定时器TIM1的3个互补的信号，从3个通道-6个引脚中输出。
	// TIM1_CH1  - PA8、    CH2  - PA9、    CH3  - PA10
	// TIM1_CH1N - PB13、   CH2N - PB14、   CH3N - PB15	
	// 此外，还有TIM1_CH4-PA11、TIM1_ETR-PA12、TIM1_BKIN-PB12（死区插入会用到）

	
	// 本程序演示死区生成，只需要一对互补CH1和CH1N，以及一个死区插入BRKIN 
	
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//开启GPIOB的时钟	
		
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;                 // 如果是所有通道，这里需要GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				      // 将CH1-PA8引脚初始化为复用推挽输出	
	// 结构体已经写入到GPIOA的硬件寄存器了。接下来，可以换个参数，还可以继续使用该结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;  // 如果是所有通道，这里需要GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);					  // 将BKIN-PB12、CH1N-PB13引脚初始化为复用推挽输出	
			

/* -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	生成1个互补的PWM信号。
	
	TIM1时钟（TIM1CLK）固定为系统核心时钟，TIM1的预分频器等于71，因此使用的TIM1计数器时钟为1MHz。
    定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	
	占空比的计算如下所示：
	通道1的占空比设置为40%，因此通道1N的占空比为60%。	
    
	插入一个死区时间等于 ((32+6)*16*2)/systemcoreclock系统核心时钟 = 16.9us微秒
		>>> 首先，根据刹车和死区寄存器(TIMx_BDTR)的UTG[7:0]
		>>>>>> 刹车和死区结构体参数-死区时间TIM_DeadTime = 0xE6(11100110)，对应UTG[7:5]=111。有DT = (32+DTG[4:0]) * Tdtg，Tdtg = 16 * tDTS。
	
		>>> 又有，根据控制寄存器(TIMx_CR1)的CKD[1:0]
		>>>>>> 时基单元结构体参数-时钟分频因子TIM_ClockDivision = TIM_CKD_DIV2，对应CKD[1:0]=01。有tDTS = 2 * tCK_INT = 2 / fCK_INT。
		
		>>> 最终，DT = (32+DTG[4:0]) * 16 * 2 / fCK_INT = (32+6)*32 = 1216 / 72000000 = 0.0000169s = 0.0169ms = 16.9us
		>>>>>> 死区时间为16.9us微秒
	
    此外，刹车和死区结构体的其他参数配置：死区特性高电平有效；自动输出使能；使能锁定级别2。
										（配置死区特性，低电平有效（与地线相连））

    CH1的正脉冲宽度16us < 死区时间16.9us < CH1N负脉冲宽度24us
   ------------------------------------------------------------------------------------- */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV2;     //时钟分频，影响tDTS，进而影响死区时间DT。
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (40-1+1) = 25kHz，即计数周期为40us。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 40 - 1;				    //计数周期，即ARR的值。最大65535U。
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
	TIM_OCInitStructure.TIM_Pulse = 16;				    // CCR寄存器值。占空比duty = CCR / (ARR+1) = 16 / (40-1+1) = 40%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
	
	
	/*输出比较：CCR预装功能*/
	// 用于配置CCR捕获/比较寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是CHxAVL寄存器，代码为timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// STM32中是下列代码
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在CCR1上的捕获/比较寄存器
	                                                    // 仅用于输出比较
	// 当输出PWM波控制电机时，通过反馈值和PID算法，得到下个周期的控制参数。那么我们就Disable，等更新事件后再写入寄存器。
	// 当输出给ADC采样的时候，一个周期内要多次采样。这次采完，下次采样值就会更高或更低，这时候要Enable，使得修改值立即写入。
	

    /*死区初始化*/
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;          //刹车和死区结构体
	TIM_BDTRStructInit(&TIM_BDTRInitStructure);         //结构体初始化，若结构体没有完整赋值
														//则最好执行此函数，给结构体所有成员都赋一个默认值
														//避免结构体初值不确定的问题
    // 由于后续的TIM_CtrlPWMOutputs(TIM1, ENABLE);会将TIM1_BDTR寄存器的MOE位置1，
	// 所以这里只有runoffstate参数配置有效，ideloffstate参数配置无效。
	// 《STM32参考手册》没有整理好的表格，我们可以看《GD32参考手册16.1.4-死区时间插入 表16-2》：MOE对应POEN，OSSR对应ROS，OSSI对应IOS。
	// 可知，若MOE=0，则OSSR=0/1无所谓，只看OSSI；若MOE=1，则OSSI=0/1无所谓，只看OSSR。
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;               // 运行模式下“关闭状态”选择。MOE置1时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;               // 空闲模式下“关闭状态”选择。MOE置0时，此参数有效；保持默认值Disable。
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_2;                     // 锁定设置。默认是TIM_LOCKLevel_OFF无写保护，这里修改位锁定级别2。
	TIM_BDTRInitStructure.TIM_DeadTime = 0xE6;                                 // 死区时间0x00~0xFF，即0~255。默认是0x00，这里修改为0xE6。
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;                        // 中止使能。默认是Disable失能，这里需要修改为Enable使能。
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;          // 中止信号极性。默认是Low低电平有效，这里需要修改为High高电平有效。
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;    // 自动输出使能。默认是Disable，这里需要修改为Enable使能。
    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);  // 将结构体变量交给刹车和死区功能配置函数，初始化TIM1。


    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
		

	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		
		
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}

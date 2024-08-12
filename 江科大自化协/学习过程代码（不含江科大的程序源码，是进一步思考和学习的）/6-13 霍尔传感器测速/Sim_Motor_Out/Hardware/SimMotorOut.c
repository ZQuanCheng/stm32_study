#include "stm32f10x.h"                  // Device header

extern uint32_t gSimStep;	

/**
  * 函    数：模拟霍尔输入信号，及模拟BLDC电机上的霍尔传感器的ABC三相波形
  * 参    数：无
  * 返 回 值：无
  */
void Sim_Motor_Output_Init(void)
{
	// 这里只是用作普通GPIO口，不受片上外设TIM2的输出通道TIMx_CH1、TIMNx_ CH2、TIMx_CH3控制
	// TIM2这里只是用于定时中断，固定间隔换相

	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		    //开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);						        //将PA0、PA1、PA2引脚初始化为推挽输出	
	
	
	/*复位外设TIM2*/
    TIM_DeInit(TIM2);		
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM2);		//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，选择向上计数
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PCS / (PSC+1) / (ARR+1) = 72MHz / (12-1+1) / (1000-1+1) = 6kHz。
	// 后续执行Sim_Motor_Prepare_Comutation()，即霍尔传感器的ABC相的周期为定时周期的6倍，频率为定时颜率的1/6，即1kHz。
	// 由于我们是用GPIO口模拟霍尔传感器的ABC相信号，会有误差，不一定定全准确1000Hz。
    // 就算是实际运行的BLDC上的霍尔传感器的ABC相信号，也会有存在换相时间偏差。
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;				//计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 12 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			//重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元
	// TIM_TimeBaseInit函数末尾，手动产生了更新事件；需要清除定时器更新标志位。若不清除此标志位，则开启中断后，会立刻进入一次中断。
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	// 如果需要单独清除一个通道的中断标志位，可以这样：
	// TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);	
	
	
	/*中断输出配置*/	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);					//开启TIM2的更新中断
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				//选择配置NVIC的TIM2线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设


    /*自动预装功能：使能*/
	// TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);  // 使能或者失能TIMx在CCR1上的预装载寄存器
	                                                      // 仅用于输出比较
														  
	/*TIM使能*/
	TIM_Cmd(TIM2, ENABLE);			//使能TIM2，定时器开始运行
}

/* 定时器中断函数，可以复制到使用它的地方
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
*/

/**
  * 函    数：CH1开启
  * 参    数：无
  * 返 回 值：无
  */
void CH1_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_0);		//设置PA0引脚为高电平
}

/**
  * 函    数：CH1关闭
  * 参    数：无
  * 返 回 值：无
  */
void CH1_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);		//设置PA0引脚为低电平
}

/**
  * 函    数：CH2开启
  * 参    数：无
  * 返 回 值：无
  */
void CH2_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_1);		//设置PA1引脚为高电平
}

/**
  * 函    数：CH2关闭
  * 参    数：无
  * 返 回 值：无
  */
void CH2_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);		//设置PA1引脚为低电平
}

/**
  * 函    数：CH3开启
  * 参    数：无
  * 返 回 值：无
  */
void CH3_ON(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_2);		//设置PA2引脚为高电平
}

/**
  * 函    数：CH3关闭
  * 参    数：无
  * 返 回 值：无
  */
void CH3_OFF(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);		//设置PA2引脚为低电平
}

/**
  * 函    数：模拟电机换相
  * 参    数：无
  * 返 回 值：无
  */
void Sim_Motor_Prepare_Comutation(void)
{
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PCS / (PSC+1) / (ARR+1) = 72MHz / (12-1+1) / (1000-1+1) = 6kHz。
	// 后续执行Sim_Motor_Prepare_Comutation()，即霍尔传感器的ABC相的周期为定时周期的6倍，频率为定时颜率的1/6，即1kHz。
    if (gSimStep == 1){
        // Step 1 Configuration
        CH1_ON();
        CH2_OFF();
        CH3_OFF();
        gSimStep++;
    }
    else if (gSimStep == 2){
        // Step 2 Configuration
        CH1_ON();
        CH2_ON();
        CH3_OFF();
        gSimStep++;
    }
    else if (gSimStep == 3){
        // Step 3 Configuration
        CH1_ON();
        CH2_ON();
        CH3_ON();
        gSimStep++;
    }
    else if (gSimStep == 4){
        // Step 4 Configuration
        CH1_OFF();
        CH2_ON();
        CH3_ON();
        gSimStep++;
    }
    else if (gSimStep == 5){
        // Step 5 Configuration
        CH1_OFF();
        CH2_OFF();
        CH3_ON();
        gSimStep++;
    }
    else{
        // Step 6 Configuration      
        CH1_OFF();
        CH2_OFF();
        CH3_OFF();  
        gSimStep = 1;
    }   
}

#include "stm32f10x.h"                  // Device header

/**
  * 函    数：TIM3_TRGO初始化
  * 参    数：无
  * 返 回 值：无
  */
void TIM3_TRGO_Init(void)
{
	/*
       这里，TIM3仅用于产生更新事件（寄存器ADC_CR2的位域EXTSEL[2:0]=100，即定时器3的TRGO事件）
	   由于只有计数器更新（CNT的值自增到和ARR的值相同时）才触发一次ADC转换。
	   因此ADC转换的触发频率，和TIM3的更新频率一致，都是1Hz，即周期为1s。		
	*/
	
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);			//开启TIM3的时钟																	
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM3);		//选择TIM3为内部时钟，若不调用此函数，TIM3默认也为内部时钟
	
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，影响tDTS，进而影响死区时间DT。
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (7200-1+1) / (10000-1+1) = 1Hz，即计数周期为1s。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;				//计数周期，即ARR的值。最大65535U。
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;				//预分频器，即PSC的值。最大65535U。计数时钟频率CK_CNT为72MHz/(72-1+1)=1MHz。
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM1的时基单元
	// TIM_TimeBaseInit函数末尾，手动产生了更新事件：需要清除定时器更新标志位。若不清除此标志位，则开启中断后，会立刻进入一次中断
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update); 		
	// 单独清除一个通道的中断标志位
	//TIM_ClearITPendingBit(TIM3, TIM_IT_CC1); 				
	

	/*TIM3：选择从模式、TRGI输入的触发源*/
	// TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Trigger);		     // 从模式：选择“触发模式”															
	// TIM_SelectInputTrigger(TIM3, TIM_TS_ITR2);					 // TRGI输入的触发源：选择ITR2	

    /*TIM3：主/从模式使能、TRGO输出的触发模式*/
	TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable); // 主从/模式：使能
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);        // TRGO输出的触发模式：更新事件。换成TIM_TRGOSource_OC1Ref是否可以


	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM3, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
	

	/*TIM使能*/
	TIM_Cmd(TIM3, ENABLE);			//使能TIM3，定时器开始运行
}

#include "stm32f10x.h"                  // Device header
#include "LED.h"

uint16_t ADC1_Value[4];					//定义用于存放ADC1转换结果的全局数组
uint16_t ADC2_Value[4];					//定义用于存放ADC2转换结果的全局数组

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  */
void AD_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);	//开启ADC2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	// RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);						//选择时钟6分频，ADCCLK = 72MHz / 8 = 9MHz，一个ADCCLK周期为1/9us
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				        //将PA0~PA3引脚初始化为模拟输入	
	
	
	// ----------------------------------------------- 双ADC、规则组配置 ------------------------------------------------------------------------
		
	/*
	  结构体ADC_InitTypeDef中的成员ADC_NbrOfChannel和ADC_ExternalTrigConv，都是针对规则组的，无法影响注入组
	  	结构体成员ADC_NbrOfChannel：设置规则组（扫描模式）的前x个通道
	  	结构体成员ADC_ExternalTrigConv：设置规则组的外部触发源（包括设置软件触发，即ADC_ExternalTrigConv_None）

	  注入组的转换序列长度、外部触发源（包括软件触发）需要通过库函数设置
	  	如果注入组需要设置转换序列长度，就得使用ADC_InjectedSequencerLengthConfig()
	  	如果注入组需要配置外部触发源，就得使用ADC_ExternalTrigInjectedConvConfig()（包括设置软件触发，即参数ADC_ExternalTrigInjecConv_None）
	*/	
	
	/*定义ADC结构体变量*/	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_InjecSimult;							//模式，选择同步注入，即使用ADC1的注入组、ADC2的注入组
	
	/*ADC1初始化*/
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据右对齐
	//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;		//（只针对规则组）外部触发源		
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//单次转换
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式
	//ADC_InitStructure.ADC_NbrOfChannel = 4;									//（只针对规则组）通道数
	ADC_Init(ADC1, &ADC_InitStructure);		
	
	ADC_InjectedSequencerLengthConfig(ADC1, 4); 							    //（只针对注入组）通道数，为4
	
	ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T2_CC1); //（只针对注入组）外部触发源，使用TIM2的CC1
	
	/*ADC2初始化*/
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据右对齐
	//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;		//（只针对规则组）外部触发源		
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//单次转换
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式
	//ADC_InitStructure.ADC_NbrOfChannel = 4;									//（只针对规则组）通道数
	ADC_Init(ADC2, &ADC_InitStructure);		
	
	ADC_InjectedSequencerLengthConfig(ADC2, 4); 							    //（只针对注入组）通道数，为4
	
	ADC_ExternalTrigInjectedConvConfig(ADC2, ADC_ExternalTrigInjecConv_None);   //（只针对注入组）外部触发源，无
		
	
	
	/*ADC注入组通道配置*/
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);	//ADC1-注入组序列1的位置，配置为通道ADC1_IN0，对应PA0
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);	//ADC1-注入组序列2的位置，配置为通道ADC1_IN1，对应PA1
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_55Cycles5);	//ADC1-注入组序列3的位置，配置为通道ADC1_IN2，对应PA2
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5);	//ADC1-注入组序列4的位置，配置为通道ADC1_IN3，对应PA3

	ADC_InjectedChannelConfig(ADC2, ADC_Channel_3, 1, ADC_SampleTime_55Cycles5);	//ADC2-注入组序列1的位置，配置为通道ADC1_IN3，对应PA3
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_2, 2, ADC_SampleTime_55Cycles5);	//ADC2-注入组序列2的位置，配置为通道ADC1_IN2，对应PA2
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_1, 3, ADC_SampleTime_55Cycles5);	//ADC2-注入组序列3的位置，配置为通道ADC1_IN1，对应PA1
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_0, 4, ADC_SampleTime_55Cycles5);	//ADC2-注入组序列4的位置，配置为通道ADC1_IN0，对应PA0

	
	/*ADC注入组的外部触发*/	
	ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);  //ADC1-（注入组）外部触发使能
	ADC_ExternalTrigInjectedConvCmd(ADC2, ENABLE);  //ADC2-（注入组）外部触发使能		
	
	
	
	
	// -------------------------------------------- ADC的中断配置 -------------------------------------------------------
	
	/*清除ADC的中断标志位*/
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	
	ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);	             
	
	/*开启ADC的中断*/	
    //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);  // 使能ADC的EOC中断 （规则组或注入组 A/D转换结束）	
    ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);   // 使能ADC的JEOC中断（        注入组 A/D转换结束）


	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;			//选择配置NVIC的ADC1_2线，ADC1和ADC2全局中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	// -------------------------------------------------------------------------------------------------------------------------




	/*ADC1使能*/
	ADC_Cmd(ADC1, ENABLE);									//使能ADC1，ADC开始运行

	/*ADC1校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
	/*ADC2使能*/
	ADC_Cmd(ADC2, ENABLE);									//使能ADC2，ADC开始运行

	/*ADC2校准*/
	ADC_ResetCalibration(ADC2);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC2) == SET);
	ADC_StartCalibration(ADC2);
	while (ADC_GetCalibrationStatus(ADC2) == SET);	
}

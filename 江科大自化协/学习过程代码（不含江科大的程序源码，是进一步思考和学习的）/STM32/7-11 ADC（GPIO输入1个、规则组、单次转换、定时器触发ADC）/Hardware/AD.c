#include "stm32f10x.h"                  // Device header
#include "LED.h"

uint16_t AD_Value;					//定义用于存放AD转换结果的全局变量

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  */
void AD_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);						//选择时钟6分频，ADCCLK = 72MHz / 8 = 9MHz，一个ADCCLK周期为1/9us
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				        //将PA1引脚初始化为模拟输入	
	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	
	//要配合Main.c中调用TIM3_TRGO_Init();还是TIM1_CC1_Init();来选择
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;		//（只针对规则组）外部触发源，使用TIM3的TRGO
	// ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;	//（只针对规则组）外部触发源，使用TIM1的CC1		
	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//连续转换，失能，每转换一次规则组序列后停止
	
	//这里，由于只有一个ADC输入，是否扫描模式，没区别；
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式，使能，扫描规则组的序列，扫描数量由ADC_NbrOfChannel确定
	// ADC_InitStructure.ADC_ScanConvMode = DISABLE;							//扫描模式，失能，只转换规则组的序列1这一个位置

    ADC_InitStructure.ADC_NbrOfChannel = 1;									    //（只针对规则组）通道数，为1，扫描规则组的前1个通道
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1
	/*
	  结构体ADC_InitTypeDef中的成员ADC_NbrOfChannel和ADC_ExternalTrigConv，都是针对规则组的，无法影响注入组
	  	结构体成员ADC_NbrOfChannel：设置规则组（扫描模式）的前x个通道
	  	结构体成员ADC_ExternalTrigConv：设置规则组的外部触发源（包括设置软件触发，即ADC_ExternalTrigConv_None）

	  注入组的转换序列长度、外部触发源（包括软件触发）需要通过库函数设置
	  	如果注入组需要设置转换序列长度，就得使用ADC_InjectedSequencerLengthConfig()
	  	如果注入组需要配置外部触发源，就得使用ADC_ExternalTrigInjectedConvConfig()（包括设置软件触发，即参数ADC_ExternalTrigInjecConv_None）
	*/
	
	
	/*ADC规则组的外部触发源
	
	  如果用TIM3_TRGO，需要配置主模式输出TRGO，但是不需要配置输出比较OC结构体；
	  需要在AD.c中设置结构体的成员 ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;		//（只针对规则组）外部触发源，使用TIM3的TRGO	
	
	  如果用TIM1_CC1，需要配置输出比较OC结构体，但是不需要配置主模式输出TRGO；
	  需要在AD.c中设置结构体的成员 ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;		//（只针对规则组）外部触发源，使用TIM1的CC1	
    */
	
	
	/*ADC规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);	//规则组序列1的位置，配置为通道ADC1_IN1，对应PA1
	/*
	  ADC总转换时间（Convert Time）TCONV=采样时间+12.5个ADC周期
	  ADC_SampleTime_55Cycles5：55.5 cycles。即采样时间为55.5个ADCCLK周期
	  则TCONV=55.5+12.5=68个ADCCLK周期 = 68/9us = 7.56us
	*/
	
	
	/*ADC规则组的外部触发：使能或失能*/	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);             //（规则组）外部触发使能
	// ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);  //（注入组）外部触发使能
		
	
	
	
	
	// -------------------------------------------- ADC的规则组中断配置 -------------------------------------------------------
	
	/*
	    ADC1转换完成时，EOC置1，会触发ADC全局中断
		如果想要在中断函数里翻转LED电平来指示ADC的运行状态，可以使用ADC的全局中断来翻转LED电平
	*/
	
	/*清除ADC的中断标志位*/
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	 // 规则组、注入组都可以开启EOC中断
	ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);	 // 规则组与JEOC无关，只有注入组可以开启JEOC中断	             
	
	/*开启ADC的中断*/	
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);      // 使能ADC的EOC中断 （规则组或注入组 A/D转换结束）	
    // ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE);  // 使能ADC的JEOC中断（        注入组 A/D转换结束）


	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
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





	/*ADC使能*/
	ADC_Cmd(ADC1, ENABLE);									//使能ADC1，ADC开始运行

	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

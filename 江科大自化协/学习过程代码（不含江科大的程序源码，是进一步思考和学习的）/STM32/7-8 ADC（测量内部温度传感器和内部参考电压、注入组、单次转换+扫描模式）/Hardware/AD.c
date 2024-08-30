#include "stm32f10x.h"                  // Device header
#include "LED.h"

uint16_t AD_Value[2];					//定义用于存放AD转换结果的全局数组

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  */
void AD_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	// RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);						//选择时钟6分频，ADCCLK = 72MHz / 8 = 9MHz，一个ADCCLK周期为1/9us
	
	/*GPIO初始化*/
	// 测量内部温度传感器、内部参考电压，不用GPIO
	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	//ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;		//（只针对规则组）外部触发源，使用软件触发，不需要外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//连续转换，失能，每转换一次注入组序列后停止
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式，使能，扫描注入组的序列，扫描数量由ADC_NbrOfChannel确定
	//ADC_InitStructure.ADC_NbrOfChannel = 2;									//（只针对规则组）通道数，为2，扫描规则组的前2个通道
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1
	/*
	  结构体ADC_InitTypeDef中的成员ADC_NbrOfChannel和ADC_ExternalTrigConv，都是针对规则组的，无法影响注入组
	  	结构体成员ADC_NbrOfChannel：设置规则组（扫描模式）的前x个通道
	  	结构体成员ADC_ExternalTrigConv：设置规则组的外部触发源（包括设置软件触发，即ADC_ExternalTrigConv_None）

	  注入组的转换序列长度、外部触发源（包括软件触发）需要通过库函数设置
	  	如果注入组需要设置转换序列长度，就得使用ADC_InjectedSequencerLengthConfig()
	  	如果注入组需要配置外部触发源，就得使用ADC_ExternalTrigInjectedConvConfig()（包括设置软件触发，即参数ADC_ExternalTrigInjecConv_None）
	*/
	ADC_InjectedSequencerLengthConfig(ADC1, 2); 							  //（只针对注入组）通道数，为2，扫描规则组的前2个通道
	ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_None); //（只针对注入组）外部触发源，使用软件触发，不需要外部触发

	
	/*ADC规则组/注入组通道配置
	
	      规则组通道配置函数ADC_RegularChannelConfig，是配置规则序列寄存器（ADC_SRQx）的
		        如果规则组需要设置转换序列长度，使用结构体ADC_InitTypeDef的成员ADC_NbrOfChannel
				
		  注入组通道配置函数ADC_InjectedChannelConfig，是配置注入序列寄存器（ADC_JSRQ）的
		  	如果注入组需要设置转换序列长度，就得使用库函数ADC_InjectedSequenceLengthConfig()
		  
		  与规则组ADC_SRQx不同，注入组ADC_JSRQ要特别注意转换序列的长度
		      长度不足4时，需要通过ADC_InjectedSequencerLengthConfig()配置ADC_JSRQ的JL[1:0]位域
			  
	      即，ADC_RegularChannelConfig() 与     ADC_InitStructure.ADC_NbrOfChannel  的先后顺序无所谓
		      ADC_InjectedChannelConfig  必须在 ADC_InjectedSequencerLengthConfig() 之后调用
			  去看ADC_InjectedChannelConfig()的源码，就会知道需要提前设置好Injected_length
	*/
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);	//注入组序列1的位置，配置为通道ADC1_IN16，对应的内部温度传感器
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_239Cycles5);	//注入组序列2的位置，配置为通道ADC1_IN17，对应的内部参考电压值
	/*
	  STM32参考手册中，建议温度传感器的采样时间至少设置为17.1μs。这里的“采样时间”应该是指的是“ADC总转换时间”
	  ADC总转换时间（Convert Time）TCONV=采样时间+12.5个ADC周期
	  ADC_SampleTime_239Cycles5：239.5 cycles。即采样时间为239.5个ADCCLK周期
	  则TCONV=239.5+12.5=252个ADCCLK周期 = 252/9us = 28us
	*/
	
	
	/*ADC注入组的外部触发：使能或失能*/	
	// ADC_ExternalTrigConvCmd(ADC1, ENABLE);       //（规则组）外部触发使能
	ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);  //（注入组）外部触发使能
	
	
	/*温度传感器和Vrefint通道使能*/
	ADC_TempSensorVrefintCmd(ENABLE);   // ADC_CR2寄存器的TSVREFE位，置1
	

	/*ADC使能*/
	ADC_Cmd(ADC1, ENABLE);									//使能ADC1，ADC开始运行
	
	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

/**
  * 函    数：触发AD转换一次
  * 参    数：无
  * 返 回 值：无
  */
void AD_StartConv(void)
{   
	/* 触发AD转换，每个通道转换完，存入数据寄存器ADC_DR，同时触发DMA转运*/
	// ADC_SoftwareStartConvCmd(ADC1, ENABLE);			//（规则组）软件触发使能
	ADC_SoftwareStartInjectedConvCmd(ADC1, ENABLE); 	//（注入组）软件触发使能
	
	
	/* 等待第7个通道转换完成，就完了一轮扫描*/
	while( (ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == RESET) || (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) );	//等待JEOC和EOC标志位，即等待AD转换结束
	/*
	  想要在这里捕捉EOC从0到1的瞬间（即最后一个通道转换完成之后，EOC和JEOC都会置1）
	  由于注入组没有用到DMA，所以不存在DMA搬运，即不会自动读取ADC_DR寄存器，硬件不会自动将EOC清0，EOC只能由软件清0
	  此外，JEOC只能由软件清0
	  因此，肯定可以捕捉到EOC为1（高电平）的时期
	*/
	LED0_ON();

	
	AD_Value[0] = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1); // 读取ADC1_JDR1
	AD_Value[1] = ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_2); // 读取ADC1_JDR2	
	
    
	/* 清除EOC、JEOC标志位
	   EOC可以用软件清0，也可以由硬件清0.（当我们读取ADC_DR规则数据寄存器时，EOC会自动清0）
	      但是这里，我们有用到DMA搬运，也没有调用ADC_GetConversionValue();，即没有读取ADC_DR，所以还是得软件将EOC清0
	   
	   JEOC只能用软件清0，读取ADC_JDR注入数据寄存器时，硬件不会自动将JEOC清0
	   
	   注意：如果给while(ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == RESET);打断点，Debug，
	             可能会卡住，这是debug的问题，实际运行时不会出现
			 这里如果不给while(ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == RESET);打断点，Debug，
			     而是给ADC_SoftwareStartInjectedConvCmd();和LED0_ON();打断点，Debug，
				 就会发现EOC在while前后从0变1，符合实际运行规律
			 如果想要查看实际运行情况，
			 可以使用if(ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC) == SET)(LED0_ON;)来判断EOC是否为1
	*/	
	ADC_ClearFlag(ADC1, ADC_FLAG_JEOC); 	
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);   
}

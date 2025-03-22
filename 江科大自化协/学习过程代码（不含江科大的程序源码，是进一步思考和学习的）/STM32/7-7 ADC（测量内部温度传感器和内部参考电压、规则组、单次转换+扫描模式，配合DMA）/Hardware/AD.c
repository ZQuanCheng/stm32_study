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
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);						//选择时钟6分频，ADCCLK = 72MHz / 8 = 9MHz，一个ADCCLK周期为1/9us
	
	/*GPIO初始化*/
	// 测量内部温度传感器、内部参考电压，不用GPIO
	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//外部触发，使用软件触发，不需要外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;							//连续转换，失能，每转换一次规则组序列后停止
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式，使能，扫描规则组的序列，扫描数量由ADC_NbrOfChannel确定
	ADC_InitStructure.ADC_NbrOfChannel = 2;										//通道数，为2，扫描规则组的前7个通道
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1

	
	/*ADC规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5);	//规则组序列1的位置，配置为通道ADC1_IN16，对应的内部温度传感器
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_239Cycles5);	//规则组序列2的位置，配置为通道ADC1_IN17，对应的内部参考电压值
	/*
	  STM32参考手册中，建议温度传感器的采样时间至少设置为17.1μs。这里的“采样时间”应该是指的是“ADC总转换时间”
	  ADC总转换时间（Convert Time）TCONV=采样时间+12.5个ADC周期
	  ADC_SampleTime_239Cycles5：239.5 cycles。即采样时间为239.5个ADCCLK周期
	  则TCONV=239.5+12.5=252个ADCCLK周期 = 252/9us = 28us
	*/
	
	
	/*ADC规则组的外部触发：使能或失能*/	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);	


	/*温度传感器和Vrefint通道使能*/
	ADC_TempSensorVrefintCmd(ENABLE);   // ADC_CR2寄存器的TSVREFE位，置1
	
	
	/*复位外设DMA1的通道CH1*/
	DMA_DeInit(DMA1_Channel1);
	
	
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;											//定义结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;				//外设基地址，给定数据寄存器ADC_DR。等同于(uint32_t)(&(ADC1->DR))
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，对应16为的ADC数据寄存器
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，始终以ADC数据寄存器为源
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)AD_Value;					//存储器基地址，给定存放AD转换结果的全局数组AD_Value
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，与源数据宽度对应
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向，选择由外设到存储器，ADC数据寄存器转到数组
	DMA_InitStructure.DMA_BufferSize = 2;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;								//模式，选择正常模式，与ADC的单次转换一致
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
    //ADC单次转换的话，DMA就选择正常模式
    //ADC连续转换的话，DMA就选择循环模式	


	/*DMA和ADC使能*/
	//DMA_Cmd(DMA1_Channel1, ENABLE);						//ADC连续转换的话，这里直接使能
	DMA_Cmd(DMA1_Channel1, DISABLE);						//ADC单次转换的话，这里先不给使能，初始化后不会立刻工作，等后续调用AD_StartConv后，再ENABLE
	
	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
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
	/* 先使能DMA */
	DMA_Cmd(DMA1_Channel1, DISABLE);					//DMA失能，在写入传输计数器之前，需要DMA暂停工作
	DMA_SetCurrDataCounter(DMA1_Channel1, 2);	        //写入传输计数器，指定将要转运的次数
	DMA_Cmd(DMA1_Channel1, ENABLE);						//DMA使能，开始工作
	
	
	/* 触发AD转换，每个通道转换完，存入数据寄存器ADC_DR，同时触发DMA转运*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);					//软件触发AD转换一次
	
	
	/* 等待第7个通道转换完成，就完了一轮扫描*/
	// while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);	//等待EOC标志位，即等待AD转换结束
	/*
	  在这里捕捉EOC从0到1的瞬间（即最后一个通道转换完成之后，自动触发DMA将最后一个通道的数据转运走之前，这之间的时间很短）
	
	  纠结这个没意义，因为我们不需要判断EOC置1的时间，DMA搬运走（读取ADC_DR）时，硬件会自动将EOC清0
	                  即EOC从0到1后，马上又会变为0，高电平时间很短，一个while循环判断捕捉不到也没关系
					
	  我们只需要判断DMA是否工作完成就可以了，而且不用考虑A/D转换时间TCONV的限制
	*/
	LED0_ON();

    
	/* 等待DMA转运完第7个，即最后一个通道的数据 */
	while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);	    //等待DMA工作完成
	// 其实，DMA转运总是在AD转换之后，所以可以只判断DMA转运完成标志位DMA1_FLAG_TC1（一轮转运7个完成后，DMA1的ISR寄存器的TCIF1位置1）


	/* 清除EOC标志位*/	
	// ADC_ClearFlag(ADC1, ADC_FLAG_EOC);   //其实这里不用软件清除，当我们读取ADC_DR数据寄存器时，EOC由硬件自动清0。
	                                        //DMA搬运也是读取，也会自动将EOC清0
	
	
	/* 清除DMA工作完成标志位*/	
	DMA_ClearFlag(DMA1_FLAG_TC1);		//将DMA1的ISR寄存器的TCIF1位清0
}

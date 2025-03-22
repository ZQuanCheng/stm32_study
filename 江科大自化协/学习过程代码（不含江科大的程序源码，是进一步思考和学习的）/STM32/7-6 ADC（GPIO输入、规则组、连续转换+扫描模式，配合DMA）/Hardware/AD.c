#include "stm32f10x.h"                  // Device header

uint16_t AD_Value[7];					//定义用于存放AD转换结果的全局数组

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
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz，一个ADCCLK周期为1/12us
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				        //将PA0~PA6引脚初始化为模拟输入	
	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐，选择右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//外部触发，使用软件触发，不需要外部触发
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;							//连续转换，使能，每转换一次规则组序列后立刻开始下一次转换
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式，使能，扫描规则组的序列，扫描数量由ADC_NbrOfChannel确定
	ADC_InitStructure.ADC_NbrOfChannel = 7;										//通道数，为7，扫描规则组的前7个通道
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1


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
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_55Cycles5);	//规则组序列1的位置，配置为通道2
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_55Cycles5);	//规则组序列2的位置，配置为通道5
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_55Cycles5);	//规则组序列3的位置，配置为通道4
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 4, ADC_SampleTime_55Cycles5);	//规则组序列4的位置，配置为通道1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 5, ADC_SampleTime_55Cycles5);	//规则组序列5的位置，配置为通道0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 6, ADC_SampleTime_55Cycles5);	//规则组序列6的位置，配置为通道3
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_55Cycles5);	//规则组序列7的位置，配置为通道6
    // 这里改变对应顺序也没关系，只是AD转换的前后顺序变化了，响了DMA转运的次序，从而影存储在数组中的顺序发生了变化
	/*
	  ADC总转换时间（Convert Time）TCONV=采样时间+12.5个ADC周期
	  ADC_SampleTime_55Cycles5：55.5 cycles。即采样时间为55.5个ADCCLK周期
	  则TCONV=55.5+12.5=68个ADCCLK周期 = 68/12us = 5.67us
	*/
	
	
	/*ADC规则组的外部触发：使能或失能*/	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
	/*
	  其实，这句代码不是必要的，即使没调用（默认DISABLE），也不影响软件触发
	  但是，如果不加这句话，EOC就不会在扫描一轮后置1，而是一直为0
	        SR寄存器中的EOC位，由硬件在（规则或注入）通道组转换结束时设置
	  如果你后续想要while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);，即通过EOC位来等待转换结束的时机，
	  就需要使用ADC_ExternalTrigConvCmd(ADC1, ENABLE);，将CR2寄存器中的EXTTRIG位置1
	*/
	
	
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
	DMA_InitStructure.DMA_BufferSize = 7;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，与ADC的连续转换一致
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
    //ADC单次转换的话，DMA就选择正常模式
    //ADC连续转换的话，DMA就选择循环模式


	/*DMA和ADC使能*/
	DMA_Cmd(DMA1_Channel1, ENABLE);						    //ADC连续转换的话，这里直接使能。DMA1的通道1使能
	//DMA_Cmd(DMA1_Channel1, DISABLE);						//ADC单次转换的话，这里先不给使能，初始化后不会立刻工作，等后续调用AD_StartConv后，再ENABLE

	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
	ADC_Cmd(ADC1, ENABLE);									//使能ADC1，ADC开始运行
	
	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
	
	/*ADC触发*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//软件触发ADC开始工作，由于ADC处于连续转换模式，故触发一次后ADC就可以一直连续不断地工作
}

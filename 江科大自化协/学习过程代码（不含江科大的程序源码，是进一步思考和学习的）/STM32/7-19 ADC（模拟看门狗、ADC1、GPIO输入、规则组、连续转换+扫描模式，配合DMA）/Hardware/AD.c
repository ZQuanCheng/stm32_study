#include "stm32f10x.h"                  // Device header

uint16_t AD_Value[4];					//定义用于存放AD转换结果的全局数组

//define ADC_WATCHDOG_HT 0x0A00;
//define ADC_WATCHDOG_LT 0x0400;
const uint16_t ADC_WATCHDOG_HT = 0x0A00;
const uint16_t ADC_WATCHDOG_LT = 0x0400;

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
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				        //将PA0~PA3引脚初始化为模拟输入	
	
	
	// ----------------------------------------------- ADC1规则组配置 ------------------------------------------------------------------------
	
	/*
	  结构体ADC_InitTypeDef中的成员ADC_NbrOfChannel和ADC_ExternalTrigConv，都是针对规则组的，无法影响注入组
	  	结构体成员ADC_NbrOfChannel：设置规则组（扫描模式）的前x个通道
	  	结构体成员ADC_ExternalTrigConv：设置规则组的外部触发源（包括设置软件触发，即ADC_ExternalTrigConv_None）

	  注入组的转换序列长度、外部触发源（包括软件触发）需要通过库函数设置
	  	如果注入组需要设置转换序列长度，就得使用ADC_InjectedSequencerLengthConfig()
	  	如果注入组需要配置外部触发源，就得使用ADC_ExternalTrigInjectedConvConfig()（包括设置软件触发，即参数ADC_ExternalTrigInjecConv_None）
	*/
	
	/*定义ADC结构体变量*/	
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//模式，选择独立模式，即单独使用ADC1
	
	
	/*ADC1初始化*/	
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//（只针对规则组）外部触发源，无
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;							//连续转换
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式
	ADC_InitStructure.ADC_NbrOfChannel = 4;										//（只针对规则组）通道数，为4，扫描规则组的前4个通道
	ADC_Init(ADC1, &ADC_InitStructure);											//将结构体变量交给ADC_Init，配置ADC1


	/*ADC规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);	//ADC1-规则组序列1的位置，配置为通道ADC1_IN0，对应PA0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);	//ADC1-规则组序列2的位置，配置为通道ADC1_IN1，对应PA1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_55Cycles5);	//ADC1-规则组序列3的位置，配置为通道ADC1_IN2，对应PA2
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5);	//ADC1-规则组序列4的位置，配置为通道ADC1_IN3，对应PA3
	
	
	/*ADC规则组的外部触发：使能或失能*/	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);             //ADC1-（规则组）外部触发使能 
	
	
	/*主ADC1-DMA模式使能*/	
	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
	
	// ------------------------------------------------------------------------------------------------------------------------------------
		
		
		
	// ----------------------------------------------- DMA配置 ------------------------------------------------------------------------
		
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
	DMA_InitStructure.DMA_BufferSize = 4;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，与ADC的连续转换一致
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
    //ADC单次转换的话，DMA就选择正常模式
    //ADC连续转换的话，DMA就选择循环模式
	//如果是外部触发，即使ADC单次转换，也需要配置为循环模式，转运完一轮后才可以回到AD_Value开头，等待下一个外部触发信号
	
	
	/*DMA使能*/
	//DMA_Cmd(DMA1_Channel1, ENABLE);						//ADC连续转换的话，这里直接使能
	//DMA_Cmd(DMA1_Channel1, DISABLE);						//ADC单次转换的话，这里先不给使能，初始化后不会立刻工作，等后续调用AD_StartConv后，再ENABLE
	DMA_Cmd(DMA1_Channel1, ENABLE);     // 如果是外部触发，即使ADC单次转换，也需要直接使能。因为后续无法Enable，
	
	// ------------------------------------------------------------------------------------------------------------------------------------
	

	
	
	// -------------------------------------------- ADC1的模拟看门狗配置 -------------------------------------------------------

    /* 配置ADC模拟看门狗：在单个规则通道上*/
    ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);
    /* 配置ADC模拟看门狗：在整个规则通道组上*/
    //ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_AllRegEnable);


    /* 配置ADC模拟看门狗的阈值*/
	//ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0A00, 0x0400);
    ADC_AnalogWatchdogThresholdsConfig(ADC1, ADC_WATCHDOG_HT, ADC_WATCHDOG_LT);
	

    /* (单通道)配置ADC模拟看门狗*/
    ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_0);


	// -------------------------------------------- ADC1的中断配置（模拟看门狗中断） -------------------------------------------------------
		
	/*清除ADC的中断标志位*/
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	
	ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);	             
	ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);	
	
	/*开启ADC的中断*/		
    ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);   // 使能ADC1的AWD中断（模拟看门狗analog watchdog interrupt）

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
	
	
	/*ADC触发*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//软件触发ADC开始工作，由于ADC处于连续转换模式，故触发一次后ADC就可以一直连续不断地工作
}

#include "stm32f10x.h"                  // Device header
#include "LED.h"

uint16_t AD_Value[8];					//定义用于存放AD转换结果的全局数组

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
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);						//选择时钟6分频，ADCCLK = 72MHz / 8 = 9MHz，一个ADCCLK周期为1/9us
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);				        //将PA0~PA7引脚初始化为模拟输入	
	
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;								//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;				//模式，选择独立模式，即单独使用ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;			//数据对齐，选择右对齐
	
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO;	//（只针对规则组）外部触发源，使用EXTI的11号线EXTI_11	
	
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;				//连续转换，失能，每转换一次规则组序列后停止
	// 单次转换，是为了配合外部触发源
	
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;					//扫描模式，使能，扫描注入组的序列，扫描数量由ADC_NbrOfChannel确定
	//ADC_InitStructure.ADC_ScanConvMode = DISABLE;					//扫描模式，失能，只转换注入组的序列1这个位置。
    // 由于后续设置了间断模式，这里是非扫描模式、还是扫描模式，已经没区别了。间断模式只看规则组通道配置了几个，不会只转换序列1这一个位置。
	
	ADC_InitStructure.ADC_NbrOfChannel = 8;							//（只针对规则组）通道数，为8，扫描规则组的前8个通道
	ADC_Init(ADC1, &ADC_InitStructure);								//将结构体变量交给ADC_Init，配置ADC1
	/*
	  结构体ADC_InitTypeDef中的成员ADC_NbrOfChannel和ADC_ExternalTrigConv，都是针对规则组的，无法影响注入组
	  	结构体成员ADC_NbrOfChannel：设置规则组（扫描模式）的前x个通道
	  	结构体成员ADC_ExternalTrigConv：设置规则组的外部触发源（包括设置软件触发，即ADC_ExternalTrigConv_None）

	  注入组的转换序列长度、外部触发源（包括软件触发）需要通过库函数设置
	  	如果注入组需要设置转换序列长度，就得使用ADC_InjectedSequencerLengthConfig()
	  	如果注入组需要配置外部触发源，就得使用ADC_ExternalTrigInjectedConvConfig()（包括设置软件触发，即参数ADC_ExternalTrigInjecConv_None）
	*/
	
	
	/*ADC规则组的外部触发源
	
	  如果用EXTI_11，需要配置EXTI_11外部中断线为事件模式（注意，不是中断模式，这里EXTI_11仅作为事件，来触发ADC）
	  需要在AD.c中设置结构体的成员 ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO;	//（只针对规则组）外部触发源，使用EXTI_11		
    */
	
	
	/*ADC规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5);	//规则组序列1的位置，配置为通道ADC1_IN0，对应PA0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_55Cycles5);	//规则组序列2的位置，配置为通道ADC1_IN1，对应PA1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_55Cycles5);	//规则组序列3的位置，配置为通道ADC1_IN2，对应PA2
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_55Cycles5);	//规则组序列4的位置，配置为通道ADC1_IN3，对应PA3
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5);	//规则组序列5的位置，配置为通道ADC1_IN4，对应PA4
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_55Cycles5);	//规则组序列6的位置，配置为通道ADC1_IN5，对应PA5
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_55Cycles5);	//规则组序列7的位置，配置为通道ADC1_IN6，对应PA6
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_55Cycles5);	//规则组序列8的位置，配置为通道ADC1_IN7，对应PA7
	/*
	  ADC总转换时间（Convert Time）TCONV=采样时间+12.5个ADC周期
	  ADC_SampleTime_55Cycles5：55.5 cycles。即采样时间为55.5个ADCCLK周期
	  则TCONV=55.5+12.5=68个ADCCLK周期 = 68/9us = 7.56us
	*/
	
	
	/*ADC规则组的外部触发：使能或失能*/	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);             //（规则组）外部触发使能
	// ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);  //（注入组）外部触发使能
		
	
	/*ADC间断模式配置（discontinuous mode）*/
	ADC_DiscModeChannelCountConfig(ADC1, 3);    // 将ADC1的间断模式通道计数设置为3
	/*
	  解释如下：
	  第二个参数：间断模式下的转换次数。设置寄存器ADC_CR1的DISCNUM[2:0]位域
	  对于规则组来说，这个数字可以是1到8.
	  对于注入组来说，这个数字无效，没有影响。
	*/
	ADC_DiscModeCmd(ADC1, ENABLE);              // 使能ADC1（规则组）通道的间断模式
	//ADC_InjectedDiscModeCmd(ADC1, ENABLE);    // 使能ADC1（注入组）通道的间断模式
	/*
	  解释如下：
	  ADC_DiscModeCmd(ADC1, ENABLE);,            即寄存器ADC_CR1的DISCEN位置1， 规则组的间断模式使能
	  ADC_InjectedDiscModeCmd(ADC1, ENABLE);,    即寄存器ADC_CR1的JDISCEN位置1，注入组的间断模式使能
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
	DMA_InitStructure.DMA_BufferSize = 8;										//转运的数据大小（转运次数），与ADC通道数一致
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，具体解释见下方注释
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由ADC外设触发转运到存储器
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道1
    //ADC单次转换的话，DMA就选择正常模式
    //ADC连续转换的话，DMA就选择循环模式	
	//这里虽然ADC是单次转换，但是由于是外部触发源，需要配置为循环模式，转运完一轮后才可以回到AD_Value开头，等待下一个外部触发信号
	
	
	
	
	
	// -------------------------------------------- ADC的规则组中断配置 -------------------------------------------------------
	
	/*
	    这里的配置没有意义，由于DMA很快，EOC高电平实际很短，难以捕捉，所以无法触发ADC的全局中断
		如果想要在中断函数里翻转LED电平来指示ADC的运行状态，可以使用DMA的全局中断来翻转LED电平
	*/
	
	/*清除ADC的中断标志位*/
	// ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);	
	// ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);	             
	
	/*开启ADC的中断*/	
    // ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);  // 使能ADC的EOC中断 （规则组或注入组 A/D转换结束）	
    // ADC_ITConfig(ADC1, ADC_IT_JEOC, ENABLE); // 使能ADC的JEOC中断（        注入组 A/D转换结束）


	/*NVIC中断分组*/
	// NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	// NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	// NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;			//选择配置NVIC的ADC1_2线，ADC1和ADC2全局中断
	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	// NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	// -------------------------------------------------------------------------------------------------------------------------



	// -------------------------------------------- DMA的中断配置 -------------------------------------------------------
	
	/*清除DMA的中断标志位*/
	DMA_ClearITPendingBit(DMA1_IT_TC1);     // DMA1_CH1的TC（转运完成 transfer complete）

	/*开启ADC的中断*/
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);   // 使能DMA1_CH1的TC中断（转运完成 transfer complete）

	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			    //配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;	//选择配置NVIC的DMA1_Channel1线，DMA1_CH1全局中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//指定NVIC线路的抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	// -------------------------------------------------------------------------------------------------------------------------






	/*DMA和ADC使能*/
	//DMA_Cmd(DMA1_Channel1, ENABLE);						//ADC连续转换的话，这里直接使能
	//DMA_Cmd(DMA1_Channel1, DISABLE);						//ADC单次转换的话，这里先不给使能，初始化后不会立刻工作，等后续调用AD_StartConv后，再ENABLE
	DMA_Cmd(DMA1_Channel1, ENABLE);     // 这里由于是外部触发，无法软件控制Enable，需要直接使能
	
	ADC_DMACmd(ADC1, ENABLE);								//ADC1触发DMA1的信号使能
	ADC_Cmd(ADC1, ENABLE);									//使能ADC1，ADC开始运行

	
	/*ADC校准*/
	ADC_ResetCalibration(ADC1);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
}

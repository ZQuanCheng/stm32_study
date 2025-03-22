#include "stm32f10x.h"                  // Device header

// 参考手册中，TIM1_DMAR的寄存器地址偏移为0x4C
// stm32f10x.h中有   typedef struct{} TIM_TypeDef; 虽然成员是uint16_t类型，但是每个寄存器都会分32bit的空间，只不过高16bit没使用罢了
                     // 其中，有成员DMAR，DMAR之前有19个寄存器成员，每个寄存器占4个字节，所以偏移地址为76=0x4C个字节
// stm32f10x.h中有   #define TIM1                  ((TIM_TypeDef *) TIM1_BASE)
//                   #define TIM1_BASE             (APB2PERIPH_BASE + 0x2C00)
//                   #define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
//                   #define PERIPH_BASE           ((uint32_t)0x40000000)
// 从而，反推出TIM1_DMAR的地址为((uint32_t)0x40012C4C)
#define TIM1_DMAR           ((uint32_t)0x40012C4C)
//#define TIM1_DMAR           ((uint32_t)&TIM1->DMAR)    // 按理说这两种写法都可以，但是需要示波器对比验证一下

uint16_t buffer[8] = {100,200,300,400,500,600,700,800};   // 这里是uint16_t，是为了对应TIM1_DMAR的低16位（不使用高16bit）
	         // 占空比10%,20%,30%,40%,50%,60%,70%,80%,

//uint16_t buffer[8] = {500,600,700,800,100,200,300,400};   // CH1的占空比为50%和10%循环变动；CH2的占空比为60%和20%循环变动
	           // 占空比50%,60%,70%,80%,10%,20%,30%,40%

//uint16_t buffer[8] = {200,300,400,500,600,700,800,900};   // CH1的占空比为20%和60%循环变动；CH2的占空比为30%和70%循环变动
	           // 占空比20%,30%,40%,50%,60%,70%,80%,90%

//uint16_t buffer[8] = {100,200,300,400,900,1000,700,800};  // CH1的占空比为10%和90%循环变动；CH2的占空比为20%和100%循环变动
	           // 占空比10%,20%,30%,40%,90%,100%,70%,80%


/**
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
void MultiPWM_Timer_DMA_Mode_Init(void)
{
	/*开启时钟*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);			    //开启DMA1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			//开启TIM1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			//开启GPIOA的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);			//开启GPIOB的时钟	
		
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;	 // 对应TIM1的CH1、CH2、CH3、CH4通道	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							                     // 将PA8、PA9、PA10引脚初始化为复用推挽输出	


 	/*复位DMA外设*/
    // 使用普通DMA模式，单一触发源搬运单一地址值（DMA1的CH2、CH3、CH6、CH4、CH5分别对应TIM1的CH1、CH2、CH3、CH4、UP等DMA请求）
	DMA_DeInit(DMA1_Channel5);     // DMA1_Channel5的硬件触发源有TIM1_UP
	
	
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;	
	// TIMx_DMAR是DMA连续传送寄存器(DMA register for burst accesses)（相当于GD32中的发送缓冲区寄存器TIMERx_DMATB）
	// 对TIMx_DMAR寄存器的读或写会导致对以下地址所在寄存器的存取操作：TIMx_CR1地址 + DBA（存放偏移量） + DMA索引，DMA索引”由硬件计算，范围在0和DBL之间
	// 后续配置定时器DMA模式时，会有TIM_DMAConfig()配置TIMx_DCR寄存器的DBA[4:0]位域和DBL[4:0]位域。
	// DBA中是DMA基地址(DMA base address)（相对于TIMx_CR的地址偏移量），DBL中是DMA连续传送长度 (DMA burst length)（限制了被访问的目的地址范围）
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM1_DMAR;				//外设基地址，给定给定TIM1_DMAR，或者(uint32_t)&TIM1->DMAR
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，因为&TIM1->CCR1寄存器是低16bit有效
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，TIM1_CCR1固定。如果自增，就跑到TIM1_CCR2去了
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buffer;				    //存储器基地址，给定SRAM中的数组buffer[]
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，因为SRAM中的数组buffer是uint16_t类型的
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;							//数据传输方向，选择由存储器到外设，数组转到TIM1_CCR1数据寄存器
	DMA_InitStructure.DMA_BufferSize = 8;										//转运的数据大小（转运次数），这里是8。
														//这是为了让buffer[0-7]循环完。如果是4，数组只有一半[0-3]转运，然后就回到buffer[0]，占空比无法改变
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，连续不断修改占空比（3次、3次、...）
	//DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	   //不循环，选择修改一轮（3次）占空比，之后保持不变。
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由存储器转运到外设
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道5


	/*DMA使能*/
	DMA_Cmd(DMA1_Channel5, ENABLE);		//如果是循环模式DMA_Mode_Circula，这里直接使能
	//DMA_Cmd(DMA1_Channel5, DISABLE);	//如果是单次模式DMA_Mode_Normal，这里先不给使能，初始化后不会立刻工作，等手动ENABLE后，再开始	
	
	
/*  -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	TIM1时钟（TIM1CLK）固定为系统核心时钟，预分频器等于7199，
	TIM1计数器时钟 = 系统核心时钟 / 7200 = 10kHz。
	
    目标是配置定时器TIM1通道1~4（PA8~PA11）生成PWM信号
    捕获比较寄存器CCR1~4每轮循环更新2次	
	在第一次更新DMA请求时（第1个PWM波形周期结束），
	data1（buffer[0]=100）传输到CCR1，占空比给10%；data2（buffer[1]=200）传输到CCR2，占空比给20%；
	data3（buffer[2]=300）传输到CCR3，占空比给30%；data4（buffer[3]=400）传输到CCR4，占空比给40%；	
	
	在第二次更新DMA请求时（第2个PWM波形周期结束），
	data5（buffer[4]=500）传输到CCR1，占空比给50%；data6（buffer[5]=600）传输到CCR2，占空比给60%；
	data7（buffer[6]=700）传输到CCR3，占空比给70%；data8（buffer[7]=800）传输到CCR4，占空比给80%；


    TIM_DMAConfig(TIM1, TIM_DMABase_CCR1, TIM_DMABurstLength_4Transfers); TIM_DMA连续传送次数
	一次TIM1的更新事件（PWM波形的一个周期），会给DMA请求，连续搬运4个连续地址的存储器数据，给连续地址的4个外设数据寄存器CCR1、CCR2、CCR3、CCR4
	
	uint16_t buffer[]，即buffer[0]或buffer[4]传入CCR1时，直接复制低16bit位，不改变高16bit位。（CCR的高16bit位是无效的）
	------------------------------------------------------------------------------------- */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (7200-1+1) / (1000-1+1) = 10Hz，即计数周期为0.1s。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;				//计数周期，即ARR的值。最大65535U。
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;				//预分频器，即PSC的值。最大65535U。计数时钟频率CK_CNT为72MHz/(7200-1+1)=10kHz。
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
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;	  	// 通道输出状态：输出使能
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;	    // 互补通道输出状态：输出使能。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;		 	// 通道输出极性，选择为高，若选择极性为低，则输出高低电平取反。
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;	    	// 互补通道输出极性，同上。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;   	    // 空闲状态下通道输出：低电平。只有高级定时器需要配置
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;  	// 空闲状态下互补通道输出：同上。只有高级定时器需要配置	
	// 配置通道1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// 输出比较模式，选择PWM模式1	
	TIM_OCInitStructure.TIM_Pulse = 300;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 300 / (1000-1+1) = 30%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在通道1上的捕获/比较寄存器CCR1的预装功能
	// 配置通道2
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// 输出比较模式，选择PWM模式1	
	TIM_OCInitStructure.TIM_Pulse = 300;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 300 / (1000-1+1) = 30%
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC2Init，配置TIM1的输出比较通道2
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在通道2上的捕获/比较寄存器CCR2的预装功能
	// 配置通道3
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// 输出比较模式，选择PWM模式1	
	TIM_OCInitStructure.TIM_Pulse = 300;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 300 / (1000-1+1) = 30%
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC3Init，配置TIM1的输出比较通道3
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在通道3上的捕获/比较寄存器CCR3的预装功能
	// 配置通道4
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	// 输出比较模式，选择PWM模式1	
	TIM_OCInitStructure.TIM_Pulse = 300;				// CCR寄存器值。占空比duty = CCR / (ARR+1) = 300 / (1000-1+1) = 30%
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);			// 将结构体变量交给TIM_OC4Init，配置TIM1的输出比较通道4
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在通道4上的捕获/比较寄存器CCR4的预装功能


    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
	
		
	/*ARR预装功能*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		

	/*TIM1的DMA请求：使能TIM更新DMA源（TIM1 update DMA request enable）*/
	// 对应GD32的timer_dma_transfer(TIMER0, TIMER_DMACFG_DMATA_CH0CV, TIMER_DMACFG_DMATC_4TRANSFER)。
			// TIMER0的CH0CV寄存器作为DMA传输起始地址，DMA连续传送4次（目的地址自增）
			// 一轮循环中，2次定时器更新。一次定时器更新，触发一次DMA，DMA连续更新4个寄存器CHxCV(x=1,2,3,4)中的比较值CHxVAL[15:0](x=1,2,3,4)
	// 都是3个参数：某个定时器、DMA传输起始地址、DMA连续传送次数
	// 一轮循环中，2次定时器更新。一次定时器更新，触发一次DMA，DMA连续更新4个寄存器CCRx(x=1,2,3,4)中的比较值CCRx[15:0](x=1,2,3,4)
	TIM_DMAConfig(TIM1, TIM_DMABase_CCR1, TIM_DMABurstLength_4Transfers); // TIM_DMA连续传送次数
	
    // 对应GD32的timer_dma_enable(TIMER0, TIMER_DMA_UPD); 参数：某个定时器、待使能的DMA源
    // 这里多了个参数：某个定时器、待使能的DMA源、ENABLE或DISABLE
	// 使能或失能指定TIMx的DMA请求。
    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE); // 定时器更新，即一个周期结束后，会触发一次DMA请求，搬运一次buffer的数据。


	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}

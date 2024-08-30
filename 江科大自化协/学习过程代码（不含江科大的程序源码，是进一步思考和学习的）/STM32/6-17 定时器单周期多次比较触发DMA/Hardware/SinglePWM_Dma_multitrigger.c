#include "stm32f10x.h"                  // Device header

// 参考手册中，TIM1_CCR1的寄存器地址偏移为0x34
// stm32f10x.h中有   typedef struct{} TIM_TypeDef; 虽然成员是uint16_t类型，但是每个寄存器都会分32bit的空间，只不过高16bit没使用罢了
                     // 其中，有成员CCR1，CCR1之前有13个寄存器成员，每个寄存器占4个字节，所以偏移地址为52=0x34个字节
// stm32f10x.h中有   #define TIM1                  ((TIM_TypeDef *) TIM1_BASE)
//                   #define TIM1_BASE             (APB2PERIPH_BASE + 0x2C00)
//                   #define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
//                   #define PERIPH_BASE           ((uint32_t)0x40000000)
// 从而，反推出TIM1_CCR1的地址为((uint32_t)0x40012C34)
#define TIM1_CCR1           ((uint32_t)0x40012C34)
//#define TIM1_CCR1           ((uint32_t)&TIM1->CCR1)    // 按理说这两种写法都可以，但是需要示波器对比验证一下

uint16_t buffer[4] = {200,400,600,800};   // 这里是uint16_t，是为了对应TIM1_CCR1的低16位（不使用高16bit）

	
/*  -------------------------------------- 思路 ---------------------------------------------
	
	保证buffer是顺序正确的。
		uint16_t buffer[4]={200,400,600,800}
		uint16_t buffer[4]={800,200,400,600}
		uint16_t buffer[4]={600,800,200,400}
		uint16_t buffer[4]={400,600,800,200}
		以上顺序数组都可以
	但是，如果是{200,400,800,600}，那么PWM波形的占空比会交替变化，上一个周期的占空比60%、下一个周期的占空比80%。

    
    -------------------------------------- buffer[4]={200,400,600,800}的执行逻辑 ---------------------------------------------

	对于uint16_t buffer[4]={200,400,600,800}，执行逻辑如下：
		* 最开始，因为输出比较结构体参数设置为TIM_OCInitStructure.TIM_Pulse = buffer[0]； （CCR1=200）
		
		* 第一个循环是不完整的，只有1次比较中断
			** CNT从0开始自增。（CCR1=200）
                TIMl_CH1输出高电平的；
			** 当CNT(200)=CCR1(200)时，
                TIMl_CH1转而输出低电平的；
				这一瞬间触发了DMA，但是，DMA搬运的buffer[0]=200给CCR1，CCR1的值未变。
				所以此后一直CNT > CCR1(200),保持低电平
			** 直到CNT(1000)=ARR(1000自动重装)时，定时器TIM1更新。TIM1_CH1再次拉高
        
		* 第二个循环是完整的，有4次比较中断
			** CNT从0开始自增。（CCR1=200）
                TIMl_CH1输出高电平的；
			** 当CNT(200)=CCR1(200)时，
				本来TIM1_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[1]=400给CCR1，又有CNT(200) < CCR1(400)；
				所以低电平瞬间又拉高（示波器可以观察到毛刺）。
			** 当CNT(400)=CCR1(400)时，
				本来TIMl_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[2]=600给CCR1，又有CNT(400) < CCR1(600)；
				所以低电平瞬间又拉高（示波器可以观察到毛刺）。
			** 当CNT(600)=CCR1(600)时,
				本来TIM1_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[3]=800给CCR1，又有CNT(600) < CCR1(800)；
				所以低电平瞬问又拉高（示波器可以观察到毛刺）。
            
			    注意，由于DMA_InitStructure.DMA_BufferSize = 4; 
			    此时“DMA传输计数器”减到0，之前自增的地址，会恢复到起始地址的位置buffer[0]，以方便以后开始新一轮转运。

			** 当CNT(800)=CCR1(800)时，TIM1_CH1输出低电平。
				这一瞬间触发了DMA，DMA搬运buffer[0]=200给CCR1，为下一PWM周期准备
			** 直到CNT(1000)=ARR(1000自动重装)时，定时器TIM1更新。TIM1_CH1再次拉高
	
		* 此后，定时器执行逻辑稳定，循环往复	

    -------------------------------------- buffer[4]={400,600,800,200}的执行逻辑 ---------------------------------------------

	对于uint16_t buffer[4]={400,600,800,200}，执行逻辑如下：
		* 最开始，因为输出比较结构体参数设置为TIM_OCInitStructure.TIM_Pulse = buffer[0] （CCR1=400）
        
		* 第一个循环是不完整的，只有1次比较中断
			** CNT从0开始自增。（CCR1=400）
                TIMl_CH1输出高电平的；
			** 当CNT(400)=CCR1(400)时，
                TIMl_CH1转而输出低电平的；
				这一瞬间触发了DMA，但是，DMA搬运的buffer[0]=400给CCR1，CCR1的值未变。
				所以此后一直CNT > CCR1(400),保持低电平
			** 直到CNT(1000)=ARR(1000自动重装)时，定时器TIM1更新。TIM1_CH1再次拉高		
		
		* 第二个循环是不完整的，只有3次比较中断
			** CNT从0开始自增。（CCR1=400）
                TIMl_CH1输出高电平的；
			** 当CNT(400)=CCR1(400)时，
				本来TIMl_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[1]=600给CCR1，又有CNT(400) < CCR1(600)；
				所以低电平瞬间又拉高（示波器可以观察到毛刺）。
			** 当CNT(600)=CCR1(600)时,
				本来TIM1_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[2]=800给CCR1，又有CNT(600) < CCR1(800)；
				所以低电平瞬问又拉高（示波器可以观察到毛刺）。
			** 当CNT(800)=CCR1(800)时，
				TIM1_CH1输出低电平。
				这一瞬间触发了DMA，DMA搬运buffer[3]=200给CCR1，为下一PWM周期准备

			    注意，由于DMA_InitStructure.DMA_BufferSize = 4; 
			    此时“DMA传输计数器”减到0，之前自增的地址，会恢复到起始地址的位置buffer[0]，以方便以后开始新一轮转运。

			** 直到CNT(1000)=ARR(1000自动重装)时，定时器TIM1更新。TIM1_CH1再次拉高	
	
		* 第三个循环是完整的，有4次比较中断
			** CNT从0开始自增。（CCR1=200）
                TIMl_CH1输出高电平的；
			** 当CNT(200)=CCR1(200)时，
				本来TIM1_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[0]=400给CCR1，又有CNT(200) < CCR1(400)；
				所以低电平瞬间又拉高（示波器可以观察到毛刺）。
			** 当CNT(400)=CCR1(400)时，
				本来TIMl_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[1]=600给CCR1，又有CNT(400) < CCR1(600)；
				所以低电平瞬间又拉高（示波器可以观察到毛刺）。
			** 当CNT(600)=CCR1(600)时,
				本来TIM1_CH1是要输出低电平的；
				但是由于这一瞬间触发了DMA，DMA搬运buffer[2]=800给CCR1，又有CNT(600) < CCR1(800)；
				所以低电平瞬问又拉高（示波器可以观察到毛刺）。
			** 当CNT(800)=CCR1(800)时，TIM1_CH1输出低电平。
				这一瞬间触发了DMA，DMA搬运buffer[3]=200给CCR1，为下一PWM周期准备

			    注意，由于DMA_InitStructure.DMA_BufferSize = 4; 
			    此时“DMA传输计数器”减到0，之前自增的地址，会恢复到起始地址的位置buffer[0]，以方便以后开始新一轮转运。

			** 直到CNT(1000)=ARR(1000自动重装)时，定时器TIM1更新。TIM1_CH1再次拉高
	
		* 此后，定时器执行逻辑稳定，循环往复		
		
    -------------------------------------- 要修改的参数 --------------------------------------------
	
	要修改的参数：
		* 修改DMA通道：
			TIM1_CH1对应DMA通道2，即DMA1_Channel2
			不是原来TIM1_UP对应的DMA通道5，即DMA1_Channel5
		* 修改DMA触发源：
			CH1比较触发DMA，即TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);
			而不是更新事件触发DMA，即TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);

		* 注意，这里必须保证OC影子寄存器禁能Disable，不然只能更新事件时才能修改CCR的值，一个周期内无法多次修改CCR1的值
    
	
    -------------------------------------- 中断处理，帮助检查 --------------------------------------------
	
	中断处理，帮助检查：
		* 可以使用TMI1_CC_IRQHandler，在比较中断函数中进行操作，来判断是否在一个周期中，出现了多次比较中断
		* 可以使用TIM1_UP_IRQHandler，在更新中断函数中进行操作。
	
	-------------------------------------------------------------------------------------  */


/**
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
void SinglePWM_DMA_MultiTRIG_Init(void)
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
	DMA_DeInit(DMA1_Channel2);     // DMA1_Channel2的硬件触发源有TIM1_CH1
	
	
	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;											//定义DMA结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)TIM1_CCR1;				//外设基地址，给定给定TIM1_CCR1，或者(uint32_t)&TIM1->CCR1
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度，选择半字，因为&TIM1->CCR1寄存器是低16bit有效
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址自增，选择失能，TIM1_CCR1固定。如果自增，就跑到TIM1_CCR2去了
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buffer;				    //存储器基地址，给定SRAM中的数组buffer[]
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度，选择半字，因为SRAM中的数组buffer是uint16_t类型的
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能，每次转运后，数组移到下一个位置
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;							//数据传输方向，选择由存储器到外设，数组转到TIM1_CCR1数据寄存器
	DMA_InitStructure.DMA_BufferSize = 4;										//转运的数据大小（转运次数），这里是4。这是为了让buffer[0-3]循环完
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//模式，选择循环模式，连续不断修改占空比（3次、3次、...）
	//DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;	  //不循环，选择修改一轮（4次）占空比，之后保持不变。
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//存储器到存储器，选择失能，数据由存储器转运到外设
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);								//将结构体变量交给DMA_Init，配置DMA1的通道2


	/*DMA使能*/
	DMA_Cmd(DMA1_Channel2, ENABLE);		//如果是循环模式DMA_Mode_Circula，这里直接使能
	//DMA_Cmd(DMA1_Channel2, DISABLE);	//如果是单次模式DMA_Mode_Normal，这里先不给使能，初始化后不会立刻工作，等手动ENABLE后，再开始	
	
	
/*  -------------------------------------------------------------------------------------
	定时器TIM1配置为：
	TIM1时钟（TIM1CLK）固定为系统核心时钟，预分频器等于71，
	TIM1计数器时钟 = 系统核心时钟 / 72 = 1MHz。
	
    目标是配置定时器TIM1通道1（PA8）生成PWM信号
    捕获比较寄存器CCR1每轮循环更新4次	
	在第一次更新DMA请求时（CNT=200），data1（buffer[0]=400）传输到CCR1
	在第二次更新DMA请求时（CNT=400），data2（buffer[1]=600）传输到CCR1
	在第三次更新DMA请求时（CNT=600），data3（buffer[2]=800）传输到CCR1	
	在第四次更新DMA请求时（CNT=800），data4（buffer[3]=200）传输到CCR1		
	-------------------------------------------------------------------------------------  */


	/*复位外设TIM1*/
	TIM_DeInit(TIM1);
		

	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		//选择TIM1为内部时钟，若不调用此函数，TIM默认也为内部时钟
		
		
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	// 此参数TIM_ClockDivision仅用于输出比较-死区时间配置、输入捕获-滤波器采样，不影响定时周期。
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	// 以下4个参数影响时基单元的定时周期
	// 定时频率=计数器的溢出频率CK_CNT_OVT = CK_PSC / (PSC+1) / (ARR+1) = 72MHZ / (72-1+1) / (1000-1+1) = 1kHz，即计数周期为1ms。
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;				//计数周期，即ARR的值。最大65535U。
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;				//预分频器，即PSC的值。最大65535U。计数时钟频率CK_CNT为72MHz/(72-1+1)=1MHz。
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
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	 // 输出比较模式，选择PWM模式1	
	TIM_OCInitStructure.TIM_Pulse = buffer[0];		     // CCR寄存器值。占空比duty = CCR / (ARR+1) = 400 / (4000-1+1) = 40%
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);			 // 将结构体变量交给TIM_OC1Init，配置TIM1的输出比较通道1
    
	// 注意，这里必须保证OC影子寄存器禁能Disable，不然只能更新事件时才能修改CCR的值，一个周期内无法多次修改CCR1的值

	/*CCR预装功能*/
	// 用于配置CCR捕获/比较寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是CHxAVL寄存器，代码为timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
	// STM32中是下列代码
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);  // 使能或失能TIMx在通道1上的捕获/比较寄存器CCR1的预装功能
	                                                    // 仅用于输出比较
	// 当输出PWM波控制电机时，通过反馈值和PID算法，得到下个周期的控制参数。那么我们就Disable，等更新事件后再写入寄存器。
	// 当输出给ADC采样的时候，一个周期内要多次采样。这次采完，下次采样值就会更高或更低，这时候要Enable，使得修改值立即写入。


    /*使能TIM1的主输出*/
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  // 只有高级定时器需要TIM_CtrlPWMOutputs()，将所有的输出通道使能或者失能
	
		
	/*ARR预装功能：自动重装寄存器的影子寄存器*/
	// 用于配置ARR自动重装寄存器的预装功能的，即影子寄存器。写入的值不会立即生效，而是在更新事件才会生效，一般不用。
	// GD32中对应的是ACR寄存器，代码为timer_auto_reload_shadow_enable(TIMER0);
	// STM32中是下列代码
    TIM_ARRPreloadConfig(TIM1, ENABLE);	// 这里是Disable，不使用ARR上的影子寄存器
		
		
		
		
		
	// -------------------------------- 中断处理，帮助检查，非必要 --------------------------------------	
		
	/*中断输出配置*/
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);					//开启TIM1的更新中断
	TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);					    //开启TIM1的CC1通道的比较中断
	
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
																//即抢占优先级范围：0~3，响应优先级范围：0~3
																//此分组配置在整个工程中仅需调用一次
																//若有多个中断，可以把此代码放在main函数内，while循环之前
																//若调用多次配置分组的代码，则后执行的配置会覆盖先执行的配置
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	// 使能TIM1的更新中断，优先级较高，对应的处理函数TIM1_UP_IRQHandler。当计数值CNT，等于ARR（自动重装寄存器）的值时，触发中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;			//选择配置NVIC的TIM1_UP线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//指定NVIC线路的响应优先级为0
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设		
	// 使能TIM1的捕获/比较中断，优先级较低，对应的处理函数TIM1_CC_IRQHandler。当计数值CNT，等于CCR1的值时，触发中断
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;			//选择配置NVIC的TIM1_CC线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设					
		
	// ---------------------------------------------------------------------------------------------------			
		
		
		
	

	/*TIM1 update DMA request enable*/
    // 对应GD32的timer_dma_enable(TIMER0, TIMER_DMA_UPD); 参数：某个定时器、待使能的DMA源
    // 这里多了个参数：某个定时器、待使能的DMA源、ENABLE或DISABLE
	// 使能或失能指定TIMx的DMA请求。
	TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);      // 计数器CNT的值，自增到通道CC1的捕获/比较寄存器CCR1的值时，会触发一次DMA请求，搬运一次buffer的数据。
    //TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE); // 定时器更新，即一个周期结束后，会触发一次DMA请求，搬运一次buffer的数据。


	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			//使能TIM1，定时器开始运行
}

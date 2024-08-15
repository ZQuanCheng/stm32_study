#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "OLED.h"
#include "AD.h"
#include "EXTI_11.h"

uint16_t dma_it_num;   // 记录进入DMA全局中断的次数

int main(void)
{
	/*模块初始化*/
	OLED_Init();			//OLED初始化
	LED_Init(); 			//LED0（贴片LED）初始化
	AD_Init();				//AD初始化
	
	EXTI_11_Init();	
	/*
	      如果用EXTI_11，需要配置EXTI_11外部中断线为事件模式（注意，不是中断模式，这里EXTI_11仅作为事件，来触发ADC）
	
	      需要在AD.c中设置结构体的成员 ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO;	//（只针对规则组）外部触发源，使用EXTI_11
	
	      由于规则组的EXTI触发源只有EXTI_11线，STM32F103C8T6最小系统板上有PA11和PB11的引脚，所以将外部中断EXTI_11线映射到PA11和PB11都可以
	      为了提供上升沿，将User_Key连接到PA11（PB11）。这样，按下Key时，就会给PA11（PB11）一个上升沿，从而顺利触发一次ADC。
	      一轮扫描结束后，DMA也搬运结束，就会触发DMA中断。在DMA1_Channel1_IRQHandler中翻转LED的电平，看出来确实与Key同步。
	*/		
	

	
	LED0_ON();
	
	
	/*
	     由于这里有8个AD转换结果值，OLED只能显示4行，就有2种方法查看
	     一、 用仿真Debug，将AD_Value数据加入Watch窗口，直接让程序一直跑，就可以观测到AD值的更新，和LED、按键是否一致
	     二、每行显示2个AD值
	*/
	
		
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "AD0_1:");
	OLED_ShowString(2, 1, "AD2_3:");
	OLED_ShowString(3, 1, "AD4_5:");
	OLED_ShowString(4, 1, "AD6_7:");
	

	
	while (1)
	{	
		//LED0_Turn();
		
		OLED_ShowNum(1, 7,  AD_Value[0], 4);		//显示转换结果第0个数据
		OLED_ShowNum(1, 12, AD_Value[1], 4);		//显示转换结果第1个数据	

		OLED_ShowNum(2, 7,  AD_Value[2], 4);		//显示转换结果第2个数据
		OLED_ShowNum(2, 12, AD_Value[3], 4);		//显示转换结果第3个数据		
		
		OLED_ShowNum(3, 7,  AD_Value[4], 4);		//显示转换结果第4个数据
		OLED_ShowNum(3, 12, AD_Value[5], 4);		//显示转换结果第5个数据				
		
		OLED_ShowNum(4, 7,  AD_Value[6], 4);		//显示转换结果第6个数据
		OLED_ShowNum(4, 12, AD_Value[7], 4);		//显示转换结果第7个数据			
		
		Delay_ms(100);			//延时100ms，手动增加一些转换的间隔时间
	}
}

	/*
	    由于DMA很快，EOC高电平实际很短，难以捕捉，所以无法触发ADC的全局中断
		如果想要在中断函数里翻转LED电平来指示ADC的运行状态，可以使用DMA的全局中断来翻转LED电平
	*/

/**
  * 函    数：ADC1_2中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
/*
void ADC1_2_IRQHandler(void)
{
	if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)          //判断是否是ADC1中的EOC（规则组或注入组转换结束）触发的中断
	{
		//ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);			//清除ADC1中的EOC（规则组或注入组转换结束）事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
		
		LED0_Turn();  // 翻转LED，指示进入中断的频率

	    // 清除EOC、JEOC标志位
	    // ADC_ClearFlag(ADC1, ADC_FLAG_JEOC); 	
	    // ADC_ClearFlag(ADC1, ADC_FLAG_EOC); 	// DMA自动搬运，读取了ADC_DR寄存器，会由硬件清除EOC标志位
	}
}
*/

	/*
	    由于DMA很快，EOC高电平实际很短，难以捕捉，所以无法触发ADC的全局中断
		如果想要在中断函数里翻转LED电平来指示ADC的运行状态，可以使用DMA的全局中断来翻转LED电平
	*/
	
/**
  * 函    数：DMA1_Channel1中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1) == SET)                //判断是否是DMA1_CH1的TC（转运完成 transfer complete）触发的中断
	{
		DMA_ClearITPendingBit(DMA1_IT_TC1);			        //清除DMA1_CH1中的TC（转运完成 transfer complete））事件的中断标志位
															//中断标志位必须清除
															//否则中断将连续不断地触发，导致主程序卡死
		
		LED0_Turn();  // 翻转LED，指示进入中断的频率
		/*
		  TIM3的更新周期是1S，那么输出比较触发的频率也是1S，
		  在DMA1_Channel1_IRQHandler中翻转LED的电平，看出来确实是1s。
		  而且数据变动也是1s一次。即使OLED是100ms刷新一次
		*/
		
		
		dma_it_num++;   // 记录进入DMA全局中断的次数
		
		
	    /* 清除EOC、JEOC标志位 */
	    // ADC_ClearFlag(ADC1, ADC_FLAG_JEOC); 	
	    // ADC_ClearFlag(ADC1, ADC_FLAG_EOC); 	// DMA自动搬运，读取了ADC_DR寄存器，会由硬件清除EOC标志位
	}
}


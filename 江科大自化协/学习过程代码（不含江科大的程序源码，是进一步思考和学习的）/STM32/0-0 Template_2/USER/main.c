#include "stm32f10x.h"

int main(void)
{
	/*  寄存器方式，实现PC13口的配置
	RCC->APB2ENR = 0x00000010; //  APB2 外设时钟使能寄存器(RCC_APB2ENR):IOPCEN这一位写1，打开GPIOC的时钟。
	GPIOC->CRH = 0x00300000;   //  端口配置高寄存器(GPIOx_CRH): CNF13配置为通用推挽输出模式，即这两位为00; MOD13配置为输出模式、最大速度50MHz，即这两位为11
    GPIOC->ODR = 0x00000000;   //  端口输出数据寄存器(GPIOx_ODR): ODR13这一位写1，则PC13口就是高电平；
	*/
	
	// 库函数方式，实现PC13口的配置
	// 使能GPIOC的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	// 配置GPIOC的端口模式
	GPIO_InitTypeDef GPIO_InitStructure;                  // 声明结构体GPIO_InitStructure
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;            // 选中管脚 13
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // 最高输出速率 50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 通用推挽输出
	GPIO_Init(GPIOC, &GPIO_InitStructure);                // 端口初始化
	// PC13端口输出
	// GPIO_SetBits(GPIOC, GPIO_Pin_13);    // 输出高电平
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);  // 输出低电平
	
	
	while (1){
	
	}

}

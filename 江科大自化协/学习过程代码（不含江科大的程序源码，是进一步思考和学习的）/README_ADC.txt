7-3 ADC（规则组、单次转换+非扫描模式）：
                  AD.C：规则组、单次转换、非扫描模式、使能外部GPIO的模拟输入（PA2)
                  
                 main.c：手动进行软件触发转换, Oled显示AD值、以及对应的电压值
                               如果将Dealy_ms设置为1000，在仿真的watch窗口可以看到，（while的一次循环1s内）AD_Value的值只能更新—次，说明工作在单次转换模式


7-4 ADC（规则组、连续转换+非扫描模式）：
                  AD.C：规则组、连续转换、非扫描模式、使能外部GPIO的模拟输入 （PA2)
                  
                 main.c：手动进行软件触发转换, Oled显示AD值、以及对应的电压值
                               如果将Dealy_ms设置为1000，在仿真的watch窗口可以看到，（while的一次循环1s内）AD_Value的值有多次更新，说明工作在连续转换模式



7-5 ADC（GPIO输入、规则组、单次转换+扫描模式、配合DMA）：
                  AD.C：规则组、单次转换、扫描模式（配合DMA）、使能外部GPIO的模拟输入 （PA0~PA6)
                  
                 main.c：手动进行软件触发转换，Oled显示AD值
                               如果将Dealy_ms设置为1000，在仿真的watch窗口可以看到，（while的一次循环1s内）AD_Value[0]~ AD_Value[3]的值只能更新—次，说明工作在单次转换模式


7-6 ADC（GPIO输入、规则组、连续转换+扫描模式、配合DMA）：
                  AD.C：规则组、连续转换、扫描模式（配合DMA）、使能外部GPIO的模拟输入 （PA0~PA6)
                  
                 main.c：手动进行软件触发转换，Oled显示AD值
                               如果将Dealy_ms设置为1000，在仿真的watch窗口可以看到，（while的一次循环1s内）AD_Value[0]~ AD_Value[3]的值有多次更新，说明工作在连续转换模式


7-7 ADC（测量内部温度传感器和内部参考电压、规则组、单次转换+扫描模式，配合DMA）：
                  AD.C：规则组、单次转换、扫描模式（配合DMA）、使能内部温度传感器IN16和内部参考电压的通道N17

                  main.c：手动进行软件触发转换，Oled显示AD值、以及转换后的对应值


7-8 ADC（测量内部温度传感器和内部参考电压、注入组、单次转换+扫描模式）：
                  AD.C：注入组、单次转换、扫描模式（不需要DMA）、使能内部温度传感器IN16和内部参考电压的通道N17

                  main.c：手动进行软件触发转换，Oled显示AD值、以及转换后的对应值


7-9 ADC（GPIO输入4个、注入组、单次转换+扫描模式、定时器触发ADC）
                  AD.C：ADC1、注入组、单次转换、扫描模式（不需要DMA）。可使用PA0-PA3。
                             外部触发源放在main.c中配置ADC_ExternalTriglnjectedConvConfig()。
                             此外，使能了ADC的EOC中断、JEOC中断

                  TIM1_TRGO.c：如果用TIM1_TRGO，需要配置主模式输出TRGO，但是不需要配置输出比较OC结构体；

                  TIM2_CC1.c：如果用TIM2_CC1，需要配置输出比较OC结构体，但是不需要配置主模式输出TRGO；

                  main.c：可选择TIM1_TRGO_Init(); 和 ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T1_TRGO);
                                可选择TIM2_CC1_Init();    和  ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_T2_CC1);
                                * 定时器TIM1或者TIM2进行外部硬件触发转换
                                * ADC1转换完成后，会触发ADC全局中断，我们在中断函数ADC1_2_IRQHandler中进行读取ADC_JDRx(x=1,2,3,4)的数据
                                * Oled显示AD值


7-10 ADC（GPIO输入4个、规则组、单次转换+扫描模式、配合DMA、定时器触发ADC）
                  AD.C：ADC1、规则组、单次转换、扫描模式（需要DMA）。可使用PA0-PA3。
                             外部触发源可以在AD.c中的结构体成员ADC_ExternalTrigConv中配置。
                             在这里，即使使能了ADC的EOC中断，也无法触发ADC的全局中断。因为DMA很快，EOC高电平时间很短，难以捕捉，所以无法触发ADC的全局中断
                             如果想要在中断函数中，翻转LED电平来指示ADC的运行状态，只能使用DMA的全局中断来翻转LED电平
                             所以，这里使能了DMA1_CH1的FTF中断

                  TIM3_TRGO.c：如果用TIM3_TRGO，需要配置主模式输出TRGO，但是不需要配置输出比较OC结构体；

                  TIM1_CC1.c：如果用TIM1_CC1，需要配置输出比较OC结构体，但是不需要配置主模式输出TRGO；

                  main.c：可选择TIM3_TRGO_Init(); 和  ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
                                可选择TIM1_CC1_Init();    和  ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
                                * 定时器TIM3或者TIM1进行外部硬件触发转换
                                *  ADC1转换完成时，DMA也会马上转运完成，会触发DMA全局中断，我们在中断函数DMA1_Channel1_IRQHandler()中进行LED电平翻转
                                * Oled显示AD值



7-11 ADC（GPIO输入1个、规则组、单次转换+扫描模式、定时器触发ADC）
                  AD.C：ADC1、规则组、单次转换、扫描模式（不需要DMA）。只输入一个PA0。
                              这里，由于只有一个ADC输入，是否扫描模式，没区别；而且不需要DMA进行转运，一个DR数据寄存器就够用了
                             外部触发源可以在AD.c中的结构体成员ADC_ExternalTrigConv中配置
                             此外，使能了ADC的EOC中断。既然没有DMA，就不会硬件将EOC自动清0，就可以顺利触发ADC的全局中断

                  TIM3_TRGO.c：如果用TIM3_TRGO，需要配置主模式输出TRGO，但是不需要配置输出比较OC结构体；

                  TIM1_CC1.c：如果用TIM1_CC1，需要配置输出比较OC结构体，但是不需要配置主模式输出TRGO；

                  main.c：可选择TIM3_TRGO_Init(); 和  ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
                                可选择TIM1_CC1_Init();    和  ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
                                * 定时器TIM3或者TIM1进行外部硬件触发转换
                                *  ADC1转换完成时，EOC置1，会触发ADC全局中断，我们在中断函数ADC1_2_IRQHandler()中进行LED电平翻转、读取ADC_DR的数据到AD_Value变量中
                                * Oled显示AD值













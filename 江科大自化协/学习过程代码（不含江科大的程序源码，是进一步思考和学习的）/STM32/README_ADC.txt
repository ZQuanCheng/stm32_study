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



7-11 ADC（GPIO输入1个、规则组、单次转换、定时器触发ADC）
                  AD.C：ADC1、规则组、单次转换。只输入一个PA1。
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



7-12 ADC（GPIO输入8个、规则组、单次转换+扫描模式、配合DMA、EXTI触发ADC）
                  AD.C：ADC1、规则组、单次转换、扫描模式（需要DMA）。可使用PA0-PA7.
                             外部触发源可以在AD.c中的结构体成员ADC_ExternalTrigConv中配置。
                             由于规则组的EXTI触发源只有EXTI_11线，所以将外部中断EXTI_11号线映射到PA11（或PB11）
                             为了提供上升沿，将按键Key连接到PA11（一端接PA11，另一端接VCC）。这样，按下Key时，就会给PA11一个上升沿，从而顺利触发一次ADC
                             在这里，即使使能了ADC的EOC中断，也无法触发ADC的全局中断。因为DMA很快，EOC高电平时间很短，难以捕捉，所以无法触发ADC的全局中断
                             如果想要在中断中翻转LED电平来指示ADC的运行状态，只能使用DMA的全局中断来翻转LED电平
                             所以，这里使能了DMA1_CH1的FTF中断

                            注意，如果不使用Key按键来给PA11上升沿，可以直接用杜邦线连接VCC，那么当拔走VCC时（想要给下降沿），可能也会触发一次。
                            因为抖动，又没有滤波电容，电压波动也会有小的上升沿

                  EXTl_11.0：EXTl_11，需要配置EXTL_11外部中断线为事件模式（注意，不是中断模式。这里EXTL_11仅作为事件，来触发ADC）；

                  main.c：EXTl_11_Init(); 和 ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO;
                               * EXTI_11进行外部硬件触发转换
                               * ADC1转换完成时，DMA也会马上转运完成，会触发DMA全局中断，我们在中断函数DMA1_Channel1_IRQHandler中进行LED电平翻转
                               * Oled显示AD值



7-13 ADC（GPIO输入8个、规则组、单次转换+间断模式、配合DMA、EXTI触发ADC）
                  AD.C：ADC1、规则组、单次转换、间断模式（需要DMA） 。 可使用PA0-PA7.
                             只需要在上一个程序7-12的基础上。① 加一句ADC_DiscModeChannelCountConfig(ADC1, 3); ② 加一句ADC_DiscModeCmd(ADC1, ENABLE);
                            这里，由于是间断模式，① 非扫描模式DISABLE ② 扫描模式ENABLE，这两种没有区别，不影响间断模式的执行。
                            需要按下3次按键，才能触发3次ADC转换（第1次，转换序列1、2、3；第2次，转换序列4、5、6；第3次，转换序列7，8；EOC置1, DMA转运完成、触发DMA中断）
                            即Key按下3次，才能翻转一次LED电平



7-14 ADC（GPIO输入4个、注入组、单次转换+扫描模式、EXTI触发ADC）
                  AD.C：ADC1、注入组、单次转换、扫描模式（不需要DMA）。可使用PA0-PA3.
                             外部触发源可以在AD.c中的ADC_ExternalTrigInjectedConvConfig()库函数中配置。
                             由于注入组的EXTI触发源只有EXTI_15线，所以将外部中断EXTI_15号线映射到PB15（或PA15）
                             为了提供上升沿，将按键Key连接到PB15（一端接PB15，另一端接VCC）。这样，按下Key时，就会给PB15一个上升沿，从而顺利触发一次ADC
                             此外，使能了ADC的JEOC中断。

                            注意，如果不使用Key按键来给PB15上升沿，可以直接用杜邦线连接VCC，那么当拔走VCC时（想要给下降沿），可能也会触发一次。
                            因为抖动，又没有滤波电容，电压波动也会有小的上升沿

                  EXTl_15.0：EXTl_15，需要配置EXTL_15外部中断线为事件模式（注意，不是中断模式。这里EXTL_15仅作为事件，来触发ADC）；

                  main.c：EXTl_15_Init(); 和 ADC_ExternalTrigInjectedConvConfig(ADC1, ADC_ExternalTrigInjecConv_Ext_IT15_TIM8_CC4);
                               * EXTI_15进行外部硬件触发转换
                               * ADC1转换完成后，会触发ADC全局中断，我们在中断函数ADC1_2_IRQHandler中进行LED电平翻转、ADC1_JDRx(x=1,2,3,4)的数据
                               * Oled显示AD值



7-15 ADC（GPIO输入4个、注入组、单次转换+间断模式、EXTI触发ADC）
                  AD.C：ADC1、注入组、单次转换、间断模式（不需要DMA） 。 可使用PA0-PA3.
                             只需要在上一个程序7-14的基础上。加一句ADC_InjectedDiscModeCmd(ADC1, ENABLE);
                            这里，由于是间断模式，① 非扫描模式DISABLE ② 扫描模式ENABLE，这两种没有区别，不影响间断模式的执行。
                            需要按下4次按键，才能触发4次ADC转换（第1次，转换序列1；第2次，转换序列2；第3次，转换序列3；第4次，转换序列4；EOC置1, 触发ADC中断）
                            即Key按下4次，才能翻转一次LED电平

                            注意，由于没有DMA实时搬运，在Debug的Watch窗口中，会在第4次按下时统一更新到AD_Value数据中。
                            如果想实时看，就打开System Viewer Windows，看ADC1的JDRx(x=1,2,3,4)注入组数据寄存器，更新顺序与按键按下一致

                            注意，STM32上一个特别现象，估计是IC硬件问题:
                            * 扫描模式、按下4次按键:（第1次，转换序列1；第2次，转换序列2；第3次，转换序列3；第4次，转换序列4；EOC置1, 触发ADC中断）
                            * 非扫描模式、按下4次按键:（上来就将转换序列1转换了，触发了一次ADC中断。然后第1次，转换序列2；第2次，转换序列3；第3次，转换序列4；第4次，转换序列1；EOC置1, 触发ADC中断）
                            即非扫描模式+间断模式，出现了一种类似bug的现象。这种现象在GD32F303上没有出现。



7-16 ADC（GPIO输入2个、规则组、单次转换+扫描模式、配合DMA、定时器触发ADC）
                  AD.C：ADC1、规则组、单次转换、扫描模式（需要DMA）。可使用PA0-PA1。
                             (在7-10的AD.c基础上修改)
                             GPIO配置：PA0~PA3，改成PA0-PA1。
                             外部触发源：TIM1_CC1，改成TIM2_CC2。
                             扫描通道数：4，改成2。
                             规则组序列：序列1~4，改成序列1~2。
                             DMA转运次数buffer：4，改成2。
                             DMA数据宽度：半字16bit，改成字32bit
                             数组：uint16_t AD_Value[4];   改成  uint32_t AD_Value[2];


                  TIM2_CC2.c：如果用TIM2_CC2，需要配置输出比较OC结构体，但是不需要配置主模式输出TRGO;
                             (在7-10的TIM1_CC1.c基础上修改)
                             时钟：APB2-TIM1、改成APB1-TIM2
                             ARR值：50000改成10000，保证定时周期为1s。(对应的，CCR的值改为5000,，反正1~9999都行)
                             PWM模式：模式1，改成模式2。(这个倒是无所谓，不影响效果)


                  main.c：选择TIM2_CC2_Init();    和  ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;
                                * 记得修改OLED显示长度：4改成8
                                * 定时器TIM2进行外部硬件触发转换
                                * ADC1转换完成时，DMA也会马上转运完成，会触发DMA全局中断，我们在中断函数DMA1_Channel1_IRQHandler()中进行LED电平翻转
                                * Oled显示AD值



7-17 ADC（双ADC、同步规则模式、GPIO输入2个、规则组、单次转换+扫描模式、配合DMA、定时器触发ADC）
                  只需要在7-16 ADC（GPIO输入2个、规则组、单次转换+扫描模式、配合DMA、定时器触发ADC）的基础上修改AD.c就可以了
                  即对照“主ADC1”的配置，添加“从ADC2”的配置

                  AD.c：ADC1、规则组、单次转换、扫描模式（需要DMA）。可使用PA0-PA1。
                                    整体ADC外设设置：ADC_Mode_RegSimult，即ADC1和ADC2运行“同步规则”模式
                                    对于ADC1：规则组、单次转换、扫描模式、通道数为2（序列1、2对应使用PA0、PA1）、数据右对齐、定时器TIM2_CC2触发、 ADC1-DMA模式使能
                                    对于ADC2：规则组、单次转换、扫描模式、通道数为2（序列1、2对应便用PA1、PA0）、数据右对齐、无外部触发源、
                                    此外，别忘了添加“从ADC2”的时钟



7-18 ADC（双ADC、同步注入模式、GPIO输入4个、注入组、单次转换+扫描模式、定时器触发ADC）
                  只需要在7-9 ADC（GPIO输入4个、注入组、单次转换+扫描模式、定时器触发ADC）的基础上修改AD.c就可以了
                  即对照“主ADC1”的配置，添加“从ADC2”的配置

                  AD.c：ADC1、规则组、单次转换、扫描模式（需要DMA）。可使用PA0-PA1。
                                    整体ADC外设设置：ADC_Mode_InjecSimult，即ADC1和ADC2运行“同步注入”模式
                                    对于ADC1：注入组、单次转换、扫描模式、通道数为4（序列0~3对应使用通道IN0~IN3，即PA0~PA3）、数据右对齐、定时器TIM2_CC1触发
                                                      uint16_t ADC1_Value[4];
                                    对于ADC2：注入组、单次转换、扫描模式、通道数为4（序列0~3对应便用通道IN3~IN0，即PA3~PA0）、数据右对齐、无外部触发源
                                                      uint16_t ADC2_Value[4];
                                    此外，别忘了添加“从ADC2”的时钟
                                    只需要ADC1的中断，不用开启ADC2的中断。因为都设置为一样的ADC_SampleTime_55Cycles5，ADC1转换结束时，ADC2也结束了。
                  
                  也可以修改下触发的频率，即TIM2的定时周期
                  TIM2_CC1.c：如果用TIM2_CC1，需要配置输出比较OC结构体，但是不需要配置主模式输出TRGO;
                             时钟：APB1-TIM2
                             ARR值：50000改成10000，保证定时周期为1s。(对应的，CCR的值改为5000,，反正1~9999都行)
                             PWM模式：模式1，改成模式2。(这个倒是无所谓，不影响效果)



7-19 ADC（模拟看门狗、GPIO输入、规则组、连续转换+扫描模式，配合DMA）
                需要在ADC输入通道的引脚上，接可调电阻
                但是也不一定，因为输入通道INx浮空时也有值（0x0100~0x0C00），我们设置上下限制，包含浮空值就行

                  AD.c：
                          只需要在7-6 ADC（GPIO输入、规则组、连续转换+扫描模式，配合DMA）的基础上修改AD.c就可以了
                          不用ADC2，因为ADC2无法使用DMA搬运，这样我们不方便观察adc_value
                          添加了模拟看门狗配置、失能模拟看门狗中断

               mian.c：
                          在模拟看门狗中断中，点亮LED，弄一个int变量，方便在while循环中进行逻辑判断



7-20 ADC（模拟看门狗、ADC2、GPIO输入、规则组、连续转换）
            只有一个输入，扫描与否无所谓了







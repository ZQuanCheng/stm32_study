/*!
    \file    readme.txt
    \brief   description of the CMP Blanking_output demo

    \version 2024-01-05, V1.2.0, firmware for GD32H7xx
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

  This demo is based on the GD32H759I-EVAL-V1.1 board, it shows how to configure the
comparator blank function.

  CMP0 is configured as following:
  - Inverting input is internally connected to VREFINT = 1.2V.
  - Non inverting input is connected to PB0.
  - Output is internally connected to port output(PC5).
  - TIMER1_CH2 is selected as blanking source.
  - PB0 is connected to TIMER1_CH3(PA3).
  TIMER1 is configured as following:
  - TIMER clock frequency is set to systemcoreclock.
  - Prescaler is set to 599 make the counter clock is 1MHz.
  - Configure TIMER1_CH2 as the blank source.
  - Counter autoreload value is 9999 and CH2/CH3 output compare value is 499/4999.
  - TIMER1_CH2/TIMER1_CH3 outputs PWM with 5/50% dutycycle.

  Connect PA3 and PB0 as CMP non inverting input. use oscilloscope to observe CMP
input pin(PB0) and output pin(PC5) at the same time, the output signal is low level at the
beginning of the dutycycle.

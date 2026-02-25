#include "Swim_TIM2_CH1_CH2_TIM5_CH3_B.h"


static u8 SWIM_Inited;




//开启DMA1_S6_CH3  注意IFCR是基于stream流通道号的，并非channel
//ucos版本必须用强制内联函数？否则反复下载会有失败情况出现，裸机程序不需要内联？
//#pragma inline = forced
void SYNCSWPWM_IN_TIMER_RISE_DMA_INIT_B(u8 length, u32* address)
{
  SYNCSWPWM_IN_TIMER_RISE_DMA->CR &= ~1;
  DMA1->HIFCR |= (0x3F<<16);
  SYNCSWPWM_IN_TIMER_RISE_DMA->NDTR = length;
  SYNCSWPWM_IN_TIMER_RISE_DMA->M0AR = (u32)address;
  SYNCSWPWM_IN_TIMER_RISE_DMA->CR |= 0x00000001;
}


//开启DMA1_S0_CH6
//ucos版本必须用强制内联函数？否则反复下载会有失败情况出现，裸机程序不需要内联？
//#pragma inline = forced
void SYNCSWPWM_OUT_TIMER_DMA_INIT_B(u8 length, u32* address)
{
  SYNCSWPWM_OUT_TIMER->EGR = TIM_PSCReloadMode_Immediate;
  SYNCSWPWM_OUT_TIMER_DMA->CR &= ~1;
  DMA1->LIFCR |= (0x3F);
  SYNCSWPWM_OUT_TIMER_DMA->NDTR = length;
  SYNCSWPWM_OUT_TIMER_DMA->M0AR = (u32)address;
  SYNCSWPWM_OUT_TIMER_DMA->CR |= 0x00000001;
}


#define SYNCSWPWM_IN_TIMER_RISE_DMA_READY()		(DMA1->HISR & (1<<21))
#define SYNCSWPWM_IN_TIMER_RISE_DMA_RESET()		(DMA1->HIFCR |= (0x3F<<16))

void SYNCSWPWM_IN_TIMER_RISE_DMA_WAIT_B(u32 dly)
{
  while((!SYNCSWPWM_IN_TIMER_RISE_DMA_READY()) && --dly);
  SYNCSWPWM_IN_TIMER_RISE_DMA_RESET();
}
                                                                                                  
void SYNCSWPWM_OUT_TIMER_DMA_WAIT_B(u32 dly)
{
  while(!(DMA1->LISR & (1<<5))&&--dly);
  DMA1->LIFCR |= (0x3F);
}

void SYNCSWPWM_OUT_TIMER_SetCycle_B(u32 cycle)
{
  SYNCSWPWM_OUT_TIMER->ARR = (cycle);
  SYNCSWPWM_OUT_TIMER->EGR = TIM_PSCReloadMode_Immediate;
}

/*
功能：IO口初始化配置，
RST复位脚即PB1脚 设置为推挽输出。
SWIM输入口PD12,PD13设置为复用功能PWM输入捕获状态。
SWIM输出口TIM3_CH3 即PB0脚 设置为复用功能开漏输出状态。
输入：无
输出：无
时间：2015年12月30号
作者：郭东
*/
void SWIM_GPIO_Init_SWIM_B(void *pswim)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  /*使能GPIO时钟，使能复用功能IO时钟*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
  
  /*使能TIM5，TIM2时钟*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5|RCC_APB1Periph_TIM2, ENABLE);
  
  
  /*TIM5_CH3,即SWIM输出口映射到PA2脚*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM5); 
  
  
  /*PA3 SWIM RST复位脚输出*/
  SWIM_RST_HIGH(pswim);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_SWIM_RSTOUT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  
  GPIO_WriteBit(GPIOA, GPIO_SWIM_OUT_PIN, Bit_SET);
  /*PA2 SWIM OUTPUT PWM输出 这里先初始化为普通IO*/
  GPIO_InitStructure.GPIO_Pin = GPIO_SWIM_OUT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出，高电平靠外接两个并联的1K电阻（合计为500R）
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  
  /*PDA0,PA1 SWIM INPUT PWM输入模式TIM2_CH1、TIM2_CH2 */
  /*必须放在GPIO_Init之前，否则如果放在GPIO_Init后面，GPIO_Init会产生一个短时间的低电平*/  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2);  
  
  
  /*PA0,PA1 SWIM INPUT PWM输入模式*/
  GPIO_InitStructure.GPIO_Pin = GPIO_SWIM_IN1_PIN | GPIO_SWIM_IN2_PIN;			
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  
  /*设置为PWM输入复用功能后，GPIO_WriteBit已经无效，不能再控制IO电平*/
//  GPIO_WriteBit(GPIOD, GPIO_SWIM_IN1_PIN, Bit_SET);
//  GPIO_WriteBit(GPIOD, GPIO_SWIM_IN2_PIN, Bit_SET);
//  GPIO_WriteBit(GPIOD, GPIO_SWIM_IN1_PIN, Bit_RESET);
//  GPIO_WriteBit(GPIOD, GPIO_SWIM_IN2_PIN, Bit_RESET);
//  GPIO_WriteBit(GPIOD, GPIO_SWIM_IN1_PIN, Bit_SET);
//  GPIO_WriteBit(GPIOD, GPIO_SWIM_IN2_PIN, Bit_SET);

}



/*
功能：SWIM输出口PB0脚 设置为复用功能开漏输出状态TIM5_CH3 。
输入：无
输出：无
时间：2023年08月29号
作者：郭东
*/
void SWIM_Init_B(void)
{
  GPIO_InitTypeDef GPIO_InitStructure; 
  DMA_InitTypeDef DMA_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;
  if (!SWIM_Inited)
  {
    SWIM_Inited = 1;
    
    
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    
    
    DMA_DeInit(SYNCSWPWM_OUT_TIMER_DMA);
    while(DMA_GetCmdStatus(SYNCSWPWM_OUT_TIMER_DMA)!=DISABLE); //等待DMA可配置，当离开while循环后，DMA就已经使能了  
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SYNCSWPWM_OUT_TIMER->CCR3);
    
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_InitStructure.DMA_Channel=DMA_Channel_6;  //DMA通道选择
    DMA_Init(SYNCSWPWM_OUT_TIMER_DMA, &DMA_InitStructure);
    
    
    
    DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_TCIF0);
    DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_HTIF0);
    DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_FEIF0);
    DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_DMEIF0);
    DMA_Cmd(SYNCSWPWM_OUT_TIMER_DMA, ENABLE);
    
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(SYNCSWPWM_OUT_TIMER, &TIM_TimeBaseStructure);
    
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    //TIM_OCInitStructure.TIM_OCPolarity = (p) ? TIM_OCPolarity_High : TIM_OCPolarity_Low;// p=0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OC3Init(SYNCSWPWM_OUT_TIMER, &TIM_OCInitStructure);
    
    TIM_OC3PreloadConfig(SYNCSWPWM_OUT_TIMER, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(SYNCSWPWM_OUT_TIMER, ENABLE);
    TIM_DMACmd(SYNCSWPWM_OUT_TIMER, TIM_DMA_CC3, ENABLE);
    TIM_Cmd(SYNCSWPWM_OUT_TIMER, ENABLE);
   // TIM_CtrlPWMOutputs(SYNCSWPWM_OUT_TIMER, ENABLE);//只有高级定时器TIM1和TIM8才有这BDTR这个功能寄存器
    
    
    

    
    //端口切换到第二功能输出口
    /*以下IO口配置必须放在TIM定时器设置的后边设置才行，否则编程不正常，过不去*/
    
    
  //  GPIO_WriteBit(GPIOB, GPIO_SWIM_OUT_PIN, Bit_SET);
    

    
    GPIO_InitStructure.GPIO_Pin = GPIO_SWIM_OUT_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能开漏输出
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
    /*TIM5_CH3,即SWIM输出口映射到PA2脚*/
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM5); 

    
    
  }
}


/*
功能：SWIM输出口设置为默认开漏状态。SWIM输入口对应的内部定时器的时钟及DMA关闭
输入：无
输出：无
时间：2015年12月30号
作者：郭东
*/
void SWIM_DeInit_B(void *pswim)
{
  GPIO_InitTypeDef GPIO_InitStructure; 
  
  /*SWIM_PWM输出脚*/
  GPIO_InitStructure.GPIO_Pin = GPIO_SWIM_OUT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出，高电平靠外接两个并联的1K电阻（合计为500R）
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  
  
  /*关输入定时器TIM2时钟TIM2_CH2 PD13脚，及其对应输出DMA通道DMA1_S6_CH3/
  TIM_DeInit(SYNCSWPWM_IN_TIMER);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
  DMA_DeInit(SYNCSWPWM_IN_TIMER_RISE_DMA);
 // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, DISABLE);
  
  
  /*关输出定时器TIM5时钟TIM5_CH3 PA2脚，及其对应输出DMA通道DMA1_S0_CH6*/
  TIM_DeInit(SYNCSWPWM_OUT_TIMER);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, DISABLE);
  DMA_DeInit(SYNCSWPWM_OUT_TIMER_DMA);
 // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, DISABLE);
  

  
  
  /*PB1 SWIM RST复位脚输出*/
  SWIM_RST_HIGH(pswim);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_SWIM_RSTOUT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  SWIM_Inited = 0;
}


/*
功能：SWIM输入口对应的内部定时器TIM2_CH2的时钟及D1S6C3使能
输入：无
输出：无
时间：2015年12月30号
作者：郭东
*/
void SWIM_EnableClockInput_B(void)
{

  
  DMA_InitTypeDef DMA_InitStructure;
  TIM_ICInitTypeDef TIM_ICInitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  
  
  

  
  
  DMA_DeInit(SYNCSWPWM_IN_TIMER_RISE_DMA);
  while(DMA_GetCmdStatus(SYNCSWPWM_IN_TIMER_RISE_DMA)!=DISABLE); //等待DMA可配置，当离开while循环后，DMA就已经使能了  
  DMA_StructInit(&DMA_InitStructure);
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SYNCSWPWM_IN_TIMER->CCR2);//测PWM输入占空比
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_InitStructure.DMA_Channel=DMA_Channel_3;  //DMA通道选择
  DMA_Init(SYNCSWPWM_IN_TIMER_RISE_DMA, &DMA_InitStructure);
  
  
  
  DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_TCIF6);
  DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_HTIF6);
  DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_FEIF6);
  DMA_ClearITPendingBit(SYNCSWPWM_IN_TIMER_RISE_DMA, DMA_IT_DMEIF6);
  DMA_Cmd(SYNCSWPWM_IN_TIMER_RISE_DMA, ENABLE);
  
  
  


  
  
  
  
////   //时基初始化
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
//  TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //死区控制用。
//  TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  //计数器方向
//  TIM_TimeBaseInitStructure.TIM_Prescaler = 2;   //Timer clock = sysclock /(TIM_Prescaler+1) = 2M
//  TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
//  TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;    //Period = (TIM counter clock / TIM output clock) - 1 = 40Hz 
//  TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);
  
  
  TIM_Cmd(SYNCSWPWM_IN_TIMER, DISABLE);
  TIM_DMACmd(SYNCSWPWM_IN_TIMER, TIM_DMA_CC2, DISABLE);
  
  TIM_ICStructInit(&TIM_ICInitStructure);
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_IndirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_PWMIConfig(SYNCSWPWM_IN_TIMER, &TIM_ICInitStructure);
  
  TIM_SelectInputTrigger(SYNCSWPWM_IN_TIMER, TIM_TS_TI1FP1);
  TIM_SelectSlaveMode(SYNCSWPWM_IN_TIMER, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(SYNCSWPWM_IN_TIMER, TIM_MasterSlaveMode_Enable);
  TIM_DMACmd(SYNCSWPWM_IN_TIMER, TIM_DMA_CC2, ENABLE);
  
  TIM_PrescalerConfig(SYNCSWPWM_IN_TIMER, 0, TIM_PSCReloadMode_Immediate);
  
  
  TIM_SetCounter(SYNCSWPWM_IN_TIMER, 0);
  TIM_SetCompare1(SYNCSWPWM_IN_TIMER, 0);
  TIM_SetCompare2(SYNCSWPWM_IN_TIMER, 0);
  
  TIM_ClearITPendingBit(SYNCSWPWM_IN_TIMER, TIM_IT_Update);
  TIM_ClearITPendingBit(SYNCSWPWM_IN_TIMER, TIM_IT_CC1);
  TIM_ClearITPendingBit(SYNCSWPWM_IN_TIMER, TIM_IT_CC2);
  TIM_ClearITPendingBit(SYNCSWPWM_IN_TIMER, TIM_IT_CC3);
  TIM_ClearITPendingBit(SYNCSWPWM_IN_TIMER, TIM_IT_CC4);
  
    /* Clear the IT pending Bit */
  SYNCSWPWM_IN_TIMER->SR = (uint16_t)~(1<<9);
  TIM_ClearITPendingBit(SYNCSWPWM_IN_TIMER, TIM_IT_Trigger);
    
  TIM_Cmd(SYNCSWPWM_IN_TIMER, ENABLE);
  
}
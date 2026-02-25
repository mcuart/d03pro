#include "Swim_TIM2_CH1_CH2_TIM5_CH3.h"

extern cJSON *root;
extern u8 flash_control_reg_address;

static FIL FilSys;


static u8 FileData[4096]; 
static u8 fill0[100];
static u8 SendData[512*2];


static u8 SWIM_Inited ;
static u16 SWIM_PULSE_0;
static u16 SWIM_PULSE_1;
static u16 SWIM_PULSE_Threshold;
static u32 SWIM_DMA_IN_Buffer[12];
static u32 SWIM_DMA_OUT_Buffer[12];
static uint16_t SWIM_clock_div = 0;
static u8 ReadBuff[256];
static u8 WriteBuff[12];
static u8 cmdrtv;
static u32 ReadCount;




static const u32 STM8_FLASH_PUKR_ADDRESS[2]   = {0x00005062,0x00005052};
static const u32 STM8_FLASH_DUKR_ADDRESS[2]   = {0x00005064,0x00005053};
static const u32 STM8_FLASH_CR2_ADDRESS[2]    = {0x0000505B,0x00005051};
static const u32 STM8_FLASH_IAPSR_ADDRESS[2] = {0x0000505F,0x00005054};
static u8 STM8_READ_OUT_PROTECTION[2] = {0x00,0xAA};

static const u8 STM8_FLASH_CR2_VALUE[2]={0x10,0x01};




static void SWIM_DeInit();
static u8 SWIM_HW_In(u8* data, u8 bitlen);
static u8 SWIM_HW_Out(u8 cmd, u8 bitlen, u16 retry_cnt);
static u8 SWIM_ROTF(uint32_t addr, uint16_t len, u8 *data);
static u8 SWIM_WOTF(uint32_t addr, uint16_t len, u8 *data);
static u8 EraseFlash(void);
static u8 EraseEEPROM(void);

static u32 JiaMiSuanFa_STM8(u8 gongshi,u32 changshu, u8 D[]);





//开启DMA1_S6_CH3  注意IFCR是基于stream流通道号的，并非channel
//ucos版本必须用强制内联函数？否则反复下载会有失败情况出现，裸机程序不需要内联？
#pragma inline = forced
static void SYNCSWPWM_IN_TIMER_RISE_DMA_INIT(u8 length, u32* address)
{
  
  SYNCSWPWM_IN_TIMER_RISE_DMA->CR &= ~1;
  DMA1->HIFCR |= (0x3F<<16);
  SYNCSWPWM_IN_TIMER_RISE_DMA->NDTR = length;
  SYNCSWPWM_IN_TIMER_RISE_DMA->M0AR = (u32)address;
  SYNCSWPWM_IN_TIMER_RISE_DMA->CR |= 0x00000001;
}


//开启DMA1_S0_CH6
//ucos版本必须用强制内联函数？否则反复下载会有失败情况出现，裸机程序不需要内联？
#pragma inline = forced
static void SYNCSWPWM_OUT_TIMER_DMA_INIT(u8 length, u32* address)
{
  SYNCSWPWM_OUT_TIMER->EGR = TIM_PSCReloadMode_Immediate;
  SYNCSWPWM_OUT_TIMER_DMA->CR &= ~1;
  DMA1->LIFCR |= (0x3F);
  SYNCSWPWM_OUT_TIMER_DMA->NDTR = length;
  SYNCSWPWM_OUT_TIMER_DMA->M0AR = (u32)address;
  SYNCSWPWM_OUT_TIMER_DMA->CR |= 0x00000001;
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
void SWIM_GPIO_Init_SWIM_0(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  /*使能GPIO时钟，使能复用功能IO时钟*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
  
  /*使能TIM5，TIM2时钟*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5|RCC_APB1Periph_TIM2, ENABLE);
  
  
  /*TIM5_CH3,即SWIM输出口映射到PA2脚*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_TIM5); 
  
  
  /*PA3 SWIM RST复位脚输出*/
  SWIM_RST_HIGH();
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
static void SWIM_Init(void)
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
static void SWIM_DeInit(void)
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
  SWIM_RST_HIGH();
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
static void SWIM_EnableClockInput(void)
{
  SWIM_clock_div = 0;
  
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


static u8 SWIM_EnterProgMode(void)
{
  u8 i;
  uint32_t dly;


  SYNCSWPWM_IN_TIMER_RISE_DMA_INIT(10, SWIM_DMA_IN_Buffer); 			//分配DMA存储空间
  SWIM_LOW();
  
  delay_ms(1);
 // delay_us(500);
  
  
  
  for (i = 0; i < 4; i++)
  {
    SWIM_HIGH();
    // System_Delay_us(500);
    delay_us(500);
    SWIM_LOW();
    //System_Delay_us(500);
    delay_us(500);
  }
  for (i = 0; i < 4; i++)
  {
    SWIM_HIGH();
    //System_Delay_us(250);
    delay_us(250);
    SWIM_LOW();
    //System_Delay_us(250);
    delay_us(250);
  }
  SWIM_HIGH();//这里拉高之后目标MCU会发送一个低电平同步帧给烧录器，STM8L051F3实测拉低时间为16微妙
  dly = SWIM_MAX_DLY;
  SYNCSWPWM_IN_TIMER_RISE_DMA_WAIT(dly);
  delay_us(250);//开始SWIM通信之前，SWIM线必须释放为高电平，以保证SWIM准备好通信（至少维持300ns)
  if (!dly)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}



//设置SWIM通讯时钟参数
//默认SWIM时钟为HSI/2=8MHz,周期=0.125us x 22拍 =2.75us
static u8 SWIM_SetClockParam(u8 mHz, u8 cnt0, u8 cnt1)		//cnt0和cnt1构成一个周期
{
  uint16_t clock_div;
  
  if (SWIM_clock_div)
  {
    clock_div = SWIM_clock_div;
  }
  else
  {
    clock_div = SYNCSWPWM_OUT_TIMER_MHZ / mHz;			
    if ((SYNCSWPWM_OUT_TIMER_MHZ % mHz) >= (mHz / 2))	
    {
      clock_div++;
    }
    clock_div *= SWIM_SYNC_CYCLES;							//9x128=1152
  }
  
  SWIM_PULSE_0 = cnt0 * clock_div / SWIM_SYNC_CYCLES;
  if ((cnt0 * clock_div % SWIM_SYNC_CYCLES) >= SWIM_SYNC_CYCLES / 2)
  {
    SWIM_PULSE_0++;
  }
  SWIM_PULSE_1 = cnt1 * clock_div / SWIM_SYNC_CYCLES;
  if ((cnt1 * clock_div % SWIM_SYNC_CYCLES) >= SWIM_SYNC_CYCLES / 2)
  {
    SWIM_PULSE_1++;
  }
  SWIM_PULSE_Threshold = SWIM_PULSE_0 + SWIM_PULSE_1;
  
  SYNCSWPWM_OUT_TIMER_SetCycle(SWIM_PULSE_Threshold);
  SWIM_PULSE_Threshold >>= 1;
  return 0;
}





static u8 SWIM_SRST(void)
{
  return SWIM_HW_Out(SWIM_CMD_SRST, SWIM_CMD_BITLEN, SWIM_MAX_RESEND_CNT);
}


u8 Pro_Init_Auto_SWIM_0(void)
{
  SWIM_RST_HIGH();
  
  
  
  //System_Delay_ms(1);
  delay_ms(1);
  SWIM_RST_LOW();
  
  //System_Delay_ms(1);  
  
  delay_ms(1);
  SWIM_EnableClockInput();		
  cmdrtv=SWIM_EnterProgMode();
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 1;
  }
  return 0;
}






u8 Pro_Init_SWIM_0(void)
{
  static u16 tmp=0;
#define SWIM_CSR_SAFE_MASK 0x80
#define SWIM_CSR_SWIM_DM 0x20
#define SWIM_CSR_HS 0x10   
#define SWIM_CSR_RST 0x04 
#define SWIM_CSR_HSIT 0x02
#define SWIM_CSR_PRI 0x01
#define DM_CSR2_STALL 0x08

  SWIM_RST_HIGH();//IO口初始化时候应该默认设置成高电平，这里再次设置高电平以防IO初始化时候漏设置高电平
  //System_Delay_ms(1);
  //delay_ms(10);
  delay_ms(1);
  SWIM_RST_LOW();
  // System_Delay_ms(1);
    delay_ms(1);
  SWIM_EnableClockInput();		
  cmdrtv=SWIM_EnterProgMode();
  
  
  tmp=50;
  while(cmdrtv>0&&tmp>0)
  {
    cmdrtv=SWIM_EnterProgMode();
    
    tmp--;
    
    
  }
  
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 1;
  }
  
  SWIM_Inited=0;
  SWIM_Init();//将PB0配置为复用PWM输出比较模式，配置TIIM3_CH3 PWM输出
  SWIM_SetClockParam(8,20,2);//SWIM初始时钟为HSI/2=8MHz	
  
  
  cmdrtv=SWIM_SRST();
  
  
  //System_Delay_ms(1);
  delay_ms(1);
  WriteBuff[0] = SWIM_CSR_SAFE_MASK|SWIM_CSR_SWIM_DM;//SAFE_MASK SWIM_DM	
  
  cmdrtv=SWIM_WOTF(SWIM_CSR, 1, WriteBuff);//SWIM_CSR,控制寄存器中写入0B1010,0000
  
  
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 3;
  }	
  
  cmdrtv=SWIM_SRST();
  
  
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 2;
  }
  
  SWIM_RST_HIGH();
  
  WriteBuff[0]=SWIM_CSR_SAFE_MASK|SWIM_CSR_SWIM_DM|SWIM_CSR_PRI|SWIM_CSR_HSIT|SWIM_CSR_HS;	
  
  cmdrtv=SWIM_WOTF(SWIM_CSR, 1, WriteBuff);//SWIM_CSR,控制寄存器中写入0B1011,0000
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 4;
  }	
  
  SWIM_SetClockParam(8,10,2);	
  
  
  WriteBuff[0]=DM_CSR2_STALL;
  
  cmdrtv=SWIM_WOTF(DM_CSR2,1,WriteBuff);
  
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 5;
  }
  
  
  /******解锁eeprom和optionbyte保护***************/
  WriteBuff[0]=0xae;
  cmdrtv=SWIM_WOTF(STM8_FLASH_DUKR_ADDRESS[flash_control_reg_address],1,WriteBuff);
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 6;
  }
  
  WriteBuff[0]=0x56;	
  cmdrtv=SWIM_WOTF(STM8_FLASH_DUKR_ADDRESS[flash_control_reg_address],1,WriteBuff);
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 7;
  }
  
  /******解锁flash保护***************/
  WriteBuff[0]=0x56;				   //MASSKEY1
  cmdrtv=SWIM_WOTF(STM8_FLASH_PUKR_ADDRESS[flash_control_reg_address],1,WriteBuff);   //解锁Flash区域保护
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 6;
  }
  
  WriteBuff[0]=0xAE;			           //MASSKEY2
  
  cmdrtv=SWIM_WOTF(STM8_FLASH_PUKR_ADDRESS[flash_control_reg_address],1,WriteBuff);
  
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 7;
  }
  return 0;
}	




u8 End_Pro_Auto_1_SWIM_0(void)
{	
  
  
  switch(cJSON_GetObjectItem(root, "reset_type")->valueint)
  {
  case 0:
    WriteBuff[0]=0x00;
    break;
    
  case 1:
    WriteBuff[0]=0x00;
    break;
    
  case 2:
    WriteBuff[0]=0x08;
    break;
    
  }
  
  
  cmdrtv=SWIM_WOTF(DM_CSR2,1,WriteBuff);	  //DM_CSR2,Bit4 STALL =0
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 2;
  }
  
  
  
  
  
  while(1)
  {
    //System_Delay_ms(50);
    delay_ms(50);
    cmdrtv=SWIM_ROTF((u32)cJSON_GetObjectItem(root, "flash_base")->valuedouble,(u8)cJSON_GetObjectItem(root, "page_size")->valueint,ReadBuff);
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 1;
    } 
    
    
  }
}
u8 End_Pro_Auto_SWIM_0(void)
{	
  cmdrtv=Pro_Init_Auto_SWIM_0();
  while(1)
  {
    
    //System_Delay_ms(50);
    delay_ms(50);
    cmdrtv=Pro_Init_Auto_SWIM_0();
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 1;
    } 
    
    
  }
  return 0;
}

u8 End_Pro_SWIM_0(void)
{	
  
  u32 flash_base_address=0;
  flash_base_address=(u32)cJSON_GetObjectItem(root, "flash_base")->valuedouble;
  
  
  cmdrtv=SWIM_ROTF(flash_base_address,(u8)cJSON_GetObjectItem(root, "page_size")->valueint,ReadBuff);
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 1;
  } 
  //System_Delay_ms(20);
  delay_ms(20);
  
  switch(cJSON_GetObjectItem(root, "reset_type")->valueint)
  {
  case 0:
    WriteBuff[0]=0x00;
    break;
    
  case 1:
    WriteBuff[0]=0x00;
    break;
    
  case 2:
    WriteBuff[0]=0x08;
    break;
    
  }
  
  
  
  
  
  cmdrtv=SWIM_WOTF(DM_CSR2,1,WriteBuff);	  //DM_CSR2,Bit4 STALL =0
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 2;
  } 
SWIM_DeInit();
  
  
//SWIM_GPIO_Init();
  return 0;
}


extern char sp1[100];
extern u16 ImageNumber_store;




u8 ProgramFlash_SWIM_0(void)
{
  u32 dly=0;
  u16 i,blk=0;
  u32 aes_key=3000;
  u8 stm8_page_size=0;
  
  
  
  
  
  if((u8)cJSON_GetObjectItem(root, "mass_erase")->valueint == 1)
  {
    
  }
  else
  {
    if(EraseFlash())
      return 100;
  }
  
  
  
  
  sprintf(sp1,"0:/system/dat/DAT%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  
  
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 1;
  }
  
  
  
  u8 *Point=FileData;
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),5)->valuedouble;
    AES_Decrypt_Config_Init(16,aes_key);
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
  }
  
  
  stm8_page_size=(u8)cJSON_GetObjectItem(root, "page_size")->valueint;
  
  
  while(ReadCount)
  {
    Point=FileData;
    //数据足够整个page
    for(i=0;i<ReadCount/stm8_page_size;i++)
    {
      WriteBuff[0]=STM8_FLASH_CR2_VALUE[0]; //0000505B,Flash_CR2,Bit0=1,标准块编程方式
      WriteBuff[1]=~STM8_FLASH_CR2_VALUE[0];//0000505C,Flash_NCR2
      
      cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff); //每次写块数据之前都要重新配置Flash_CR2，NCR2,块写结束后被硬件自动清0
      
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 2;
      }
      
      cmdrtv=SWIM_WOTF((u32)cJSON_GetObjectItem(root, "flash_base")->valuedouble+(blk*stm8_page_size),stm8_page_size,Point);
      
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 3;
      }
      
      
      
      
      
      ///////////////////////////////////////////////开始校验FLASH     
      if((u8)cJSON_GetObjectItem(root, "verify_flash")->valueint == 1)
      {
        cmdrtv=SWIM_ROTF((u32)cJSON_GetObjectItem(root, "flash_base")->valuedouble+(blk*stm8_page_size),stm8_page_size,ReadBuff);
        
        
        if (cmdrtv>0)
        {
          SWIM_DeInit();
          return 2;
        }
        
        
        for(u8 i=0;i<stm8_page_size;i++)
        {
          
          if(*(Point+i)!=*(ReadBuff+i))
          {
            SWIM_DeInit();
            return 2;
          }
        }
      }
      ///////////////////////////////////////////校验结束    
      
      blk++;
      
      Point+=stm8_page_size;
      
      ReadBuff[0]=0;
      
      dly=100;
      while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
      {
        cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
        
        if(cmdrtv>0)
        {
          SWIM_DeInit();
          return 4;
        }
      }
      if(dly==0)
      {
        SWIM_DeInit();
        return 90;
      }
    }
    
    //最后剩余的数据不够一整个page
    if(ReadCount%stm8_page_size)
    {
      WriteBuff[0]=STM8_FLASH_CR2_VALUE[0]; //0000505B,Flash_CR2,Bit0=1,标准块编程方式
      WriteBuff[1]=~STM8_FLASH_CR2_VALUE[0];//0000505C,Flash_NCR2
      cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);		//每次写块数据之前都要重新配置Flash_CR2，NCR2,块写结束后被硬件自动清0
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 5;
      }
      cmdrtv=SWIM_WOTF((u32)cJSON_GetObjectItem(root, "flash_base")->valuedouble+(blk*stm8_page_size),ReadCount%stm8_page_size,Point);
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 6;
      }
      
      cmdrtv=SWIM_WOTF((u32)cJSON_GetObjectItem(root, "flash_base")->valuedouble+(blk*stm8_page_size)+ReadCount%stm8_page_size,stm8_page_size-ReadCount%stm8_page_size,fill0);
      blk++;
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 7;
      }
      
      
      ReadBuff[0]=0;
      dly=100;
      while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
      {
        cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
        if(cmdrtv>0)
        {
          SWIM_DeInit();
          return 8;
        }
      }
      if(dly==0)
      {
        SWIM_DeInit();
        return 90;
      }
    }
    
    
    if(ReadCount>=1024)
    {
      if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
      {
        return 9;
      }
      if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
      {
        return 10;
      }
      
      
      if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
      {
        Point = FileData;
        
        
        for (u32 j = 0; j < ReadCount; j += 16)
        {
          //解密数据包
          AES_Decrypt_Calculate(Point);
          Point += 16;
        }
      }
    }
    else
    {
      ReadCount=0;//强制退出循环，
    }
  }
  
  
  f_close(&FilSys);
  
  
  return 0;
}




u8 ProgramEEPROM_SWIM_0(void)
{
  u32 dly=0;
  u16 i,blk=0;
  u32 aes_key=3000;
  
  
  if((u8)cJSON_GetObjectItem(root, "mass_erase")->valueint == 1)
  {
    
  }
  else
  {
    if(EraseEEPROM())
      return 100;
  }
  
  
  sprintf(sp1,"0:/system/dat/EEP%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  
  
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 1;
  }
  
  
  u8 *Point=FileData;
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),5)->valuedouble;
    AES_Decrypt_Config_Init(16,aes_key);
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
  }
  
  
  while(ReadCount)
  {
    Point=FileData;
    
    for(i=0;i<ReadCount/(u8)cJSON_GetObjectItem(root, "page_size")->valueint;i++)
    {
      WriteBuff[0]=STM8_FLASH_CR2_VALUE[0]; //0000505B,Flash_CR2,Bit0=1,标准块编程方式
      WriteBuff[1]=~STM8_FLASH_CR2_VALUE[0];//0000505C,Flash_NCR2
      
      cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff); //每次写块数据之前都要重新配置Flash_CR2，NCR2,块写结束后被硬件自动清0
      
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 2;
      }
      
      cmdrtv=SWIM_WOTF((u32)cJSON_GetObjectItem(root, "eeprom_base")->valuedouble+(blk*(u8)cJSON_GetObjectItem(root, "page_size")->valueint),(u8)cJSON_GetObjectItem(root, "page_size")->valueint,Point);
      
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 3;
      }
      
      
      ///////////////////////////////////////////////开始校验EEPROM
      if((u8)cJSON_GetObjectItem(root, "verify_eeprom")->valueint == 1)
      {
        cmdrtv=SWIM_ROTF((u32)cJSON_GetObjectItem(root, "eeprom_base")->valuedouble+(blk*(u8)cJSON_GetObjectItem(root, "page_size")->valueint),(u8)cJSON_GetObjectItem(root, "page_size")->valueint,ReadBuff);
        
        
        if (cmdrtv>0)
        {
          SWIM_DeInit();
          return 2;
        }
        
        
        for(u8 i=0;i<(u8)cJSON_GetObjectItem(root, "page_size")->valueint;i++)
        {
          if(*(Point+i)!=*(ReadBuff+i))
          {
            SWIM_DeInit();
            return 2;
          }
        }
      }
      ///////////////////////////////////////////校验结束   
      
      
      blk++;
      
      Point+=(u8)cJSON_GetObjectItem(root, "page_size")->valueint;
      
      ReadBuff[0]=0;
      
      dly=100;
      while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
      {
        cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
        
        if(cmdrtv>0)
        {
          SWIM_DeInit();
          return 4;
        }
      }
      
      if(dly==0)
      {
        End_Pro_SWIM_0();
        return 90;
      }
      
    }
    
    
    if(ReadCount%(u8)cJSON_GetObjectItem(root, "page_size")->valueint)
    {
      WriteBuff[0]=STM8_FLASH_CR2_VALUE[0]; //0000505B,Flash_CR2,Bit0=1,标准块编程方式
      WriteBuff[1]=~STM8_FLASH_CR2_VALUE[0];//0000505C,Flash_NCR2
      cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);		//每次写块数据之前都要重新配置Flash_CR2，NCR2,块写结束后被硬件自动清0
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 5;
      }
      cmdrtv=SWIM_WOTF((u32)cJSON_GetObjectItem(root, "eeprom_base")->valuedouble+(blk*(u8)cJSON_GetObjectItem(root, "page_size")->valueint),ReadCount%(u8)cJSON_GetObjectItem(root, "page_size")->valueint,Point);
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 6;
      }
      
      cmdrtv=SWIM_WOTF((u32)cJSON_GetObjectItem(root, "eeprom_base")->valuedouble+(blk*(u8)cJSON_GetObjectItem(root, "page_size")->valueint)+ReadCount%(u8)cJSON_GetObjectItem(root, "page_size")->valueint,(u8)cJSON_GetObjectItem(root, "page_size")->valueint-ReadCount%(u8)cJSON_GetObjectItem(root, "page_size")->valueint,fill0);
      blk++;
      if (cmdrtv>0)
      {
        SWIM_DeInit();
        return 7;
      }
      
      
      ReadBuff[0]=0;
      
      dly=100;
      
      while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
      {
        cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
        if(cmdrtv>0)
        {
          SWIM_DeInit();
          return 8;
        }
      }
      
      if(dly==0)
      {
        End_Pro_SWIM_0();
        return 90;
      }
    }
    
    
    if(ReadCount>=1024)
    {
      if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
      {
        return 9;
      }
      if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
      {
        return 10;
      }
      
      if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
      {
        Point = FileData;
        
        
        for (u32 j = 0; j < ReadCount; j += 16)
        {
          //解密数据包
          AES_Decrypt_Calculate(Point);
          Point += 16;
        }
      }
      
    }
    else
    {
      ReadCount=0;//强制退出循环，
    }
  }
  
  
  f_close(&FilSys);
  
  
  return 0;
}


u16 Write_OpitonByte_STM8_pre_SWIM_0(void)
{
  u32 dly=0;
  u8 option_byte_length=0;
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 1;
  }
  
  
  option_byte_length=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),0)->valueint;
  
  switch(option_byte_length)
  {
    
    
  case 3:
    
    
    SendData[0]=0;
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    /*************使能读保护开始*********************/
    Pro_Init_SWIM_0();
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    End_Pro_SWIM_0();
    /*************使能读保护结束*********************/
    
    
    
    
    /*************清除读保护开始*********************/
    Pro_Init_SWIM_0();
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    
    Pro_Init_SWIM_0();//
    
    SendData[0]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),1)->valuedouble;
    SendData[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[2]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),3)->valuedouble;
    
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+8,1,SendData+2);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
  case 4:
    
    
    SendData[0]=0;
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    /*************使能读保护开始*********************/
    Pro_Init_SWIM_0();
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    End_Pro_SWIM_0();
    /*************使能读保护结束*********************/
    
    
    
    
    /*************清除读保护开始*********************/
    Pro_Init_SWIM_0();
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    
    Pro_Init_SWIM_0();//
    
    for(u8 i=0;i<4;i++)
    {
      SendData[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),i+1)->valuedouble;
    }
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+3,1,SendData+2);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+8,1,SendData+3);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
    
  case 6:
    
    /*************清除读保护开始*********************/
    SendData[0]=0;
    
    SendData[1]=0;
    SendData[2]= ~SendData[1];
    
    SendData[3]=0;
    SendData[4]=~SendData[3];
    
    SendData[5]=0;
    SendData[6]=~SendData[5];
    
    SendData[7]=0;
    SendData[8]=~SendData[7];
    
    SendData[9]=0;
    SendData[10]=~SendData[9];
    
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    End_Pro_SWIM_0();
    /*************清除读保护结束*********************/    
    
    
    /*************使能读保护开始*********************/
    Pro_Init_SWIM_0();
    SendData[0]=0xAA;
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，使能读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 91;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    /*************使能读保护结束*********************/   
    
    
    /*************清除读保护开始*********************/
    Pro_Init_SWIM_0();
    
    
    SendData[0]=0;
    
    SendData[1]=0;
    SendData[2]= ~SendData[1];
    
    SendData[3]=0;
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 92;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 93;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    /*************清除读保护结束*********************/
    
    
    
    SendData[0]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),1)->valuedouble;
    
    SendData[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[2]= ~SendData[1];
    
    SendData[3]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),3)->valuedouble;
    SendData[4]=~SendData[3];
    
    SendData[5]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),4)->valuedouble;
    SendData[6]=~SendData[5];
    
    SendData[7]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),5)->valuedouble;
    SendData[8]=~SendData[7];
    
    SendData[9]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),6)->valuedouble;
    SendData[10]=~SendData[9];
    
    Pro_Init_SWIM_0();
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 94;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    /*************清除读保护结束*********************/     
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 95;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+4,4,SendData+4);
    
    
    ReadBuff[0]=0;
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 96;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;					
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+8);
    
    ReadBuff[0]=0;
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 97;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
  case 7://选项字节长度为7
    
    SendData[0]=0;
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    /*************使能读保护开始*********************/
    Pro_Init_SWIM_0();
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    End_Pro_SWIM_0();
    /*************使能读保护结束*********************/
    
    
    
    
    /*************清除读保护开始*********************/
    Pro_Init_SWIM_0();
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    
    Pro_Init_SWIM_0();//
    
    
    for(u8 i=0;i<7;i++)
    {
      SendData[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),i+1)->valueint;
    }
    
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+2);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    cmdrtv=SWIM_WOTF(0x0000480C,1,SendData+6);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    
    break;
    
    
  case 8://选项字节长度为8 //STM8L051x3
    
    SendData[0]=0;
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    /*************使能读保护开始*********************/
    Pro_Init_SWIM_0();
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    End_Pro_SWIM_0();
    /*************使能读保护结束*********************/
    
    
    
    
    /*************清除读保护开始*********************/
    Pro_Init_SWIM_0();
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    
    /*************清除读保护结束*********************/ 
    
    
    
    
    
    Pro_Init_SWIM_0();//
    
    for(u8 i=0;i<8;i++)
    {
      SendData[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),i+1)->valuedouble;
    }
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+7,1,SendData+2);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+3);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    cmdrtv=SWIM_WOTF(0x0000480C,1,SendData+7);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    
    break; 
    
    
  case 9:
    /*************清除读保护开始*********************/
    SendData[0]=0;
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 0);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    End_Pro_SWIM_0();
    /*************清除读保护结束*********************/   
    
    
    
    /*************使能读保护开始*********************/
    SendData[0]=0xAA;
    Pro_Init_SWIM_0();//
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，使能读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 1);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    End_Pro_SWIM_0();
    /*************使能读保护结束*********************/
    
    
    
    /*************清除读保护开始*********************/
    SendData[0]=0;
    SendData[1]=0;
    SendData[2]= ~SendData[1];
    SendData[3]=0;
    
    Pro_Init_SWIM_0();//
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 0);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    /*************清除读保护结束*********************/  
    
    
    Pro_Init_SWIM_0();//
    
    
    SendData[0]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),1)->valuedouble;
    
    SendData[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[2]= ~SendData[1];
    
    SendData[3]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),3)->valuedouble;
    SendData[4]=~SendData[3];
    
    SendData[5]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),4)->valuedouble;
    SendData[6]=~SendData[5];
    
    SendData[7]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),5)->valuedouble;
    SendData[8]=~SendData[7];
    
    SendData[9]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),6)->valuedouble;
    SendData[10]=~SendData[9];
    
    SendData[11]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),7)->valuedouble;
    SendData[12]=~SendData[11];
    
    SendData[13]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),8)->valuedouble;
    SendData[14]=~SendData[13];
    
    SendData[15]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[16]=~SendData[15];
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    /*************清除读保护结束*********************/     
    
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+4,4,SendData+4);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;					
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+8);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+12,4,SendData+12);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    cmdrtv=SWIM_WOTF(0x0000487E,1,SendData+15);
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    cmdrtv=SWIM_WOTF(0x0000487F,1,SendData+16);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
    
  case 18://STM8AF6286 LQFP32
    /*************清除读保护开始*********************/
    SendData[0]=0;
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 0);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    End_Pro_SWIM_0();
    /*************清除读保护结束*********************/   
    
    
    
    /*************使能读保护开始*********************/
    SendData[0]=0xAA;
    Pro_Init_SWIM_0();//
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，使能读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 1);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    End_Pro_SWIM_0();
    /*************使能读保护结束*********************/
    
    
    
    /*************清除读保护开始*********************/
    SendData[0]=0;
    SendData[1]=0;
    SendData[2]= ~SendData[1];
    SendData[3]=0;
    
    Pro_Init_SWIM_0();//
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + 0);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    End_Pro_SWIM_0();
    /*************清除读保护结束*********************/  
    
    
    Pro_Init_SWIM_0();//
    
    
    SendData[0]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),1)->valuedouble;
    
    SendData[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[2]= ~SendData[1];
    
    SendData[3]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),3)->valuedouble;
    SendData[4]=~SendData[3];
    
    SendData[5]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),4)->valuedouble;
    SendData[6]=~SendData[5];
    
    SendData[7]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),5)->valuedouble;
    SendData[8]=~SendData[7];
    
    SendData[9]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),6)->valuedouble;
    SendData[10]=~SendData[9];
    
    SendData[11]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),7)->valuedouble;
    SendData[12]=~SendData[11];
    
    SendData[13]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),8)->valuedouble;
    SendData[14]=~SendData[13];
    
    SendData[15]=0;//保留为零
    
    
    
    
    
    
    ///////////////////以下为STM8AF6286的18个字节选项字节增加内容
    //这里比较特殊不做互补
    SendData[16]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),9)->valuedouble;
    SendData[17]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),10)->valuedouble;
    SendData[18]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),11)->valuedouble;
    SendData[19]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),12)->valuedouble;
    
    SendData[20]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),13)->valuedouble;
    SendData[21]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),14)->valuedouble;
    SendData[22]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),15)->valuedouble;
    SendData[23]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),16)->valuedouble;
    
    SendData[24]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),17)->valuedouble;
    
    
    
    
    SendData[25]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),18)->valuedouble;
    SendData[26]=~SendData[25];
    
    
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    /*************清除读保护结束*********************/     
    
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+4,4,SendData+4);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;					
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+8);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+12,4,SendData+12);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    
    
    
    
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+16,4,SendData+16);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }  
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+20,4,SendData+20);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }      
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+24,4,SendData+24);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }  
    
    
    
    cmdrtv=SWIM_WOTF(0x0000487E,1,SendData+25);
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    cmdrtv=SWIM_WOTF(0x0000487F,1,SendData+26);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;   
    
    
  default:
    SendData[0]=0x00;
    
    SendData[1]=0x00;
    SendData[2]=0xFF;
    SendData[3]=0x00;
    
    SendData[4]=0xfF;
    
    SendData[5]=0x00;
    SendData[6]=0xff;
    SendData[7]=0x00;
    
    SendData[8]=0xff;
    
    SendData[9]=0x00;
    SendData[10]=0xFF;
    SendData[11]=0x00;
    
    SendData[12]=0xFF;//480C
    
    
    SendData[13]=0x00;//480D
    SendData[14]=0xFF;//480E
    
    SendData[15]=0x00;//487E
    SendData[16]=0xFF;//487F
    break;
  }
  
  
  return 0;
}



u8 Write_OpitonByte_STM8_SWIM_0(void)
{
  u32 dly=0;
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 1;
  }
  
  switch((u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),0)->valuedouble)
  {
    
    
  case 3:
    
    for(u8 i=0;i<3;i++)
    {
      SendData[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),i+1)->valuedouble;
    }
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+8,1,SendData+2);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
  case 4:
    for(u8 i=0;i<4;i++)
    {
      SendData[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),i+1)->valuedouble;
    }
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+3,1,SendData+2);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+8,1,SendData+3);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
    
  case 6:
    
    SendData[0]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),1)->valuedouble;
    
    SendData[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[2]= ~SendData[1];
    
    SendData[3]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),3)->valuedouble;
    SendData[4]=~SendData[3];
    
    SendData[5]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),4)->valuedouble;
    SendData[6]=~SendData[5];
    
    SendData[7]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),5)->valuedouble;
    SendData[8]=~SendData[7];
    
    SendData[9]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),6)->valuedouble;
    SendData[10]=~SendData[9];
    
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 94;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    /*************清除读保护结束*********************/     
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 95;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+4,4,SendData+4);
    
    
    ReadBuff[0]=0;
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 96;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;					
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+8);
    
    ReadBuff[0]=0;
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 97;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
  case 7://选项字节长度为7
    
    for(u8 i=0;i<7;i++)
    {
      SendData[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),i+1)->valuedouble;
    }
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+2);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    cmdrtv=SWIM_WOTF(0x0000480C,1,SendData+6);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    
    break;
    
    
  case 8://选项字节长度为8 //STM8L051x3
    for(u8 i=0;i<8;i++)
    {
      SendData[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),i+1)->valuedouble;
    }
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    /*************清除读保护结束*********************/     
    
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+2,1,SendData+1);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //默认单字节编程。 byte Programming
    
    cmdrtv=SWIM_WOTF(0x00004800+7,1,SendData+2);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+3);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //默认单字节编程。 byte Programming
    cmdrtv=SWIM_WOTF(0x0000480C,1,SendData+7);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }	
    
    
    break; 
    
    
  case 9:
    SendData[0]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),1)->valuedouble;
    
    SendData[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[2]= ~SendData[1];
    
    SendData[3]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),3)->valuedouble;
    SendData[4]=~SendData[3];
    
    SendData[5]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),4)->valuedouble;
    SendData[6]=~SendData[5];
    
    SendData[7]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),5)->valuedouble;
    SendData[8]=~SendData[7];
    
    SendData[9]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),6)->valuedouble;
    SendData[10]=~SendData[9];
    
    SendData[11]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),7)->valuedouble;
    SendData[12]=~SendData[11];
    
    SendData[13]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),8)->valuedouble;
    SendData[14]=~SendData[13];
    
    SendData[15]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),9)->valuedouble;
    SendData[16]=~SendData[15];
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    /*************清除读保护结束*********************/     
    
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+4,4,SendData+4);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;					
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+8);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+12,4,SendData+12);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    cmdrtv=SWIM_WOTF(0x0000487E,1,SendData+15);
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    cmdrtv=SWIM_WOTF(0x0000487F,1,SendData+16);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;
    
    
    
  case 18://STM8AF6286 LQFP32
    
    SendData[0]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),1)->valuedouble;
    
    SendData[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),2)->valuedouble;
    SendData[2]= ~SendData[1];
    
    SendData[3]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),3)->valuedouble;
    SendData[4]=~SendData[3];
    
    SendData[5]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),4)->valuedouble;
    SendData[6]=~SendData[5];
    
    SendData[7]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),5)->valuedouble;
    SendData[8]=~SendData[7];
    
    SendData[9]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),6)->valuedouble;
    SendData[10]=~SendData[9];
    
    SendData[11]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),7)->valuedouble;
    SendData[12]=~SendData[11];
    
    SendData[13]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),8)->valuedouble;
    SendData[14]=~SendData[13];
    
    SendData[15]=0;//保留为零
    
    
    
    
    
    
    ///////////////////以下为STM8AF6286的18个字节选项字节增加内容
    //这里比较特殊不做互补
    SendData[16]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),9)->valuedouble;
    SendData[17]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),10)->valuedouble;
    SendData[18]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),11)->valuedouble;
    SendData[19]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),12)->valuedouble;
    
    SendData[20]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),13)->valuedouble;
    SendData[21]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),14)->valuedouble;
    SendData[22]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),15)->valuedouble;
    SendData[23]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),16)->valuedouble;
    
    SendData[24]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),17)->valuedouble;
    
    
    
    
    SendData[25]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "option_byte"),18)->valuedouble;
    SendData[26]=~SendData[25];
    
    
    
    
    //默认单字节编程，使能OPT
    //目的是先写读保护，清除读保护
    WriteBuff[0]=0x80;													
    WriteBuff[1]=0x7F;	
    
    
    /*************清除读保护开始*********************/
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,1,STM8_READ_OUT_PROTECTION + flash_control_reg_address);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    /*************清除读保护结束*********************/     
    
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    
    cmdrtv=SWIM_WOTF(0x00004800+0,4,SendData+0);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;													 
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+4,4,SendData+4);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;					
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+8,4,SendData+8);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+12,4,SendData+12);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    
    
    
    
    
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+16,4,SendData+16);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }  
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+20,4,SendData+20);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }      
    
    
    //切换到4字节，字编程。 Word Programming
    WriteBuff[0]=0xC0;													 
    WriteBuff[1]=0x3F;		
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);	
    
    cmdrtv=SWIM_WOTF(0x00004800+24,4,SendData+24);
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }  
    
    
    
    cmdrtv=SWIM_WOTF(0x0000487E,1,SendData+25);
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    cmdrtv=SWIM_WOTF(0x0000487F,1,SendData+26);
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
    
    
    
    break;   
    
    
  default:
    SendData[0]=0x00;
    
    SendData[1]=0x00;
    SendData[2]=0xFF;
    SendData[3]=0x00;
    
    SendData[4]=0xfF;
    
    SendData[5]=0x00;
    SendData[6]=0xff;
    SendData[7]=0x00;
    
    SendData[8]=0xff;
    
    SendData[9]=0x00;
    SendData[10]=0xFF;
    SendData[11]=0x00;
    
    SendData[12]=0xFF;//480C
    
    
    SendData[13]=0x00;//480D
    SendData[14]=0xFF;//480E
    
    SendData[15]=0x00;//487E
    SendData[16]=0xFF;//487F
    break;
  }
  
  
  return 0;
}






static u8 SWIM_HW_Out(u8 cmd, u8 bitlen, u16 retry_cnt)
{
  int8_t i, p;
  u32 dly;
  u32 *ptr = &SWIM_DMA_OUT_Buffer[0];
  
retry:
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ptr = &SWIM_DMA_OUT_Buffer[0];//作者：郭冬，2016年4月8号添加，关键代码，解决烧录失败以后触屏失灵问题，原因为指针越界。指针位置重新指向初始位置
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  SYNCSWPWM_IN_TIMER_RISE_DMA_INIT(bitlen + 3, SWIM_DMA_IN_Buffer);
  
  *ptr++ = SWIM_PULSE_0;
  
  p = 0;
  for (i = bitlen - 1; i>=0; i--)
  {
    if ((cmd >> i) & 1)
    {
      *ptr++ = SWIM_PULSE_1;
      p++;
    }
    else
    {
      *ptr++ = SWIM_PULSE_0;
    }
  }
  // parity bit
  if (p & 1)
  {
    *ptr++ = SWIM_PULSE_1;
  }
  else
  {
    *ptr++ = SWIM_PULSE_0;
  }
  // wait for last waveform -- parity bit
  *ptr++ = 0;
  SYNCSWPWM_OUT_TIMER_DMA_INIT(bitlen + 3, SWIM_DMA_OUT_Buffer);
  dly = SWIM_MAX_DLY;
  
  SYNCSWPWM_OUT_TIMER_DMA_WAIT(dly);
  
  if (!dly)
  {
    // timeout
    return 2;
  }
  
  
  dly = SWIM_MAX_DLY;
  delay_us(1);//20230829必须加延时否则反复烧写会出现失败情况，测试芯片为STM8L0511F3P6
  SYNCSWPWM_IN_TIMER_RISE_DMA_WAIT(dly);
  SYNCSWPWM_IN_TIMER_RISE_DMA_INIT(10, SWIM_DMA_IN_Buffer + 1);
  
  if (!dly)
  {
    // timeout
    return 2;
  }
  else if (SWIM_DMA_IN_Buffer[bitlen + 2] > SWIM_PULSE_Threshold)		 //判断最后一个ACK应答是否为"1",小于半个周期的低电平在前
  {
    // nack	
    if (retry_cnt)
    {
      retry_cnt--;
      goto retry;
    }
    else
    {
      return 1;
    }
  }
  else
  {
    return 0;
  }
}


static u8 SWIM_HW_In(u8* data, u8 bitlen)
{
  u8 ret = 0;
  u32 dly;
  
  dly = SWIM_MAX_DLY;
  SYNCSWPWM_IN_TIMER_RISE_DMA_WAIT(dly);									//先接收引导bit,目标到主机，这个位必须是1
  *data = 0;
  if (dly && (SWIM_DMA_IN_Buffer[1] < SWIM_PULSE_Threshold))				//如果=1,低电平时间小于4个脉冲
  {
    for (dly = 0; dly < 8; dly++)
    {
      if (SWIM_DMA_IN_Buffer[2 + dly] < SWIM_PULSE_Threshold)							
      {
        *data |= 1 << (7 - dly);
      }
    }
    SYNCSWPWM_IN_TIMER_RISE_DMA_INIT(11, SWIM_DMA_IN_Buffer);
    
    SWIM_DMA_OUT_Buffer[0] = SWIM_PULSE_1;
    SWIM_DMA_OUT_Buffer[1] = 0;
    SYNCSWPWM_OUT_TIMER_DMA_INIT(2, SWIM_DMA_OUT_Buffer);
    
    dly = SWIM_MAX_DLY;
    SYNCSWPWM_OUT_TIMER_DMA_WAIT(dly);
    
    if (!dly)
    {
      // timeout
      return 2;
    }
  }
  else
  {
    ret = 1;
  }
  
  return ret;
}


static u8 SWIM_WOTF(uint32_t addr, uint16_t len, u8 *data)
{
  uint16_t processed_len;
  u8 cur_len, i;
  uint32_t cur_addr, addr_tmp;
  u8 rtv2;
  
  if ((0 == len) || ((u8*)0 == data))
  {
    return 1;
  }
  
  processed_len = 0;
  cur_addr = addr;
  while (processed_len < len)
  {
    if ((len - processed_len) > 255)
    {
      cur_len = 255;
    }
    else
    {
      cur_len = len - processed_len;
    }
    
    SET_LE_U32(&addr_tmp, cur_addr);
    
    if(SWIM_HW_Out(SWIM_CMD_WOTF, SWIM_CMD_BITLEN, SWIM_MAX_RESEND_CNT))
    {
      return 1;
    }
    if (SWIM_HW_Out(cur_len, 8, 0))
    {
      return 2;
    }
    rtv2=SWIM_HW_Out((addr_tmp >> 16) & 0xFF, 8, 0);	 //retry=0,出错后不重发
    if (rtv2)
    {
      return 3;
    }
    if (SWIM_HW_Out((addr_tmp >> 8) & 0xFF, 8, 0))
    {
      return 4;
    }
    if (SWIM_HW_Out((addr_tmp >> 0) & 0xFF, 8, 0))
    {
      return 5;
    }
    for (i = 0; i < cur_len; i++)
    {
      if (SWIM_HW_Out(data[processed_len + i], 8, SWIM_MAX_RESEND_CNT))
      {
        return 6;
      }
    }
    
    cur_addr += cur_len;
    processed_len += cur_len;
  }
  
  return 0;
}


static u8 SWIM_ROTF(uint32_t addr, uint16_t len, u8 *data)
{
  uint16_t processed_len;
  u8 cur_len, i;
  uint32_t cur_addr, addr_tmp;
  
  if ((0 == len) || ((u8*)0 == data))
  {
    return 7;
  }
  
  processed_len = 0;
  cur_addr = addr;
  while (processed_len < len)
  {
    if ((len - processed_len) > 255)
    {
      cur_len = 255;
    }
    else
    {
      cur_len = len - processed_len;
    }
    
    SET_LE_U32(&addr_tmp, cur_addr);
    
    if(SWIM_HW_Out(SWIM_CMD_ROTF, SWIM_CMD_BITLEN, SWIM_MAX_RESEND_CNT))
    {
      return 6;
    }
    if (SWIM_HW_Out(cur_len, 8, 0))
    {
      return 5;
    }
    if (SWIM_HW_Out((addr_tmp >> 16) & 0xFF, 8, 0))
    {
      return 4;
    }
    if (SWIM_HW_Out((addr_tmp >> 8) & 0xFF, 8, 0))
    {
      return 3;
    }
    if (SWIM_HW_Out((addr_tmp >> 0) & 0xFF, 8, 0))
    {
      return 2;
    }
    for (i = 0; i < cur_len; i++)
    {
      if (SWIM_HW_In(&data[processed_len + i], 8))
      {
        return 1;
      }
    }
    
    cur_addr += cur_len;
    processed_len += cur_len;
  }
  return 0;
}



static u8 EraseFlash(void)
{
  u16 blk;
  u16 Range;
  u32 dly=0;
  
  SendData[0]=0x00;
  SendData[1]=0x00;
  SendData[2]=0x00;
  SendData[3]=0x00;
  
  Range=(u32)cJSON_GetObjectItem(root, "flash_size")->valuedouble/(u8)cJSON_GetObjectItem(root, "page_size")->valueint;
  
  
  for(blk=0;blk<Range;blk++)
  {
    WriteBuff[0]=0x20;													//0000505B,Flash_CR2 ,Bit0=1,标准块编程方式
    WriteBuff[1]=0xDF;													//0000505C,Flash_NCR2
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);		//每次写块数据之前都要重新配置Flash_CR2，NCR2,块写结束后被硬件自动清0
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 1;
    }
    
    cmdrtv=SWIM_WOTF((u32)cJSON_GetObjectItem(root, "flash_base")->valuedouble+(blk*(u8)cJSON_GetObjectItem(root, "page_size")->valueint),4,SendData);
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 2;
    }	
    
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 93;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
  }
  return 0;
}



static u8 EraseEEPROM(void)
{
  u16 blk;
  u16 Range;
  u32 dly=0;
  
  u32 eeprom_size=0;
  u32 eeprom_base_address=0;
  u8 page_size=0;
  
  eeprom_size=(u32)cJSON_GetObjectItem(root, "eeprom_size")->valuedouble;
  eeprom_base_address=(u32)cJSON_GetObjectItem(root, "eeprom_base")->valuedouble;
  page_size=(u8)cJSON_GetObjectItem(root, "page_size")->valueint;
  
  SendData[0]=0x00;
  SendData[1]=0x00;
  SendData[2]=0x00;
  SendData[3]=0x00;
  
  Range=eeprom_size/page_size;
  
  
  for(blk=0;blk<Range;blk++)
  {
    WriteBuff[0]=0x20;
    WriteBuff[1]=0xDF;
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff);
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 1;
    }
    
    
    cmdrtv=SWIM_WOTF(eeprom_base_address+(blk*(u8)cJSON_GetObjectItem(root, "page_size")->valueint),4,SendData);
    
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 2;
    }	
    
    
    ReadBuff[0]=0;
    dly=100;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP)&&--dly)
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 93;
      }
    }	
    
    if(dly==0)
    {
      End_Pro_SWIM_0();
      return 90;
    }
  }
  return 0;
}


u8 ClearChip_SWIM_0(void)
{
  if(EraseFlash())
    return 100;
  if(EraseEEPROM())
    return 101;
  
  return 0;
}



u8 WriteRollingCode_SWIM_0(void)
{
  u8 stm8_rolling_code_value_array[4]={0,0,0,0};
  u32 rolling_code_address=0;
  u8 rolling_code_width=0;
  
  if((u8)cJSON_GetObjectItem(root, "rolling_code_endian")->valueint==1)//大端模式
  {
    switch(cJSON_GetObjectItem(root, "rolling_code_byte_width")->valueint)
    {
    case 1:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      break;
    case 2:
      stm8_rolling_code_value_array[1]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[0]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      break;
    case 3:
      stm8_rolling_code_value_array[2]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[0]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      break;
    case 4:
      stm8_rolling_code_value_array[3]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[2]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      stm8_rolling_code_value_array[0]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>24; 
      break;
    }
  }
  else//小端模式
  {
    switch(cJSON_GetObjectItem(root, "rolling_code_byte_width")->valueint)
    {
    case 1:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      break;
    case 2:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      break;
    case 3:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[2]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      break;
    case 4:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[2]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      stm8_rolling_code_value_array[3]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>24; 
      break;
    }
  }
  
  rolling_code_address=((u32)cJSON_GetObjectItem(root, "rolling_code_address")->valuedouble);
  rolling_code_width=(u8)cJSON_GetObjectItem(root, "rolling_code_byte_width")->valueint; 
  
  
  
  
  for(u8 i=0;i<rolling_code_width;i++)//滚码字节数
  {
    
    
    WriteBuff[0]=0x80;//单字节编程编程
    WriteBuff[1]=0x7F;//单字节编程编程
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff); //每次写块数据之前都要重新配置Flash_CR2，NCR2,块写结束后被硬件自动清0
    
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 2;
    }
    
    cmdrtv=SWIM_WOTF(rolling_code_address+i, 1,&stm8_rolling_code_value_array[i]);
    
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 3;
    }
    
    ReadBuff[0]=0;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP))
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
  }
  
  
  return 0;
  
}



u8 WriteUniqueIDJiaMi_SWIM_0(void)
{
  
  u32 tmp;
  u8 JiaMiID[4];
  u8 UID_array_normal[12];//UID最大是12个字节
  
  //先读取unique id根据公式计算出加密结果-start
  cmdrtv=SWIM_ROTF((u32)cJSON_GetObjectItem(root, "uid_address")->valuedouble,12,ReadBuff);//到指定的位置读取unique id
  
  if (cmdrtv>0)
  {
    SWIM_DeInit();
    return 2;
  }
  
  
  
  for(u8 i=0;i<12;i++)
  {
    UID_array_normal[i]=ReadBuff[i];
  }
  
  for(u8 i=0;i<12;i++)
  {
    ReadBuff[i]=UID_array_normal[(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "custom_secret_dat"),i)->valuedouble];
  }
  
  
  
  
  
  
  tmp=JiaMiSuanFa_STM8((u8)cJSON_GetObjectItem(root, "custom_secret_gongshi")->valueint,((u32)cJSON_GetObjectItem(root, "custom_secret_changshu")->valuedouble),ReadBuff);//buf[123];//公式序号
  
  
  JiaMiID[0]=tmp;
  JiaMiID[1]=tmp>>8;
  JiaMiID[2]=tmp>>16;
  JiaMiID[3]=tmp>>24;
  //先读取unique id根据公式计算出加密结果-end
  
  
  
  //将加密结果按照字节逐个写入指定的地址-start
  
  for(u8 i=0;i<(u8)cJSON_GetObjectItem(root, "custom_secret_zijieshu")->valueint;i++)//自定义加密字节数
  {
    WriteBuff[0]=0x80;//单字节编程编程
    WriteBuff[1]=0x7F;//单字节编程编程
    
    cmdrtv=SWIM_WOTF(STM8_FLASH_CR2_ADDRESS[flash_control_reg_address],2,WriteBuff); //每次写块数据之前都要重新配置Flash_CR2，NCR2,块写结束后被硬件自动清0
    
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 2;
    }
    
    
    
    cmdrtv=SWIM_WOTF(((u32)cJSON_GetObjectItem(root, "custom_secret_address")->valuedouble)+i,1,&JiaMiID[i]);//自定义加密算法存储地址
    
    
    
    if (cmdrtv>0)
    {
      SWIM_DeInit();
      return 3;
    }
    
    ReadBuff[0]=0;
    
    while(!(ReadBuff[0]& STM8_FLASH_EOP))
    {
      cmdrtv=SWIM_ROTF(STM8_FLASH_IAPSR_ADDRESS[flash_control_reg_address],1,ReadBuff);
      if(cmdrtv>0)
      {
        SWIM_DeInit();
        return 9;
      }
    }	
    
    
  }
  
  //将加密结果按照字节逐个写入指定的地址-end
  
  return 0;
  
}








static u8 C[4];


static void Algorithm_STM8_00(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_01(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0]; 
  OutPut[1] = C[1] | D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] | D[3]; 
}

static void Algorithm_STM8_02(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] + D[1]; 
  OutPut[2] = C[2] + D[2]; 
  OutPut[3] = C[3] + D[3]; 
}

static void Algorithm_STM8_03(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] ^ D[1]; 
  OutPut[2] = C[2] ^ D[2]; 
  OutPut[3] = C[3] ^ D[3];
}

static void Algorithm_STM8_04(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] & D[2]; 
  OutPut[3] = C[3] & D[3]; 
}

static void Algorithm_STM8_05(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] - D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] - D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_STM8_06(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] * D[2]; 
  OutPut[3] = C[3] / D[3]; 
}

static void Algorithm_STM8_07(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_STM8_08(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_STM8_09(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] + D[5]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_STM8_10(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] ^ D[5]; 
  OutPut[2] = C[2] | D[2] ^ D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_STM8_11(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] + D[9]; 
  OutPut[2] = C[2] | D[2] ^ D[6] + D[10]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_STM8_12(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[2] | D[2] ^ D[6] ^ D[10]; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_13(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[4] + D[8]; 
  OutPut[1] = D[1] ^ D[5] | D[9]; 
  OutPut[2] = D[2] ^ D[6] ^ D[10]; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_14(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3]; 
  OutPut[1] = D[4] ^ D[5] | D[6]; 
  OutPut[2] = D[7] ^ D[8] ^ D[9]; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_STM8_15(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_16(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_17(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_18(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3];
}

static void Algorithm_STM8_19(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_20(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_21(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[0] | D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_22(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] | D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_STM8_23(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_24(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_25(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] & D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_26(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] ; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[3] ; 
}

static void Algorithm_STM8_27(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_28(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_29(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_30(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}



static void Algorithm_STM8_31(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}


static void Algorithm_STM8_32(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
  OutPut[1] = C[1] & D[0] & D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2] ^ D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
  OutPut[3] = C[3] & D[0] & D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_STM8_33(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] &  D[1] & D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] - D[1] & D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] & D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] & D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_34(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] & D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] & D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] & D[2] & D[3] ^ D[4] ^ D[5] ^ D[6] & D[7] & D[8] ^ D[9] ^ D[10] & D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] & D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_35(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] & D[4] & D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] & D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_36(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_STM8_37(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_STM8_38(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] & D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_STM8_39(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] | D[4]+ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_STM8_40(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]+ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_STM8_41(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4]+ D[5] | D[6] ^ D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_42(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_43(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[4] + D[8] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[1] = D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_44(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[1] = D[4] ^ D[5] | D[6]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_STM8_45(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_46(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] | D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_47(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[1] = C[1] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_48(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3];
}

static void Algorithm_STM8_49(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_50(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11];
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_51(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[0] | D[0] - D[1] & D[2] + D[3] - D[4] + D[5] + D[6] | D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_52(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] & D[1] ^ D[2] & D[3] & D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] | D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11]; 
}

static void Algorithm_STM8_53(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] & D[1] & D[2] & D[3] & D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_54(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] & D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_55(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] & D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11];
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_56(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] & D[2] & D[3] ^ D[4] & D[5] | D[6] & D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] ; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[3] ; 
}

static void Algorithm_STM8_57(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] & D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_58(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_59(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] & D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_60(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] & D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}


static void Algorithm_STM8_61(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] & D[5] ^ D[6] ^ D[7]; 
  OutPut[1] = C[1] | D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] | D[3]; 
}

static void Algorithm_STM8_62(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] + D[1]; 
  OutPut[2] = C[2] + D[2]; 
  OutPut[3] = C[3] + D[3]; 
}

static void Algorithm_STM8_63(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] ^ D[1]; 
  OutPut[2] = C[2] ^ D[2]; 
  OutPut[3] = C[3] ^ D[3];
}

static void Algorithm_STM8_64(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] & D[2]; 
  OutPut[3] = C[3] & D[3]; 
}

static void Algorithm_STM8_65(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] - D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] - D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_STM8_66(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] * D[2] ^ D[3] ^ D[4] + D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] ^ D[11]; 
  OutPut[3] = C[3] / D[3]; 
}

static void Algorithm_STM8_67(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] + D[3] & D[4] + D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_STM8_68(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[3] - D[4] + D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_STM8_69(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] ^ D[4]; 
  OutPut[1] = C[1] ^ D[1] + D[5]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7] ^ D[8] + D[9] + D[10] ^ D[11]; 
}

static void Algorithm_STM8_70(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] ^ D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] - D[11]; 
  OutPut[2] = C[2] | D[2] ^ D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_STM8_71(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] + D[9]; 
  OutPut[2] = C[2] | D[2] ^ D[6] + D[10]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_STM8_72(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11];
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_73(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11];
  OutPut[1] = D[1] ^ D[5] | D[9]; 
  OutPut[2] = D[2] ^ D[6] ^ D[10]; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_74(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3]; 
  OutPut[1] = D[4] ^ D[5] | D[6] + D[7] - D[8] + D[9]; 
  OutPut[2] = D[7] ^ D[8] ^ D[9]; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_STM8_75(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_76(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_77(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1] & D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_78(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3];
}

static void Algorithm_STM8_79(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_80(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_81(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_82(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6]; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11]; 
}

static void Algorithm_STM8_83(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_84(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] ; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_STM8_85(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] & D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_86(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[3] ; 
}

static void Algorithm_STM8_87(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] | D[1] ^ D[2] & D[3] | D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_88(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_89(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] - D[3] + D[4] + D[5] ^ D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1] + D[2] - D[3] + D[4] + D[5] + D[6] + D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_90(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] - D[1] ^ D[2] ^ D[3] ^ D[4] - D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] - D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
}


static void Algorithm_STM8_91(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_92(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1] - D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_93(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_94(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_95(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9]; 
  OutPut[3] = C[3] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
}

static void Algorithm_STM8_96(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] ; 
  OutPut[3] = C[3] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
}


static void Algorithm_STM8_97(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_98(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] + D[3] ^ D[4] ^ D[5] | D[6] ; 
}

static void Algorithm_STM8_99(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = D[0] + D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_100(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_101(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] ^ D[2] & D[3] ^ D[4]; 
  OutPut[1] = C[1] | D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] | D[3]; 
}

static void Algorithm_STM8_102(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] + D[2]; 
  OutPut[3] = C[3] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11];  
}

static void Algorithm_STM8_103(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] ^ D[2]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_STM8_104(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] ^ D[3];
}

static void Algorithm_STM8_105(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] - D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_STM8_106(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2] & D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] / D[3]; 
}

static void Algorithm_STM8_107(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2] & D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] % D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ;
}

static void Algorithm_STM8_108(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_STM8_109(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] + D[5]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_STM8_110(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_STM8_111(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4] + D[8]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_STM8_112(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_113(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[4] + D[8]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_STM8_114(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_STM8_115(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1] + D[3] ^ D[4] + D[5] + D[6]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_116(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1] + D[3] ^ D[4] + D[5] + D[6]; 
  OutPut[2] = C[2] + D[3] - D[4] ^ D[5] + D[6]; 
  OutPut[3] = C[3] ^ D[3] - D[4] + D[5] + D[6]; 
}

static void Algorithm_STM8_117(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] & D[5] + D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2] - D[4] + D[5]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_118(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] & D[5] + D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3] - D[7] + D[8];
}

static void Algorithm_STM8_119(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_120(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] - D[5] | D[6] - D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_121(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] ^ D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] - D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[0] | D[0] - D[1] - D[2] - D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] - D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_STM8_122(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] - D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] | D[0] + D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_STM8_123(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] | D[1] ^ D[2] - D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] & D[5] - D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_STM8_124(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] & D[1] - D[2] - D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] & D[6] ^ D[7] & D[8] ^ D[9] - D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_STM8_125(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] & D[5] | D[6] ^ D[7] & D[8] & D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_STM8_126(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] ; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_STM8_127(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] & D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] & D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}




static u32 JiaMiSuanFa_STM8(u8 gongshi,u32 changshu, u8 D[])
{
  
  u8 Result[4];
  
  
  
  
  C[0]=changshu;
  C[1]=changshu>>8;
  C[2]=changshu>>16;  
  C[3]=changshu>>24;
  
  
  switch(gongshi)
  {
  case 0:
    Algorithm_STM8_00(D,Result);
    
    break;
    
  case 1:
    Algorithm_STM8_01(D,Result);
    break;
    
  case 2:
    Algorithm_STM8_02(D,Result);
    break;
    
  case 3:
    Algorithm_STM8_03(D,Result);   
    break; 
    
  case 4:
    Algorithm_STM8_04(D,Result);  
    break;
    
  case 5:
    Algorithm_STM8_05(D,Result);
    break;
    
  case 6:
    Algorithm_STM8_06(D,Result);
    break;
    
  case 7:
    Algorithm_STM8_07(D,Result);
    break;
    
  case 8:
    Algorithm_STM8_08(D,Result);
    break;
    
  case 9:
    Algorithm_STM8_09(D,Result);
    break;
    
  case 10:
    Algorithm_STM8_10(D,Result);
    break; 
    
  case 11:
    Algorithm_STM8_11(D,Result);
    break;
    
  case 12:
    Algorithm_STM8_12(D,Result);
    break;
    
  case 13:
    Algorithm_STM8_13(D,Result);
    break;
    
  case 14:
    Algorithm_STM8_14(D,Result);
    break;
    
  case 15:
    Algorithm_STM8_15(D,Result);
    break;
    
  case 16:
    Algorithm_STM8_16(D,Result);
    break;
    
  case 17:
    Algorithm_STM8_17(D,Result);
    break; 
    
  case 18:
    Algorithm_STM8_18(D,Result);
    break;
    
  case 19:
    Algorithm_STM8_19(D,Result);
    break;
    
  case 20:
    Algorithm_STM8_20(D,Result);
    break;
    
  case 21:
    Algorithm_STM8_21(D,Result);
    break;
    
  case 22:
    Algorithm_STM8_22(D,Result);
    break;
    
  case 23:
    Algorithm_STM8_23(D,Result);   
    break; 
    
  case 24:
    Algorithm_STM8_24(D,Result);  
    break;
    
  case 25:
    Algorithm_STM8_25(D,Result);
    break;
    
  case 26:
    Algorithm_STM8_26(D,Result);
    break;
    
  case 27:
    Algorithm_STM8_27(D,Result);
    break;
    
  case 28:
    Algorithm_STM8_28(D,Result);
    break;
    
  case 29:
    Algorithm_STM8_29(D,Result);
    break;
    
  case 30:
    Algorithm_STM8_30(D,Result);
    break; 
    
  case 31:
    Algorithm_STM8_31(D,Result);
    break;
    
  case 32:
    Algorithm_STM8_32(D,Result);
    break;
    
  case 33:
    Algorithm_STM8_33(D,Result);
    break;
    
  case 34:
    Algorithm_STM8_34(D,Result);
    break;
    
  case 35:
    Algorithm_STM8_35(D,Result);
    break;
    
  case 36:
    Algorithm_STM8_36(D,Result);
    break;
    
  case 37:
    Algorithm_STM8_37(D,Result);
    break; 
    
  case 38:
    Algorithm_STM8_38(D,Result);
    break;
    
  case 39:
    Algorithm_STM8_39(D,Result);
    break;
    
  case 40:
    Algorithm_STM8_40(D,Result);
    break;
    
  case 41:
    Algorithm_STM8_41(D,Result);
    break;
    
  case 42:
    Algorithm_STM8_42(D,Result);
    break;
    
  case 43:
    Algorithm_STM8_43(D,Result);   
    break; 
    
  case 44:
    Algorithm_STM8_44(D,Result);  
    break;
    
  case 45:
    Algorithm_STM8_45(D,Result);
    break;
    
  case 46:
    Algorithm_STM8_46(D,Result);
    break;
    
  case 47:
    Algorithm_STM8_47(D,Result);
    break;
    
  case 48:
    Algorithm_STM8_48(D,Result);
    break;
    
  case 49:
    Algorithm_STM8_49(D,Result);
    break;
    
  case 50:
    Algorithm_STM8_50(D,Result);
    break; 
    
  case 51:
    Algorithm_STM8_51(D,Result);
    break;
    
  case 52:
    Algorithm_STM8_52(D,Result);
    break;
    
  case 53:
    Algorithm_STM8_53(D,Result);
    break;
    
  case 54:
    Algorithm_STM8_54(D,Result);
    break;
    
  case 55:
    Algorithm_STM8_55(D,Result);
    break;
    
  case 56:
    Algorithm_STM8_56(D,Result);
    break;
    
  case 57:
    Algorithm_STM8_57(D,Result);
    break; 
    
  case 58:
    Algorithm_STM8_58(D,Result);
    break;
    
  case 59:
    Algorithm_STM8_59(D,Result);
    break;
    
  case 60:
    Algorithm_STM8_60(D,Result);
    break;
    
  case 61:
    Algorithm_STM8_61(D,Result);
    break;
    
  case 62:
    Algorithm_STM8_62(D,Result);
    break;
    
  case 63:
    Algorithm_STM8_63(D,Result);   
    break; 
    
  case 64:
    Algorithm_STM8_64(D,Result);  
    break;
    
  case 65:
    Algorithm_STM8_65(D,Result);
    break;
    
  case 66:
    Algorithm_STM8_66(D,Result);
    break;
    
  case 67:
    Algorithm_STM8_67(D,Result);
    break;
    
  case 68:
    Algorithm_STM8_68(D,Result);
    break;
    
  case 69:
    Algorithm_STM8_69(D,Result);
    break;
    
  case 70:
    Algorithm_STM8_70(D,Result);
    break; 
    
  case 71:
    Algorithm_STM8_71(D,Result);
    break;
    
  case 72:
    Algorithm_STM8_72(D,Result);
    break;
    
  case 73:
    Algorithm_STM8_73(D,Result);
    break;
    
  case 74:
    Algorithm_STM8_74(D,Result);
    break;
    
  case 75:
    Algorithm_STM8_75(D,Result);
    break;
    
  case 76:
    Algorithm_STM8_76(D,Result);
    break;
    
  case 77:
    Algorithm_STM8_77(D,Result);
    break; 
    
  case 78:
    Algorithm_STM8_78(D,Result);
    break;
    
  case 79:
    Algorithm_STM8_79(D,Result);
    break;
    
  case 80:
    Algorithm_STM8_80(D,Result);
    break;
    
  case 81:
    Algorithm_STM8_81(D,Result);
    break;
    
  case 82:
    Algorithm_STM8_82(D,Result);
    break;
    
  case 83:
    Algorithm_STM8_83(D,Result);   
    break; 
    
  case 84:
    Algorithm_STM8_84(D,Result);  
    break;
    
  case 85:
    Algorithm_STM8_85(D,Result);
    break;
    
  case 86:
    Algorithm_STM8_86(D,Result);
    break;
    
  case 87:
    Algorithm_STM8_87(D,Result);
    break;
    
  case 88:
    Algorithm_STM8_88(D,Result);
    break;
    
  case 89:
    Algorithm_STM8_89(D,Result);
    break;
    
  case 90:
    Algorithm_STM8_90(D,Result);
    break; 
    
  case 91:
    Algorithm_STM8_91(D,Result);
    break;
    
  case 92:
    Algorithm_STM8_92(D,Result);
    break;
    
  case 93:
    Algorithm_STM8_93(D,Result);
    break;
    
  case 94:
    Algorithm_STM8_94(D,Result);
    break;
    
  case 95:
    Algorithm_STM8_95(D,Result);
    break;
    
  case 96:
    Algorithm_STM8_96(D,Result);
    break;
    
  case 97:
    Algorithm_STM8_97(D,Result);
    break; 
    
  case 98:
    Algorithm_STM8_98(D,Result);
    break;
    
  case 99:
    Algorithm_STM8_99(D,Result);
    break;
    
  case 100:
    Algorithm_STM8_100(D,Result);
    break;
    
  case 101:
    Algorithm_STM8_101(D,Result);
    break;
    
  case 102:
    Algorithm_STM8_102(D,Result);
    break;
    
  case 103:
    Algorithm_STM8_103(D,Result);   
    break; 
    
  case 104:
    Algorithm_STM8_104(D,Result);  
    break;
    
  case 105:
    Algorithm_STM8_105(D,Result);
    break;
    
  case 106:
    Algorithm_STM8_106(D,Result);
    break;
    
  case 107:
    Algorithm_STM8_107(D,Result);
    break;
    
  case 108:
    Algorithm_STM8_108(D,Result);
    break;
    
  case 109:
    Algorithm_STM8_109(D,Result);
    break;
    
  case 110:
    Algorithm_STM8_110(D,Result);
    break; 
    
  case 111:
    Algorithm_STM8_111(D,Result);
    break;
    
  case 112:
    Algorithm_STM8_112(D,Result);
    break;
    
  case 113:
    Algorithm_STM8_113(D,Result);
    break;
    
  case 114:
    Algorithm_STM8_114(D,Result);
    break;
    
  case 115:
    Algorithm_STM8_115(D,Result);
    break;
    
  case 116:
    Algorithm_STM8_116(D,Result);
    break;
    
  case 117:
    Algorithm_STM8_117(D,Result);
    break; 
    
  case 118:
    Algorithm_STM8_118(D,Result);
    break;
    
  case 119:
    Algorithm_STM8_119(D,Result);
    break;
    
  case 120:
    Algorithm_STM8_120(D,Result);
    break;
  case 121:
    Algorithm_STM8_121(D,Result);
    break;
    
  case 122:
    Algorithm_STM8_122(D,Result);
    break;
    
  case 123:
    Algorithm_STM8_123(D,Result);
    break;
    
  case 124:
    Algorithm_STM8_124(D,Result);
    break;
    
  case 125:
    Algorithm_STM8_125(D,Result);
    break;
    
  case 126:
    Algorithm_STM8_126(D,Result);
    break;
    
  case 127:
    Algorithm_STM8_127(D,Result);
    break; 
  }
  
  return (Result[3]<<24|Result[2]<<16|Result[1]<<8|Result[0]);
}


#include "config.h"
#include "main.h"
//#include "usb_pwr.h"
#include "usb_dcd.h"

#include "usbd_usr.h"
#include "usbd_desc_msc.h"
#include "usbd_msc_core.h"

#include "includes.h"
extern USB_OTG_CORE_HANDLE USB_OTG_dev;
extern USB_OTG_CORE_HANDLE USB_OTG_dev_winusb;
extern USB_OTG_CORE_HANDLE USB_OTG_dev_msc;

//#define VUSB_PIN GPIO_Pin_5
//#define VUSB_GPIO GPIOC
//#define VUSB_ADC_CHANNEL        ADC_Channel_15
//
//#define VREF_PIN GPIO_Pin_0
//#define VREF_GPIO GPIOB
//#define VREF_ADC_CHANNEL        ADC_Channel_8
//
//#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//
//u16 ADCConvertedValue[2];
//u16 ADCConvertedValueAverage[2];
//u32 ADCConvertedValueTotal[2];
//float ADCConvertedValueTemp[2];
//const u8 boot[]=BOOTLOAD_FILE;
//#define countof(a) (sizeof(a) / sizeof(*(a)))
//u32 boot_loader_file_size=countof(boot);
void GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  
  
  /* GPIOA and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOD , ENABLE);
  /* GPIOA and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);//SWD_1
  /*SW_5V_PIN_0、SW_3V3_PIN_0*/
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);//TFT_LIGHT
  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  
  SW_5V_OFF_0;
  SW_3V3_OFF_0;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  /*SW_5V_PIN_1、SW_3V3_PIN_1*/
  
  SW_5V_OFF_1;
  SW_3V3_OFF_1;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  SW_3V3_ON_0;
  SW_3V3_ON_1;
  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  
  //  SW_5V_ON_0;
  //  SW_5V_ON_1;
  
  
  
  //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB|  RCC_APB2Periph_GPIOC| RCC_APB2Periph_GPIOE| RCC_APB2Periph_AFIO, ENABLE);
  //  
  //  
  //  GPIO_InitStructure.GPIO_Pin = LED_GREEN_PIN;
  //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 
  //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	 
  //  
  //  GPIO_Init(LED_GREEN_GPIO, &GPIO_InitStructure);	 
  //  
  //  
  //  GPIO_InitStructure.GPIO_Pin = LED_RED_PIN;
  //  GPIO_Init(LED_RED_GPIO, &GPIO_InitStructure);
  //  
  //  GPIO_WriteBit(LED_RED_GPIO, LED_RED_PIN, (BitAction)!GPIO_ReadOutputDataBit(LED_RED_GPIO, LED_RED_PIN));
  //  
  //  /*机台信号引脚*/
  //  GPIO_InitStructure.GPIO_Pin = JITAI_FAULT_PIN;
  //  GPIO_Init(JITAI_FAULT_GPIO, &GPIO_InitStructure);
  //  JITAI_FAULT_HIGH;
  //  
  //  
  //  GPIO_InitStructure.GPIO_Pin = JITAI_OK_PIN;
  //  GPIO_Init(JITAI_OK_GPIO, &GPIO_InitStructure);
  //  JITAI_OK_HIGH;
  //  
  //  GPIO_InitStructure.GPIO_Pin = JITAI_OK_AUTO_PIN;
  //  GPIO_Init(JITAI_OK_AUTO_GPIO, &GPIO_InitStructure);
  //  JITAI_OK_AUTO_LOW;
  //  
  //  
  //  GPIO_InitStructure.GPIO_Pin = JITAI_IDLE_PIN;
  //  GPIO_Init(JITAI_IDLE_GPIO, &GPIO_InitStructure);
  //  
  //  JITAI_IDLE_HIGH;
  //  
  //  GPIO_InitStructure.GPIO_Pin = SW_3V3_PIN;
  //  GPIO_Init(SW_3V3_GPIO, &GPIO_InitStructure);
  //  
  //  
  //  SW_5V_OFF;
  //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  //  GPIO_InitStructure.GPIO_Pin = SW_5V_PIN;
  //  GPIO_Init(SW_5V_GPIO, &GPIO_InitStructure);
  //  
  //  SW_3V3_OFF;
  //  
  //  
  //  GPIO_InitStructure.GPIO_Pin = KEY_SW1_PIN;
  //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
  //  
  //  GPIO_Init(KEY_SW1_GPIO, &GPIO_InitStructure);	 
  //  
  //  
  //  GPIO_InitStructure.GPIO_Pin = START_PE0_PIN;
  //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
  //  
  //  GPIO_Init(START_PE0_GPIO, &GPIO_InitStructure);	
  //  
  //  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);、
  
  //  void Delay_Timer_Init(void);
  //  Delay_Timer_Init();
}


void TIM1_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  
  
  uint16_t PrescalerValue = 0;
  
  
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  
  /* GPIOA and GPIOB clock enable */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
  
  //蜂鸣器
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);
  
  
  /* -----------------------------------------------------------------------
  TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
  The TIM3CLK frequency is set to SystemCoreClock (Hz), to get TIM3 counter
  clock at 24 MHz the Prescaler is computed as following:
  - Prescaler = (TIM3CLK / TIM3 counter clock) - 1
  SystemCoreClock is set to 72 MHz for Low-density, Medium-density, High-density
  and Connectivity line devices and to 24 MHz for Low-Density Value line and
  Medium-Density Value line devices
  
  The TIM3 is running at 36 KHz: TIM3 Frequency = TIM3 counter clock/(ARR + 1)
  = 24 MHz / 666 = 36 KHz
  TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR)* 100 = 50%
  TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 = 37.5%
  TIM3 Channel3 duty cycle = (TIM3_CCR3/ TIM3_ARR)* 100 = 25%
  TIM3 Channel4 duty cycle = (TIM3_CCR4/ TIM3_ARR)* 100 = 12.5%
  ----------------------------------------------------------------------- */
  /* Compute the prescaler value */
  //= 2 MHz / 1000 = 2 KHz
  PrescalerValue = (uint16_t) (SystemCoreClock/2000000) - 1;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 1000;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  
  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 1001;//蜂鸣器
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
  
  TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
  
  
  TIM_ARRPreloadConfig(TIM1, ENABLE);
  
  /* TIM1 enable counter */
  TIM_Cmd(TIM1, ENABLE);
  
  
  TIM_CtrlPWMOutputs(TIM1, ENABLE);//使能TIM1的PWM输出，TIM1与TIM8有
  
}


/**
* @brief  Configures the USART Peripheral.
* @param  None
* @retval None
*/
void USART_Config(void)
{
  
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = EVAL_IRQn[COM_X];
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  
  
  USART_InitTypeDef USART_InitStructure;
  
  /* USARTx configuration ------------------------------------------------------*/
  /* USARTx configured as follows:
  - BaudRate = 9600 baud  
  - Word Length = 8 Bits
  - Two Stop Bit
  - Odd parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  STM_EVAL_COMInit(COM_X, &USART_InitStructure);
  
  
  
  /* Enable the EVAL_COM1 Transmit interrupt: this interrupt is generated when the 
  EVAL_COM1 transmit data register is empty */  
  USART_ITConfig(COM_USART[COM_X], USART_IT_TXE, ENABLE);
  
  
  /* Enable the EVAL_COM1 Receive interrupt: this interrupt is generated when the 
  EVAL_COM1 receive data register is not empty */
  USART_ITConfig(COM_USART[COM_X], USART_IT_RXNE, ENABLE);
  
  
}



void TIM6_Config(void)
{
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  uint16_t PrescalerValue = 0;
  
  
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  TIM_DeInit(TIM6);
  
  /* -----------------------------------------------------------------------
  TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
  The TIM3CLK frequency is set to SystemCoreClock (Hz), to get TIM3 counter
  clock at 24 MHz the Prescaler is computed as following:
  - Prescaler = (TIM3CLK / TIM3 counter clock) - 1
  SystemCoreClock is set to 72 MHz for Low-density, Medium-density, High-density
  and Connectivity line devices and to 24 MHz for Low-Density Value line and
  Medium-Density Value line devices
  
  The TIM3 is running at 36 KHz: TIM3 Frequency = TIM3 counter clock/(ARR + 1)
  = 24 MHz / 666 = 36 KHz
  TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR)* 100 = 50%
  TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 = 37.5%
  TIM3 Channel3 duty cycle = (TIM3_CCR3/ TIM3_ARR)* 100 = 25%
  TIM3 Channel4 duty cycle = (TIM3_CCR4/ TIM3_ARR)* 100 = 12.5%
  ----------------------------------------------------------------------- */
  /* Compute the prescaler value */
  
  /*
  TIM6?妞?闂囧牐鍔?閻?閹?椤?閾?鐫????閾??????閹??MHz
  閻???濠�??閸?閻姊惧ú鈺冩А濡楀拋顢????閻熸洘浼??????6M閹?
  10閹插秷娲?閾?????閹叉帗鏋?閹憋附鐤嗙潰??椤晜鍟?0000閹?
  */
  PrescalerValue = (uint16_t) (SystemCoreClock / 6000000) - 1;
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 3000;//500us中断
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  
  
  TIM_ARRPreloadConfig(TIM6, ENABLE);
  TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  /* TIM3 enable counter */
  TIM_Cmd(TIM6, ENABLE);
}
//
//
//
//void TIM7_Config(void)
//{
//  
//  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//  uint16_t PrescalerValue = 0;
//  
//  
//  
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
//  TIM_DeInit(TIM7);
//  
//  /* -----------------------------------------------------------------------
//  TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
//  The TIM3CLK frequency is set to SystemCoreClock (Hz), to get TIM3 counter
//  clock at 24 MHz the Prescaler is computed as following:
//  - Prescaler = (TIM3CLK / TIM3 counter clock) - 1
//  SystemCoreClock is set to 72 MHz for Low-density, Medium-density, High-density
//  and Connectivity line devices and to 24 MHz for Low-Density Value line and
//  Medium-Density Value line devices
//  
//  The TIM3 is running at 36 KHz: TIM3 Frequency = TIM3 counter clock/(ARR + 1)
//  = 24 MHz / 666 = 36 KHz
//  TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR)* 100 = 50%
//  TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 = 37.5%
//  TIM3 Channel3 duty cycle = (TIM3_CCR3/ TIM3_ARR)* 100 = 25%
//  TIM3 Channel4 duty cycle = (TIM3_CCR4/ TIM3_ARR)* 100 = 12.5%
//  ----------------------------------------------------------------------- */
//  /* Compute the prescaler value */
//  
//  /*
//  TIM6?妞?闂囧牐鍔?閻?閹?椤?閾?鐫????閾??????閹??MHz
//  閻???濠�??閸?閻姊惧ú鈺冩А濡楀拋顢????閻熸洘浼??????6M閹?
//  10閹插秷娲?閾?????閹叉帗鏋?閹憋附鐤嗙潰??椤晜鍟?0000閹?
//  */
//  PrescalerValue = (uint16_t) (SystemCoreClock / 60000) - 1;
//  /* Time base configuration */
//  TIM_TimeBaseStructure.TIM_Period = 60000;//?妞?闂囧牐鍔?閻?閹?0000閹插秷鎮?閻????妤傛ǹ娼??0閹插秷娲?閾??????
//  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
//  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//  
//  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
//  
//  
//  TIM_ARRPreloadConfig(TIM7, ENABLE);
//  
//  
//  TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
//  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
//  
//  /* TIM3 enable counter */
//  TIM_Cmd(TIM7, ENABLE);
//  
//}
//
//
//
//void ADC_DMA_Config(void)
//{
//  GPIO_InitTypeDef GPIO_InitStructure;
//  ADC_InitTypeDef ADC_InitStructure;
//  //DMA_InitTypeDef DMA_InitStructure;
//  
//  
//  RCC_ADCCLKConfig(RCC_PCLK2_Div4);
//  
//  //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
////  GPIO_InitStructure.GPIO_Pin = VUSB_PIN;//PC5 ADC12_IN15
// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
////  GPIO_Init(VUSB_GPIO, &GPIO_InitStructure);
//  
//  GPIO_InitStructure.GPIO_Pin = VREF_PIN;//PB0 ADC12_IN8
//  GPIO_Init(VREF_GPIO, &GPIO_InitStructure);
//  
//  
////  /* DMA1 channel1 configuration ----------------------------------------------*/
////  DMA_DeInit(DMA1_Channel1);
////  
////  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
////  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCConvertedValue;
////  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
////  DMA_InitStructure.DMA_BufferSize = 2;
////  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
////  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
////  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
////  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//半“字”即两个“字节”，即16位
////  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
////  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
////  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
////  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
////  
////  
////  /* Enable DMA1 channel1 */
////  DMA_Cmd(DMA1_Channel1, ENABLE);
//  
//  /* ADC1 configuration ------------------------------------------------------*/
//  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
//  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
//  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//  ADC_InitStructure.ADC_NbrOfChannel = 1;
//  ADC_Init(ADC1, &ADC_InitStructure);
//  
//  /* ADC1 regular channel14 configuration */ 
// // ADC_RegularChannelConfig(ADC1, VUSB_ADC_CHANNEL, 1, ADC_SampleTime_55Cycles5);
//  ADC_RegularChannelConfig(ADC1, VREF_ADC_CHANNEL, 1, ADC_SampleTime_55Cycles5);
//  
//  
//  /* Enable ADC1 DMA */
//  //ADC_DMACmd(ADC1, ENABLE);
//  
//  /* Enable ADC1 */
//  ADC_Cmd(ADC1, ENABLE);
//  
//  /* Enable ADC1 reset calibration register */   
//  ADC_ResetCalibration(ADC1);
//  /* Check the end of ADC1 reset calibration register */
//  while(ADC_GetResetCalibrationStatus(ADC1));
//  
//  /* Start ADC1 calibration */
//  ADC_StartCalibration(ADC1);
//  /* Check the end of ADC1 calibration */
//  while(ADC_GetCalibrationStatus(ADC1));
//  
//  /* Start ADC1 Software Conversion */ 
//  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//}
//
//
void NVIC_Configuration(void)
{
  //NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure the NVIC Preemption Priority Bits */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  //  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  //  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  //  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //  NVIC_Init(&NVIC_InitStructure);
  //  
  //  
  //  // DMA2 STREAMx Interrupt ENABLE
  //  NVIC_InitStructure.NVIC_IRQChannel = SD_SDIO_DMA_IRQn;
  //  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  //  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //  NVIC_Init(&NVIC_InitStructure);
  //  
  
  
  //  
  //  
  //    NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
  //    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  //    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //    NVIC_Init(&NVIC_InitStructure);
  //  
  //  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  //  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  //  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
  //  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //  NVIC_Init(&NVIC_InitStructure);
  //  
  //  
  //  
  //  /* Enable the USB interrupt */
  //  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  //  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  //  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //  NVIC_Init(&NVIC_InitStructure);
}

void u_dp_dis(FunctionalState IsUsbEnable)
{
  if(IsUsbEnable!=ENABLE)
  {
    //    NVIC_InitTypeDef NVIC_InitStructure;
    //    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //    NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
    //    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    //    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    //    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    //    NVIC_Init(&NVIC_InitStructure);
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    DCD_DevDisconnect (&USB_OTG_dev_winusb);
    CPU_CRITICAL_EXIT();
  }
  else
  {
    //    NVIC_InitTypeDef NVIC_InitStructure;
    //    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    //    NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
    //    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    //    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    //    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //    NVIC_Init(&NVIC_InitStructure);
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    USB_OTG_dev=USB_OTG_dev_winusb;
    DCD_DevConnect (&USB_OTG_dev_winusb);
    CPU_CRITICAL_EXIT();
  }
}

void u_dp_dis_MASS(FunctionalState IsUsbEnable)
{
  if(IsUsbEnable!=ENABLE)
  {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    DCD_DevDisconnect (&USB_OTG_dev_msc);
    CPU_CRITICAL_EXIT();
  }
  else
  {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    USB_OTG_dev=USB_OTG_dev_msc;
    DCD_DevConnect (&USB_OTG_dev_msc);
    CPU_CRITICAL_EXIT();
  }
  
}
//
//u8 isp_result = 0;
//unsigned char rxbuf[300]={0};
//unsigned char code[1204]={0x7F,0x69,0x56,[1022]=0x74,[1023]=0x33};
////#define USART3_DR_Address USART3->DR
//#define USART3_DR_Address 0x40004804
//void USART3_Config(void)
//{
//  GPIO_InitTypeDef  GPIO_InitStructure;
//  USART_InitTypeDef USART_InitStructure;
//  DMA_InitTypeDef  DMA_InitStructure;
//  NVIC_InitTypeDef NVIC_InitStructure;
//  
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//  
//  USART_InitStructure.USART_BaudRate = 115200;
//  USART_InitStructure.USART_WordLength = USART_WordLength_9b;
//  USART_InitStructure.USART_StopBits = USART_StopBits_1;
//  USART_InitStructure.USART_Parity = USART_Parity_Even;
//  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//  
//  /* USART configuration */
//  USART_Init(USART3, &USART_InitStructure);
//  
//  /* Configure USART Tx as alternate function push-pull */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
//  
//  
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
//  
//  
//  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
//  
//  /* Enable USART */
//  USART_Cmd(USART3, ENABLE);
//  
//  
//  /*------------------------------- DMA---------------------------------------*/   
//  /* Common DMA configuration */
//  DMA_InitStructure.DMA_BufferSize = 1;
//  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//  
//  
//  //串口3发送TX通道DMA
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)code;
//  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
//  DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Address;
//  DMA_Init(DMA1_Channel2, &DMA_InitStructure);
//  
//  //串口3接收通道DMA
//  DMA_InitStructure.DMA_BufferSize = 300;
//  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rxbuf;
//  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
//  DMA_InitStructure.DMA_PeripheralBaseAddr = USART3_DR_Address;
//  DMA_Init(DMA1_Channel3, &DMA_InitStructure);
//  
//  
//  
//  /* Enable the USART1 Rx and Tx DMA2 requests */
//  USART_DMACmd(USART3, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
//  
//  
//  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//  
//}
//
//
//
//void USART3_Config_DISABLE(void)
//{
//  NVIC_InitTypeDef NVIC_InitStructure;
//  
//  USART_DeInit(USART3);
//  USART_Cmd(USART3, DISABLE);
//
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
//  
//
//  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
//  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//  NVIC_Init(&NVIC_InitStructure);
//}
//
//void TIM7_Config_DISABLE(void)
//{
//  
//  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//
//  
//  TIM_DeInit(TIM7);
//  
//  
//  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
//
//  TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
//  
//  /* TIM3 enable counter */
//  TIM_Cmd(TIM7, DISABLE);
//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, DISABLE);
//}




void Delay_Timer_Init(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
  TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Down;
  TIM_TimeBaseInitStruct.TIM_Period = 2000;
  TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock / 1000000) - 1;
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStruct);
  
  while((TIM5->SR & TIM_FLAG_Update)!=SET);
  TIM5->SR = (uint16_t)~TIM_FLAG_Update;
}


void System_Delay_ms(u16 nms)
{	 		  	  
  //  u32 temp;	
  //  SysTick->LOAD=(u32)nms*9000;			//时间加载(SysTick->LOAD为24bit)
  //  SysTick->VAL   =  (0x00);                     /* Load the SysTick Counter Value */         //清空计数器
  //  SysTick->CTRL =  1; /* Enable SysTick and SysTick Timer */      //开始倒数  
  //  do
  //  {
  //    temp=SysTick->CTRL;
  //  }
  //  while((temp&0x01)&&(!(temp&(1<<16))));//等待时间到达   
  //  SysTick->CTRL	&=  (~1);    										//关闭计数器
  //  SysTick->VAL   =  (0x00);                     /* Load the SysTick Counter Value */         //清空计数器
  
  for(u8 i=0;i<nms;i++)
  {
    System_Delay_us(1000-5);
  }
  
  
  
  
}
void System_Delay_us(u32 nus)
{		
  //  u32 temp;	    	 
  //  SysTick->LOAD=nus*9; //时间加载	  		 
  //  SysTick->VAL   =  (0x00);                     /* Load the SysTick Counter Value */         //清空计数器
  //  SysTick->CTRL = 1; 														/* Enable SysTick and SysTick Timer */  //开始倒数 	 
  //  do
  //  {
  //    temp=SysTick->CTRL;
  //  }
  //  while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
  //  SysTick->CTRL = 0x00;
  //  SysTick->VAL   =  (0x00);                     /* Load the SysTick Counter Value */         //清空计数器
  
  
  TIM5->CNT = nus-1;
  TIM5->CR1 |= TIM_CR1_CEN;
  while((TIM5->SR & TIM_FLAG_Update)!=SET);
  TIM5->SR = (uint16_t)~TIM_FLAG_Update;
  TIM5->CR1 &= ~TIM_CR1_CEN;  
  
  
}
//
//
//void System_Delay_100ns(u32 nns)
//{		
//  u32 temp;	    	 
//  SysTick->LOAD=nns; //时间加载	  		 
//  SysTick->VAL   =  (0x00);                     /* Load the SysTick Counter Value */         //清空计数器
//  SysTick->CTRL = 1; 														/* Enable SysTick and SysTick Timer */  //开始倒数 	 
//  do
//  {
//    temp=SysTick->CTRL;
//  }
//  while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
//  SysTick->CTRL = 0x00;
//  SysTick->VAL   =  (0x00);                     /* Load the SysTick Counter Value */         //清空计数器
//}
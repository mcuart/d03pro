/*
SWIM 资源占用分配情况：


SWIM_IN 输入脚2个
PB6-TIM4_CH1, PB7-TIM4_CH2

以上两个引脚一起工作在PWM输入模式（即测量PWM占空比或者周期长度），
TIM4_CCR1内的值为PWM的长度
TIM4_CCR2内的值为PWM占空比，程序中只用到了占空比
TIM4_CH2对应的的DMA通道为DMA1_Channel4
PWM输入模式只能用TIMx_CH1/TIMx_CH2引脚


SWIM_OUT输出脚1个 PB4-TIM3_CH1-DMA1_Channel6。该引脚要求只要能通过DMA自动发出PWM的任意一个TIMx_CHx引脚都可以


复位输出脚1个RST PB5，任意一个GPIO都可以



*/


#ifndef _SWIM_A_H_
#define _SWIM_A_H_

#include "stm32f4xx.h"
#include "macro.h"
#include"ff.h"
#include"main.h"
#include "aes.h"
#include <cjson/cJSON.h>
#include "config.h"


#include "os.h"

#include "delay.h"

#include "swim_core.h"

//#define SWIM_MAX_DLY					0xffff
#define SWIM_MAX_DLY					0x20000
#define SWIM_MAX_RESEND_CNT		20

#define SWIM_CMD_BITLEN					3
#define SWIM_CMD_SRST					0x00	 //复位
#define SWIM_CMD_ROTF					0x01	 //SWIM 飞速读
#define SWIM_CMD_WOTF					0x02	 //SWIM 飞速写

#define SWIM_SYNC_CYCLES				128


#define _SYS_FREQUENCY			    84						


#define GPIO_SWIM_RSTOUT_PIN						GPIO_Pin_1 //PB1 SWIM RST 输出
#define GPIO_SWIM_OUT_PIN						GPIO_Pin_0 //Tim3_CH3映射到PB0作为SWIM输出
#define GPIO_SWIM_IN1_PIN						GPIO_Pin_12 //PD12、PD13 2根线都是作为SWIM输入
#define GPIO_SWIM_IN2_PIN				    		GPIO_Pin_13


#define SWIM_RST_HIGH()	(GPIO_WriteBit(GPIOB, GPIO_SWIM_RSTOUT_PIN, Bit_SET))			 
#define SWIM_RST_LOW() 	(GPIO_WriteBit(GPIOB, GPIO_SWIM_RSTOUT_PIN, Bit_RESET))

#define SWIM_HIGH()     (GPIO_WriteBit(GPIOB, GPIO_SWIM_OUT_PIN, Bit_SET))			 
#define SWIM_LOW() 	(GPIO_WriteBit(GPIOB, GPIO_SWIM_OUT_PIN, Bit_RESET))



#define SET_U16_MSBFIRST(p, v)		\
	do{\
		*((uint8_t *)(p) + 0) = (((uint16_t)(v)) >> 8) & 0xFF;\
		*((uint8_t *)(p) + 1) = (((uint16_t)(v)) >> 0) & 0xFF;\
	} while (0)
#define SET_U24_MSBFIRST(p, v)		\
	do{\
		*((uint8_t *)(p) + 0) = (((uint32_t)(v)) >> 16) & 0xFF;\
		*((uint8_t *)(p) + 1) = (((uint32_t)(v)) >> 8) & 0xFF;\
		*((uint8_t *)(p) + 2) = (((uint32_t)(v)) >> 0) & 0xFF;\
	} while (0)
#define SET_U32_MSBFIRST(p, v)		\
	do{\
		*((uint8_t *)(p) + 0) = (((uint32_t)(v)) >> 24) & 0xFF;\
		*((uint8_t *)(p) + 1) = (((uint32_t)(v)) >> 16) & 0xFF;\
		*((uint8_t *)(p) + 2) = (((uint32_t)(v)) >> 8) & 0xFF;\
		*((uint8_t *)(p) + 3) = (((uint32_t)(v)) >> 0) & 0xFF;\
	} while (0)
#define SET_U16_LSBFIRST(p, v)		\
	do{\
		*((uint8_t *)(p) + 0) = (((uint16_t)(v)) >> 0) & 0xFF;\
		*((uint8_t *)(p) + 1) = (((uint16_t)(v)) >> 8) & 0xFF;\
	} while (0)
#define SET_U24_LSBFIRST(p, v)		\
	do{\
		*((uint8_t *)(p) + 0) = (((uint32_t)(v)) >> 0) & 0xFF;\
		*((uint8_t *)(p) + 1) = (((uint32_t)(v)) >> 8) & 0xFF;\
		*((uint8_t *)(p) + 2) = (((uint32_t)(v)) >> 16) & 0xFF;\
	} while (0)
#define SET_U32_LSBFIRST(p, v)		\
	do{\
		*((uint8_t *)(p) + 0) = (((uint32_t)(v)) >> 0) & 0xFF;\
		*((uint8_t *)(p) + 1) = (((uint32_t)(v)) >> 8) & 0xFF;\
		*((uint8_t *)(p) + 2) = (((uint32_t)(v)) >> 16) & 0xFF;\
		*((uint8_t *)(p) + 3) = (((uint32_t)(v)) >> 24) & 0xFF;\
	} while (0)

#define GET_LE_U16(p)					GET_U16_LSBFIRST(p)
#define GET_LE_U24(p)					GET_U24_LSBFIRST(p)
#define GET_LE_U32(p)					GET_U32_LSBFIRST(p)
#define GET_BE_U16(p)					GET_U16_MSBFIRST(p)
#define GET_BE_U24(p)					GET_U24_MSBFIRST(p)
#define GET_BE_U32(p)					GET_U32_MSBFIRST(p)
#define SET_LE_U16(p, v)			SET_U16_LSBFIRST(p, v)
#define SET_LE_U24(p, v)			SET_U24_LSBFIRST(p, v)
#define SET_LE_U32(p, v)			SET_U32_LSBFIRST(p, v)
#define SET_BE_U16(p, v)			SET_U16_MSBFIRST(p, v)
#define SET_BE_U24(p, v)			SET_U24_MSBFIRST(p, v)
#define SET_BE_U32(p, v)			SET_U32_MSBFIRST(p, v)



#define SYNCSWPWM_IN_TIMER				    TIM4
#define SYNCSWPWM_IN_TIMER_RISE_DMA		DMA1_Stream3		//TIM4_CH2  PD13引脚 TIM4_CH2 作为 PWM输入捕获脚     
          
#define SYNCSWPWM_OUT_TIMER_MHZ			  _SYS_FREQUENCY
#define SYNCSWPWM_OUT_TIMER				    TIM3     //TIM3 CH3 作为 PWM输出端
#define SYNCSWPWM_OUT_TIMER_DMA			  DMA1_Stream7     //TIM3_CH3  作为PWM输出端   映射在PB0引脚上          

  
          
			













void SWIM_GPIO_Init_SWIM_A(void);
void SWIM_Init_A(void);
void SWIM_DeInit_A(void *pswim);
u8 Pro_Init_SWIM(SWIM_HANDLE *pswim);	
u16 Write_OpitonByte_STM8_pre_SWIM(SWIM_HANDLE *pswim);
u8 Write_OpitonByte_STM8_SWIM(SWIM_HANDLE *pswim);
u8 ProgramFlash_SWIM(SWIM_HANDLE *pswim);
u8 ProgramEEPROM_SWIM(SWIM_HANDLE *pswim);
u8 End_Pro_SWIM(SWIM_HANDLE *pswim);
u8 ClearChip_SWIM(SWIM_HANDLE *pswim);
u8 WriteUniqueIDJiaMi_SWIM(SWIM_HANDLE *pswim);
u8 WriteRollingCode_SWIM(SWIM_HANDLE *pswim);
void SWIM_EnableClockInput_A(void);
void SYNCSWPWM_IN_TIMER_RISE_DMA_INIT_A(u8 length, u32* address);
void SYNCSWPWM_OUT_TIMER_DMA_INIT_A(u8 length, u32* address);
#endif  /* __SWIM_A_H__ */
                                                                                  

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


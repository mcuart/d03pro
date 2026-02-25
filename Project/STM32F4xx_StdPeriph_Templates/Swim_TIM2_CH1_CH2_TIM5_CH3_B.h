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


#ifndef _SWIM_B_H_
#define _SWIM_B_H_

#include "stm32f4xx.h"
#include "macro.h"
#include"ff.h"
#include"main.h"
#include "aes.h"
#include <cJSON/cJSON.h>
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


#define GPIO_SWIM_RSTOUT_PIN						GPIO_Pin_3 //PA3 SWIM RST 输出
#define GPIO_SWIM_OUT_PIN						GPIO_Pin_2 //TIM5_CH3映射到PA2作为SWIM输出
#define GPIO_SWIM_IN1_PIN						GPIO_Pin_0 //PA0、PA1 2根线都是作为SWIM输入
#define GPIO_SWIM_IN2_PIN				    		GPIO_Pin_1


#define SWIM_RST_HIGH(pswim)	(GPIO_WriteBit(((SWIM_HANDLE *)pswim)->swim_rst_port, ((SWIM_HANDLE *)pswim)->swim_rst_pin, Bit_SET))			 




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



#define SYNCSWPWM_IN_TIMER				    TIM2
#define SYNCSWPWM_IN_TIMER_RISE_DMA		DMA1_Stream6		//TIM2_CH2  PA1引脚 TIM2_CH2 作为 PWM输入捕获脚          
          
#define SYNCSWPWM_OUT_TIMER_MHZ			  _SYS_FREQUENCY
#define SYNCSWPWM_OUT_TIMER				    TIM5     //TIM5 CH3 作为 PWM输出端
#define SYNCSWPWM_OUT_TIMER_DMA			  DMA1_Stream0     //TIM5_CH3  作为PWM输出端   映射在PA2引脚上          


void SWIM_GPIO_Init_SWIM_B(void *pswim);
void SWIM_Init_B(void);
void SWIM_DeInit_B(void *pswim);
void SWIM_EnableClockInput_B(void);
void SYNCSWPWM_IN_TIMER_RISE_DMA_INIT_B(u8 length, u32 *address);
void SYNCSWPWM_IN_TIMER_RISE_DMA_WAIT_B(u32 dly);
void SYNCSWPWM_OUT_TIMER_DMA_INIT_B(u8 length, u32 *address);
void SYNCSWPWM_OUT_TIMER_DMA_WAIT_B(u32 dly);
void SYNCSWPWM_OUT_TIMER_SetCycle_B(u32 cycle);

#endif  /* __SWIM_B_H__ */
                                                                                  

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


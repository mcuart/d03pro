/**
  ******************************************************************************
  * @file    usb_core.h
  * @author  MCD Application Team
  * @version V2.2.1
  * @date    17-March-2018
  * @brief   Header of the Core Layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      <http://www.st.com/SLA0044>
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SWIM_CORE_H__
#define __SWIM_CORE_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <cJSON/cJSON.h>



typedef struct _SWIM
{
  u16 ImageNumber_store;
  void (*syncswpwm_in_timer_rise_dma_init)(u8 length, u32 *address);
  void (*syncswpwm_in_timer_rise_dma_wait)(u32 dly);
  void (*syncswpwm_out_timer_dma_init)(u8 length, u32 *address);
  void (*syncswpwm_out_timer_dma_wait)(u32 dly);
  void (*syncswpwm_out_timer_setcycle)(u32 cycle);
  void (*swim_gpio_init)(void *pswim);
  void (*swim_init)(void);
  void (*swim_deinit)(void *pswim);
  void (*swim_enable_clock_input)(void);
  cJSON *root;
  
  GPIO_TypeDef* swim_port;
  u32 swim_pin;  
  
  GPIO_TypeDef* swim_rst_port;
  u32 swim_rst_pin;  
} SWIM_HANDLE;


#endif  /* __SWIM_CORE_H__ */


/**
  * @}
  */ 

/**
  * @}
  */ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


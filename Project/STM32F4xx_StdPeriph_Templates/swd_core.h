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
#ifndef __SWD_CORE_H__
#define __SWD_CORE_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <cJSON/cJSON.h>


typedef struct _MCU
{
  u8 McuName;//MCU 名称
  u8 flash_size_index;//flash大小索引
  u8 eeprom_size_index;//eeprom大小索引
  u8 otp_size_index;//otp大小索引
  u8 OptionByteMax275;
  u8 page_size_index;	//页大小索引
  u8 flash_interface_address_index;//flash控制接口地址索引
  u8 optionbyte_start_address_index;//选项字节起始地址索引
  u8 flash_start_address_index;//flash起始地址索引
  u8 eeprom_start_address_index;//eeprom起始地址索引
  u8 otp_start_address_index;//otp起始地址索引
  u8 read_protect_key_index;//读保护key索引
  u8 flash_key_index;//flash key索引
  u8 otp_key_index;//otp key索引
  u8 program_protocol_index;//编程协议索引
  u8 flash_loader_index;
  u8 program_page_timeout_index;//Time Out of Program Page Function
  u8 erase_address_index;//Time Out of Erase Sector Function
} stm32_mcu_info;


typedef struct _SWD
{
  u16 ImageNumber_store;
  u8 (*swd_init)(void * pswd);
  
  cJSON *root;
  
  GPIO_TypeDef* swdio_port;
  u32 swdio_pin;
  
  GPIO_TypeDef* swclk_port;
  u32 swclk_pin;  
 
  GPIO_TypeDef* sw_nreset_port;
  u32 sw_nreset_pin;    
  
  u8 code_bar_plug;
} SWD_HANDLE;


#endif  /* __SWD_CORE_H__ */


/**
  * @}
  */ 

/**
  * @}
  */ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


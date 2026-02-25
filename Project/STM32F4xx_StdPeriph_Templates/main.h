/**
  ******************************************************************************
  * @file    DCMI/DCMI_CameraExample/main.h 
  * @author  MCD Application Team
  * @version V1.8.1
  * @date    27-January-2022
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 0 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/


#if defined (USE_STM324xG_EVAL)
  #include "stm324xg_eval.h"
  #include "stm324xg_eval_lcd.h"
  #include "stm324xg_eval_ioe.h"

#elif defined (USE_STM324x7I_EVAL) 
  #include "stm324x7i_eval.h"
  #include "stm324x7i_eval_lcd.h"
  #include "stm324x7i_eval_ioe.h"

#else
 #error "Please select first the Evaluation board used in your application (in Project Options)"
#endif

#include "touch.h"
#include "GUI.h"
#include "DIALOG.h"
#include "GUIDEMO.h"


#include "key.h" 
    
    
    
    
    
/* Exported types ------------------------------------------------------------*/
typedef union
{
  u32 system_set_data_all;
  struct system_set
  {
    u32 bit_0:1;//使能蜂鸣器提示
    u32 bit_1:1;//设置开机进入脱机烧写
    u32 bit_2:1;//允许自动连接计算机
    u32 bit_3:1;//使能开机密码
    u32 bit_4:1;//使能通道A
    u32 bit_5:1;//使能通道B
    u32 bit_6:1;//rolling code hex 滚码十六进制显示
    u32 bit_7:1;
    u32 bit_8_31:24;    
  }system_set_data_bit;
}sytem_parameter;

extern sytem_parameter system_set_data_r;
extern uint8_t sFLASH_Rx_Buffer[100];

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define BUZZER_ON (TIM1->CCR3=100)
#define BUZZER_ON_1 TIM1->CCR3=250;TIM1->ARR=500
#define BUZZER_ON_2 TIM1->CCR3=50;TIM1->ARR=100
#define BUZZER_OFF TIM1->CCR3=1001;TIM1->ARR=1000    
    
    



#define SW_3V3_PIN_0 GPIO_Pin_6
#define SW_3V3_GPIO_0 GPIOD

#define SW_3V3_ON_0 GPIO_WriteBit(SW_3V3_GPIO_0, SW_3V3_PIN_0, (BitAction)0)
#define SW_3V3_OFF_0 GPIO_WriteBit(SW_3V3_GPIO_0, SW_3V3_PIN_0, (BitAction)1)    


#define SW_5V_PIN_0 GPIO_Pin_3
#define SW_5V_GPIO_0 GPIOD

#define SW_5V_ON_0 GPIO_WriteBit(SW_5V_GPIO_0, SW_5V_PIN_0, (BitAction)0)
#define SW_5V_OFF_0 GPIO_WriteBit(SW_5V_GPIO_0, SW_5V_PIN_0, (BitAction)1)



#define SW_3V3_PIN_1 GPIO_Pin_9
#define SW_3V3_GPIO_1 GPIOA

#define SW_3V3_ON_1 GPIO_WriteBit(SW_3V3_GPIO_1, SW_3V3_PIN_1, (BitAction)0)
#define SW_3V3_OFF_1 GPIO_WriteBit(SW_3V3_GPIO_1, SW_3V3_PIN_1, (BitAction)1)    


#define SW_5V_PIN_1 GPIO_Pin_8
#define SW_5V_GPIO_1 GPIOA

#define SW_5V_ON_1 GPIO_WriteBit(SW_5V_GPIO_1, SW_5V_PIN_1, (BitAction)0)
#define SW_5V_OFF_1 GPIO_WriteBit(SW_5V_GPIO_1, SW_5V_PIN_1, (BitAction)1)



#define DISPLAY_POSITION_Y 185
/*******************************************************************************
parameter参数保存位于W25Q32 Block 63 Sector 15 
保存APP固件CRC、固件版本、APP内相关设置信息
*******************************************************************************/
#define  FLASH_WRITE_ADDRESS      0x3FF000
#define  FLASH_READ_ADDRESS       FLASH_WRITE_ADDRESS
#define  FLASH_SECTOR_TO_ERASE    FLASH_WRITE_ADDRESS   
#define PARAMETER_ADDRESS_W FLASH_WRITE_ADDRESS

#define PARAMETER_ADDRESS_R &sFLASH_Rx_Buffer[0]

#define OFFSET_CRC 0x0
#define OFFSET_FIRMWARE_VERSION 4
#define OFFSET_FIRMWARE_VERSION_1 8



#define OFFSET_UID1 12
#define OFFSET_UID2 16
#define OFFSET_UID3 20

#define OFFSET_IMAGE_NUMBER 24


#define OFFSET_SYSTEM_SET_DATA 28
#define OFFSET_PASSWORD_VALUE 32
#define OFFSET_LANGUAGE 36

#define OFFSET_IMAGE_NUMBER_A 40
#define OFFSET_IMAGE_NUMBER_B 44


#define SYSTEM_SET_DATA_ADDRESS_R (PARAMETER_ADDRESS_R+OFFSET_SYSTEM_SET_DATA)
#define SYSTEM_SET_DATA_ADDRESS_W (PARAMETER_ADDRESS_W+OFFSET_SYSTEM_SET_DATA)
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */


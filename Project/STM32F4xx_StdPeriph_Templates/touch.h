#ifndef TOUCH_H_
#define TOUCH_H_

#include "main.h"
/*----------------------TOUCH-----------------------------*/

//#define TOUCH_PEN_PORT              GPIOC
//#define TOUCH_PEN_CLK               RCC_APB2Periph_GPIOC  
//#define TOUCH_PEN_PIN               GPIO_PIN_0
//
//#define TOUCH_PEN                   (HAL_GPIO_ReadPin(TOUCH_PEN_PORT, TOUCH_PEN_PIN))
//
//
//#define TOUCH_TDOUT_PORT            GPIOA
//#define TOUCH_TDOUT_CLK             RCC_APB2Periph_GPIOA
//#define TOUCH_TDOUT_PIN             GPIO_PIN_6
//
//
//#define TOUCH_TDIN_PORT             GPIOA
//#define TOUCH_TDIN_CLK              RCC_APB2Periph_GPIOA
//#define TOUCH_TDIN_PIN              GPIO_PIN_7
//
//
//#define TOUCH_TCLK_PORT             GPIOA
//#define TOUCH_TCLK_CLK              RCC_APB2Periph_GPIOA
//#define TOUCH_TCLK_PIN              GPIO_PIN_5
//
//
//
//#define TOUCH_TCS_PORT              GPIOA
//#define TOUCH_TCS_CLK               RCC_APB2Periph_GPIOA
//#define TOUCH_TCS_PIN               GPIO_PIN_4
//#define Set_TOUCH_TCS               HAL_GPIO_WritePin(TOUCH_TCS_PORT,TOUCH_TCS_PIN, GPIO_PIN_SET)
//#define Reset_TOUCH_TCS             HAL_GPIO_WritePin(TOUCH_TCS_PORT,TOUCH_TCS_PIN, GPIO_PIN_RESET)




#define TOUCH_TDOUT_PORT            GPIOE
#define TOUCH_TDOUT_CLK             RCC_AHB1Periph_GPIOE
#define TOUCH_TDOUT_PIN             GPIO_Pin_4
#define TOUCH_DOUT                  GPIO_ReadInputDataBit(TOUCH_TDOUT_PORT, TOUCH_TDOUT_PIN)


#define TOUCH_TDIN_PORT             GPIOE
#define TOUCH_TDIN_CLK              RCC_AHB1Periph_GPIOE
#define TOUCH_TDIN_PIN              GPIO_Pin_5
#define Set_TOUCH_TDIN               GPIO_WriteBit(TOUCH_TDIN_PORT,TOUCH_TDIN_PIN, Bit_SET)
#define Reset_TOUCH_TDIN             GPIO_WriteBit(TOUCH_TDIN_PORT,TOUCH_TDIN_PIN, Bit_RESET)





#define TOUCH_TCLK_PORT             GPIOE
#define TOUCH_TCLK_CLK              RCC_AHB1Periph_GPIOE
#define TOUCH_TCLK_PIN              GPIO_Pin_3
#define Set_TOUCH_TCLK               GPIO_WriteBit(TOUCH_TCLK_PORT,TOUCH_TCLK_PIN, Bit_SET)
#define Reset_TOUCH_TCLK             GPIO_WriteBit(TOUCH_TCLK_PORT,TOUCH_TCLK_PIN, Bit_RESET)


#define TOUCH_TCS_PORT              GPIOE
#define TOUCH_TCS_CLK               RCC_AHB1Periph_GPIOE
#define TOUCH_TCS_PIN               GPIO_Pin_2
#define Set_TOUCH_TCS               GPIO_WriteBit(TOUCH_TCS_PORT,TOUCH_TCS_PIN, Bit_SET)
#define Reset_TOUCH_TCS             GPIO_WriteBit(TOUCH_TCS_PORT,TOUCH_TCS_PIN, Bit_RESET)

#define CMD_RDY 0X90  
#define CMD_RDX	0XD0     											 
#define TEMP_RD	0XF0  





void Touch_Init(void);

uint16_t ADS_Read_XY(uint8_t xy);
uint16_t ADS_Read_XY(uint8_t xy);


#endif

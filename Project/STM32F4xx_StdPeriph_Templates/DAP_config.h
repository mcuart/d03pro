/**************************************************************************//**
* @file     DAP_config.h
* @brief    CMSIS-DAP Configuration File for STM32F103C6/8/B
* @version  V1.00
* @date     02. Oct 2012
*
* @note
* Copyright (C) 2012 ARM Limited. All rights reserved.
*
* @par
* ARM Limited (ARM) is supplying this software for use with Cortex-M
* processor based microcontrollers.
*
* @par
* THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
* ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
******************************************************************************/

#ifndef __DAP_CONFIG_H__
#define __DAP_CONFIG_H__

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "swd_core.h"

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)


#if   defined( USE_DEBUG )
#define DEBUG(...)	printf(__VA_ARGS__);printf("\r\n")
#define INFO(...)	printf(__VA_ARGS__);printf("\r\n")
#define ERROR(...)	printf(__VA_ARGS__);printf("\r\n")
#elif defined( USE_INFO )
#define DEBUG(...)
#define INFO(...)	printf(__VA_ARGS__)
#define ERROR(...)	printf(__VA_ARGS__)
#elif defined( USE_ERROR )
#define DEBUG(...)
#define INFO(...)
#define ERROR(...)	printf(__VA_ARGS__)
#else
#define DEBUG(...)
#define INFO(...)
#define ERROR(...)
#endif

//**************************************************************************************************
/** 
\defgroup DAP_Config_Debug_gr CMSIS-DAP Debug Unit Information
\ingroup DAP_ConfigIO_gr 
@{
Provides definitions about:
- Definition of Cortex-M processor parameters used in CMSIS-DAP Debug Unit.
- Debug Unit communication packet size.
- Debug Access Port communication mode (JTAG or SWD).
- Optional information about a connected Target Device (for Evaluation Boards).
*/

#include "stm32f4xx.h"

/// Processor Clock of the Cortex-M MCU used in the Debug Unit.
/// This value is used to calculate the SWD/JTAG clock speed.
#define CPU_CLOCK				SystemCoreClock		///< Specifies the CPU Clock in Hz

/// Number of processor cycles for I/O Port write operations.
/// This value is used to calculate the SWD/JTAG clock speed that is generated with I/O
/// Port write operations in the Debug Unit by a Cortex-M MCU. Most Cortex-M processors
/// requrie 2 processor cycles for a I/O Port Write operation.  If the Debug Unit uses
/// a Cortex-M0+ processor with high-speed peripheral I/O only 1 processor cycle might be 
/// requrired.
#define IO_PORT_WRITE_CYCLES	2               ///< I/O Cycles: 2=default, 1=Cortex-M0+ fast I/0

/// Indicate that Serial Wire Debug (SWD) communication mode is available at the Debug Access Port.
/// This information is returned by the command \ref DAP_Info as part of <b>Capabilities</b>.
#define DAP_SWD                 1               ///< SWD Mode:  1 = available, 0 = not available


#define DAP_JTAG			0				///< JTAG Mode: 1 = available, 0 = not available.




/// Default communication mode on the Debug Access Port.
/// Used for the command \ref DAP_Connect when Port Default mode is selected.
#define DAP_DEFAULT_PORT        1               ///< Default JTAG/SWJ Port Mode: 1 = SWD, 2 = JTAG.

/// Default communication speed on the Debug Access Port for SWD and JTAG mode.
/// Used to initialize the default SWD/JTAG clock frequency.
/// The command \ref DAP_SWJ_Clock can be used to overwrite this default setting.
#define DAP_DEFAULT_SWJ_CLOCK   1000000         ///< Default SWD/JTAG clock frequency in Hz.

/// Maximum Package Size for Command and Response data.
/// This configuration settings is used to optimized the communication performance with the
/// debugger and depends on the USB peripheral. Change setting to 1024 for High-Speed USB.
#define DAP_PACKET_SIZE			1024				///< USB: 64 = Full-Speed, 1024 = High-Speed.

/// Maximum Package Buffers for Command and Response data.
/// This configuration settings is used to optimized the communication performance with the
/// debugger and depends on the USB peripheral. For devices with limited RAM or USB buffer the
/// setting can be reduced (valid range is 1 .. 255). Change setting to 4 for High-Speed USB.
#define DAP_PACKET_COUNT		1				///< Buffers: 64 = Full-Speed, 4 = High-Speed.


/// Debug Unit is connected to fixed Target Device.
/// The Debug Unit may be part of an evaluation board and always connected to a fixed
/// known device.  In this case a Device Vendor and Device Name string is stored which
/// may be used by the debugger or IDE to configure device parameters.
#define TARGET_DEVICE_FIXED     0               ///< Target Device: 1 = known, 0 = unknown;

#if TARGET_DEVICE_FIXED
#define TARGET_DEVICE_VENDOR    ""		///< String indicating the Silicon Vendor
#define TARGET_DEVICE_NAME      ""		///< String indicating the Target Device
#endif

///@}

#define GPIO_INIT(port, data)	GPIO_Init(port, (GPIO_InitTypeDef *)&data)
#define PIN_MODE_MASK(pin)		(((uint32_t)0x03) << ((pin) << 1))
#define PIN_MODE(mode,pin)		(((uint32_t)mode) << ((pin) << 1))
#define PIN_MASK(pin)			(((uint16_t)0x01) << (pin))


#define PIN_OTYPE_MASK(pin)		(((uint32_t)0x01) << pin)
#define PIN_OTYPE(otype,pin)		(((uint32_t)otype) << pin)

#define PIN_OSPEED_MASK(pin)		(((uint32_t)0x03) << ((pin) << 1))
#define PIN_OSPEED(ospeed,pin)		(((uint32_t)ospeed) << ((pin) << 1))

#define PIN_PUPD_MASK(pin)		(((uint32_t)0x03) << ((pin) << 1))
#define PIN_PUPD(pupd,pin)		(((uint32_t)pupd) << ((pin) << 1))

typedef enum Pin_e {
  PA = 0x00, PA0 = 0x00, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15,
  PB = 0x10, PB0 = 0x10, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15,
  PC = 0x20, PC0 = 0x20, PC1, PC2, PC3, PC4, PC5, PC6, PC7, PC8, PC9, PC10, PC11, PC12, PC13, PC14, PC15,
  PD = 0x30, PD0 = 0x30, PD1, PD2,
} Pin_t;


#define USE_B

/***********SWD两组切换只需要改变引脚定义即可，其它地方不用动20241004*****************/


/***********SWD_0组(B组 屏幕对侧)引脚定义开始*****************/
#if defined (USE_B)
// SWDIO/TMS Pin
#define PIN_SWDIO_TMS_PORT		GPIOA
#define PIN_SWDIO_TMS_PIN		2

// SWCLK/TCK Pin
#define PIN_SWCLK_TCK_PORT		GPIOA
#define PIN_SWCLK_TCK_PIN		3

//// TDO/SWO Pin (input)
//#define PIN_TDO_PORT                    GPIOA
//#define PIN_TDO			        6

// nRESET Pin
#define PIN_nRESET_PORT                 GPIOC
#define PIN_nRESET_PIN			5
#else
/***********SWD_0组引脚定义结束*****************/


/***********SWD_1组(A组 屏幕侧)引脚定义开始*****************/
// SWDIO/TMS Pin
#define PIN_SWDIO_TMS_PORT		GPIOB
#define PIN_SWDIO_TMS_PIN		0

// SWCLK/TCK Pin
#define PIN_SWCLK_TCK_PORT		GPIOB
#define PIN_SWCLK_TCK_PIN		1

//// TDO/SWO Pin (input)
//#define PIN_TDO_PORT                    GPIOA
//#define PIN_TDO			        6

// nRESET Pin
#define PIN_nRESET_PORT                 GPIOB
#define PIN_nRESET_PIN			2
/***********SWD_1组引脚定义结束*****************/
#endif


#define PIN_nRESET			PIN_MASK(PIN_nRESET_PIN)
#define PIN_SWDIO_TMS			PIN_MASK(PIN_SWDIO_TMS_PIN)
#define PIN_SWCLK_TCK			PIN_MASK(PIN_SWCLK_TCK_PIN)


#pragma inline = forced
void PIN_nRESET_LOW(SWD_HANDLE * pswd)
{
//  //开漏输出低电平
//  PIN_nRESET_PORT->MODER = (PIN_nRESET_PORT->MODER & ~PIN_MODE_MASK(PIN_nRESET_PIN)) | PIN_MODE(0x1, PIN_nRESET_PIN);//输出
//  PIN_nRESET_PORT->OTYPER = (PIN_nRESET_PORT->OTYPER & ~PIN_OTYPE_MASK(PIN_nRESET_PIN)) | PIN_OTYPE(0x1, PIN_nRESET_PIN);//开漏
//  PIN_nRESET_PORT->OSPEEDR = (PIN_nRESET_PORT->OSPEEDR & ~PIN_OSPEED_MASK(PIN_nRESET_PIN)) | PIN_OSPEED(0x3, PIN_nRESET_PIN);
//  PIN_nRESET_PORT->PUPDR = (PIN_nRESET_PORT->PUPDR & ~PIN_PUPD_MASK(PIN_nRESET_PIN)) | PIN_PUPD(0x0, PIN_nRESET_PIN);//没有上下拉电阻
    //开漏输出低电平
  pswd->sw_nreset_port->MODER = (pswd->sw_nreset_port->MODER & ~PIN_MODE_MASK(pswd->sw_nreset_pin)) | PIN_MODE(0x1, pswd->sw_nreset_pin);//输出
  pswd->sw_nreset_port->OTYPER = (pswd->sw_nreset_port->OTYPER & ~PIN_OTYPE_MASK(pswd->sw_nreset_pin)) | PIN_OTYPE(0x1, pswd->sw_nreset_pin);//开漏
  pswd->sw_nreset_port->OSPEEDR = (pswd->sw_nreset_port->OSPEEDR & ~PIN_OSPEED_MASK(pswd->sw_nreset_pin)) | PIN_OSPEED(0x3, pswd->sw_nreset_pin);
  pswd->sw_nreset_port->PUPDR = (pswd->sw_nreset_port->PUPDR & ~PIN_PUPD_MASK(pswd->sw_nreset_pin)) | PIN_PUPD(0x0, pswd->sw_nreset_pin);//没有上下拉电阻
}

#pragma inline = forced  
void PIN_nRESET_HIGH(SWD_HANDLE * pswd)							
{											
//  //浮空输入
//  PIN_nRESET_PORT->MODER = (PIN_nRESET_PORT->MODER & ~PIN_MODE_MASK(PIN_nRESET_PIN)) | PIN_MODE(0x0, PIN_nRESET_PIN);//输入
//  PIN_nRESET_PORT->OTYPER = (PIN_nRESET_PORT->OTYPER & ~PIN_OTYPE_MASK(PIN_nRESET_PIN)) | PIN_OTYPE(0x0, PIN_nRESET_PIN);
//  PIN_nRESET_PORT->OSPEEDR = (PIN_nRESET_PORT->OSPEEDR & ~PIN_OSPEED_MASK(PIN_nRESET_PIN)) | PIN_OSPEED(0x3, PIN_nRESET_PIN);
//  PIN_nRESET_PORT->PUPDR = (PIN_nRESET_PORT->PUPDR & ~PIN_PUPD_MASK(PIN_nRESET_PIN)) | PIN_PUPD(0x0, PIN_nRESET_PIN);//没有上下拉电阻
  
  //浮空输入
  pswd->sw_nreset_port->MODER = (pswd->sw_nreset_port->MODER & ~PIN_MODE_MASK(pswd->sw_nreset_pin)) | PIN_MODE(0x0, pswd->sw_nreset_pin);//输入
  pswd->sw_nreset_port->OTYPER = (pswd->sw_nreset_port->OTYPER & ~PIN_OTYPE_MASK(pswd->sw_nreset_pin)) | PIN_OTYPE(0x0, pswd->sw_nreset_pin);
  pswd->sw_nreset_port->OSPEEDR = (pswd->sw_nreset_port->OSPEEDR & ~PIN_OSPEED_MASK(pswd->sw_nreset_pin)) | PIN_OSPEED(0x3, pswd->sw_nreset_pin);
  pswd->sw_nreset_port->PUPDR = (pswd->sw_nreset_port->PUPDR & ~PIN_PUPD_MASK(pswd->sw_nreset_pin)) | PIN_PUPD(0x0, pswd->sw_nreset_pin);//没有上下拉电阻  
}


#pragma inline = forced  
void PIN_SWDIO_TMS_OUT_DISABLE(SWD_HANDLE * pswd)
{
//  //上拉输入
//  PIN_SWDIO_TMS_PORT->MODER = (PIN_SWDIO_TMS_PORT->MODER & ~PIN_MODE_MASK(PIN_SWDIO_TMS_PIN)) | PIN_MODE(0x0, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->OTYPER = (PIN_SWDIO_TMS_PORT->OTYPER & ~PIN_OTYPE_MASK(PIN_SWDIO_TMS_PIN)) | PIN_OTYPE(0x1, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->OSPEEDR = (PIN_SWDIO_TMS_PORT->OSPEEDR & ~PIN_OSPEED_MASK(PIN_SWDIO_TMS_PIN)) | PIN_OSPEED(0x3, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->PUPDR = (PIN_SWDIO_TMS_PORT->PUPDR & ~PIN_PUPD_MASK(PIN_SWDIO_TMS_PIN)) | PIN_PUPD(0x1, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->BSRR  = PIN_SWDIO_TMS;
  
  //上拉输入
  pswd->swdio_port->MODER = (pswd->swdio_port->MODER & ~PIN_MODE_MASK(pswd->swdio_pin)) | PIN_MODE(0x0, pswd->swdio_pin);
  pswd->swdio_port->OTYPER = (pswd->swdio_port->OTYPER & ~PIN_OTYPE_MASK(pswd->swdio_pin)) | PIN_OTYPE(0x1, pswd->swdio_pin);
  pswd->swdio_port->OSPEEDR = (pswd->swdio_port->OSPEEDR & ~PIN_OSPEED_MASK(pswd->swdio_pin)) | PIN_OSPEED(0x3, pswd->swdio_pin);
  pswd->swdio_port->PUPDR = (pswd->swdio_port->PUPDR & ~PIN_PUPD_MASK(pswd->swdio_pin)) | PIN_PUPD(0x1, pswd->swdio_pin);
  pswd->swdio_port->BSRR  = PIN_MASK(pswd->swdio_pin);  
  
}

#pragma inline = forced  
void PIN_SWDIO_TMS_OUT_ENABLE(SWD_HANDLE * pswd)
{
//  //推挽输出低电平
//  PIN_SWDIO_TMS_PORT->MODER = (PIN_SWDIO_TMS_PORT->MODER & ~PIN_MODE_MASK(PIN_SWDIO_TMS_PIN)) | PIN_MODE(0x1, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->OTYPER = (PIN_SWDIO_TMS_PORT->OTYPER & ~PIN_OTYPE_MASK(PIN_SWDIO_TMS_PIN)) | PIN_OTYPE(0x1, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->OSPEEDR = (PIN_SWDIO_TMS_PORT->OSPEEDR & ~PIN_OSPEED_MASK(PIN_SWDIO_TMS_PIN)) | PIN_OSPEED(0x3, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->PUPDR = (PIN_SWDIO_TMS_PORT->PUPDR & ~PIN_PUPD_MASK(PIN_SWDIO_TMS_PIN)) | PIN_PUPD(0x0, PIN_SWDIO_TMS_PIN);
//  PIN_SWDIO_TMS_PORT->BSRR  = PIN_SWDIO_TMS<<16;
  
  
  //推挽输出低电平
  pswd->swdio_port->MODER = (pswd->swdio_port->MODER & ~PIN_MODE_MASK(pswd->swdio_pin)) | PIN_MODE(0x1, pswd->swdio_pin);
  pswd->swdio_port->OTYPER = (pswd->swdio_port->OTYPER & ~PIN_OTYPE_MASK(pswd->swdio_pin)) | PIN_OTYPE(0x1, pswd->swdio_pin);
  pswd->swdio_port->OSPEEDR = (pswd->swdio_port->OSPEEDR & ~PIN_OSPEED_MASK(pswd->swdio_pin)) | PIN_OSPEED(0x3, pswd->swdio_pin);
  pswd->swdio_port->PUPDR = (pswd->swdio_port->PUPDR & ~PIN_PUPD_MASK(pswd->swdio_pin)) | PIN_PUPD(0x0, pswd->swdio_pin);
  pswd->swdio_port->BSRR  = (PIN_MASK(pswd->swdio_pin))<<16;  
  
}


void PORT_SWD_SETUP(SWD_HANDLE * pswd);
void PORT_OFF(SWD_HANDLE * pswd);


#define DAP_SETUP()	PORT_OFF()

// SWCLK/TCK I/O pin -------------------------------------

/** SWCLK/TCK I/O pin: Get Input.
\return Current status of the SWCLK/TCK DAP hardware I/O pin.
*/
#pragma inline = forced
uint8_t PIN_SWCLK_TCK_IN(void)
{
  return (PIN_SWCLK_TCK_PORT->ODR & PIN_SWCLK_TCK) ? 1 : 0;
}

/** SWCLK/TCK I/O pin: Set Output to High.
Set the SWCLK/TCK DAP hardware I/O pin to high level.
*/
#pragma inline = forced
void PIN_SWCLK_TCK_SET(SWD_HANDLE * pswd)
{
  //PIN_SWCLK_TCK_PORT->BSRR = PIN_SWCLK_TCK;
  pswd->swclk_port->BSRR =PIN_MASK(pswd->swclk_pin);
}

/** SWCLK/TCK I/O pin: Set Output to Low.
Set the SWCLK/TCK DAP hardware I/O pin to low level.
*/
#pragma inline = forced
void PIN_SWCLK_TCK_CLR (SWD_HANDLE * pswd)
{
  //PIN_SWCLK_TCK_PORT->BSRR = (PIN_SWCLK_TCK<<16);
  pswd->swclk_port->BSRR =(PIN_MASK(pswd->swclk_pin))<<16;
}

// SWDIO/TMS Pin I/O --------------------------------------

/** SWDIO/TMS I/O pin: Get Input.
\return Current status of the SWDIO/TMS DAP hardware I/O pin.
*/
#pragma inline = forced
uint8_t PIN_SWDIO_TMS_IN(void)
{
  return (PIN_SWDIO_TMS_PORT->IDR & PIN_SWDIO_TMS) ? 1 : 0;
}

/** SWDIO/TMS I/O pin: Set Output to High.
Set the SWDIO/TMS DAP hardware I/O pin to high level.
*/
#pragma inline = forced
void PIN_SWDIO_TMS_SET(SWD_HANDLE * pswd)
{
  //PIN_SWDIO_TMS_PORT->BSRR = PIN_SWDIO_TMS;
  pswd->swdio_port->BSRR = PIN_MASK(pswd->swdio_pin);
}

/** SWDIO/TMS I/O pin: Set Output to Low.
Set the SWDIO/TMS DAP hardware I/O pin to low level.
*/
#pragma inline = forced
void PIN_SWDIO_TMS_CLR(SWD_HANDLE * pswd)
{
  //PIN_SWDIO_TMS_PORT->BSRR  = (PIN_SWDIO_TMS<<16);
  pswd->swdio_port->BSRR = (PIN_MASK(pswd->swdio_pin))<<16;
}

/** SWDIO I/O pin: Get Input (used in SWD mode only).
\return Current status of the SWDIO DAP hardware I/O pin.
*/
#pragma inline = forced
uint8_t PIN_SWDIO_IN (SWD_HANDLE * pswd)
{
//  if (PIN_SWDIO_TMS_PORT->IDR & PIN_SWDIO_TMS)
//    return 1;
//  return 0;
  
    if (pswd->swdio_port->IDR & PIN_MASK(pswd->swdio_pin))
    return 1;
  return 0;
}

/** SWDIO I/O pin: Set Output (used in SWD mode only).
\param bit Output value for the SWDIO DAP hardware I/O pin.
*/
#pragma inline = forced
void PIN_SWDIO_OUT(SWD_HANDLE * pswd, uint8_t bit)
{
//  if (bit & 1)
//    PIN_SWDIO_TMS_PORT->BSRR = PIN_SWDIO_TMS;
//  else
//    PIN_SWDIO_TMS_PORT->BSRR  = (PIN_SWDIO_TMS<<16);
  if (bit & 1)
    pswd->swdio_port->BSRR = PIN_MASK(pswd->swdio_pin);
  else
    pswd->swdio_port->BSRR = (PIN_MASK(pswd->swdio_pin))<<16; 
}

/** SWDIO I/O pin: Switch to Output mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to output mode. This function is
called prior \ref PIN_SWDIO_OUT function calls.
*/
#pragma inline = forced
void PIN_SWDIO_OUT_ENABLE(SWD_HANDLE * pswd)
{
  PIN_SWDIO_TMS_OUT_ENABLE(pswd);
}

/** SWDIO I/O pin: Switch to Input mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to input mode. This function is
called prior \ref PIN_SWDIO_IN function calls.
*/
#pragma inline = forced
void PIN_SWDIO_OUT_DISABLE(SWD_HANDLE * pswd)
{
  PIN_SWDIO_TMS_OUT_DISABLE(pswd);
}


// TDI Pin I/O ---------------------------------------------
#if ( DAP_JTAG != 0 )
/** TDI I/O pin: Get Input.
\return Current status of the TDI DAP hardware I/O pin.
*/
#pragma inline = forced
uint8_t PIN_TDI_IN(void)
{
  return (PIN_TDI_PORT->ODR & PIN_TDI) ? 1 : 0;
}

/** TDI I/O pin: Set Output.
\param bit Output value for the TDI DAP hardware I/O pin.
*/
#pragma inline = forced
void PIN_TDI_OUT(uint8_t bit)
{
  if (bit & 1)
    PIN_TDI_PORT->BSRR = PIN_TDI;
  else
    PIN_TDI_PORT->BRR  = PIN_TDI;
}


// TDO Pin I/O ---------------------------------------------

/** TDO I/O pin: Get Input.
\return Current status of the TDO DAP hardware I/O pin.
*/
#pragma inline = forced
uint8_t PIN_TDO_IN(void)
{
  return (PIN_TDO_PORT->IDR & PIN_TDO) ? 1 : 0;
}
#endif

// nTRST Pin I/O -------------------------------------------

/** nTRST I/O pin: Get Input.
\return Current status of the nTRST DAP hardware I/O pin.
*/
#pragma inline = forced
uint8_t PIN_nTRST_IN(void)
{
  return (0);   // Not available
}

/** nTRST I/O pin: Set Output.
\param bit JTAG TRST Test Reset pin status:
- 0: issue a JTAG TRST Test Reset.
- 1: release JTAG TRST Test Reset.
*/
#pragma inline = forced
void PIN_nTRST_OUT(uint8_t bit)
{
  
}

// nRESET Pin I/O------------------------------------------

/** nRESET I/O pin: Get Input.
\return Current status of the nRESET DAP hardware I/O pin.
*/
#pragma inline = forced
uint8_t PIN_nRESET_IN(void)
{
  if (PIN_nRESET_PORT->IDR & PIN_nRESET)
    return 1;
  return 0;
}

/** nRESET I/O pin: Set Output.
\param bit target device hardware reset pin status:
- 0: issue a device hardware reset.
- 1: release device hardware reset.
*/
void PIN_nRESET_OUT(SWD_HANDLE * pswd,uint8_t bit);

///@}


//**************************************************************************************************
/** 
\defgroup DAP_Config_Initialization_gr CMSIS-DAP Initialization
\ingroup DAP_ConfigIO_gr
@{

CMSIS-DAP Hardware I/O and LED Pins are initialized with the function \ref DAP_SETUP.
*/

/** Reset Target Device with custom specific I/O pin or command sequence.
This function allows the optional implementation of a device specific reset sequence.
It is called when the command \ref DAP_ResetTarget and is for example required 
when a device needs a time-critical unlock sequence that enables the debug port.
\return 0 = no device specific reset sequence is implemented.\n
1 = a device specific reset sequence is implemented.
*/
#pragma inline = forced
uint8_t RESET_TARGET(void)
{
  return (0); // change to '1' when a device reset sequence is implemented
}

///@}


#endif /* __DAP_CONFIG_H__ */

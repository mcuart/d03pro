/**
  ******************************************************************************
  * @file    DCMI/DCMI_CameraExample/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.8.1
  * @date    27-January-2022
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "cpu.h"
#include "usb_dcd_int.h"

#include "usb_hcd_int.h"
/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup DCMI_CameraExample
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint32_t PressedKey;

/* Private function prototypes -----------------------------------------------*/
extern USB_OTG_CORE_HANDLE USB_OTG_dev;
extern USB_OTG_CORE_HANDLE USB_OTG_Core_dev;



/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
//
//}



/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/******************************************************************************/
/**
  * @brief  This function handles SDIO global interrupt request.
  * @param  None
  * @retval None
  */
void SDIO_IRQHandler(void)
{

  /* Process All SDIO Interrupt Sources */
//SD_ProcessIRQSrc();

}

/**
  * @brief  This function handles DMA2 Stream3 or DMA2 Stream6 global interrupts
  *         requests.
  * @param  None
  * @retval None
  */
void SD_SDIO_DMA_IRQHANDLER(void)
{

  /* Process DMA2 Stream3 or DMA2 Stream6 Interrupt Sources */
//SD_ProcessDMAIRQ();

}




/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f40xx.s/startup_stm32f427x.s).                         */
/******************************************************************************/

/**
  * @brief  This function handles External line 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{

}




/* Definition for USARTx_IRQHANDLER *******************************************/
#if defined (USE_STM324xG_EVAL)
  #define USARTx_IRQHANDLER   USART3_IRQHandler
  
#endif /* USE_STM324xG_EVAL */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TXBUFFERSIZE   (countof(aTxBuffer) - 1)
#define RXBUFFERSIZE   0x20

/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Private variables ---------------------------------------------------------*/
uint8_t aTxBuffer[] = "\n\rUSART Hyperterminal Interrupts Example: USART-Hyperterminal\
 communication using Interrupt\n\r";
uint8_t aRxBuffer[RXBUFFERSIZE];
uint8_t ubNbrOfDataToTransfer = TXBUFFERSIZE;
uint8_t ubNbrOfDataToRead = RXBUFFERSIZE;
__IO uint8_t ubTxCounter = 0; 
__IO uint16_t uhRxCounter = 0; 
/**
  * @brief  This function handles USARTx global interrupt request.
  * @param  None
  * @retval None
  */
void USARTx_IRQHANDLER(void)
{
  
   //   OSIntEnter(); 
  
  if(USART_GetITStatus(COM_USART[COM_X], USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    aRxBuffer[uhRxCounter++] = (USART_ReceiveData(COM_USART[COM_X]) & 0x7F);

    if(uhRxCounter == ubNbrOfDataToRead)
    {
      /* Disable the EVAL_COM1 Receive interrupt */
      USART_ITConfig(COM_USART[COM_X], USART_IT_RXNE, DISABLE);
    }
  }

  if(USART_GetITStatus(COM_USART[COM_X], USART_IT_TXE) != RESET)
  {   
    /* Write one byte to the transmit data register */
    USART_SendData(COM_USART[COM_X], aTxBuffer[ubTxCounter++]);

    if(ubTxCounter == ubNbrOfDataToTransfer)
    {
      /* Disable the EVAL_COM1 Transmit interrupt */
      USART_ITConfig(COM_USART[COM_X], USART_IT_TXE, DISABLE);
    }
  }
  
     //   OSIntExit(); 
}



void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler(&USB_OTG_dev);
}


//扫码枪中断，只要扫码枪插上就会不停的进入这个中断
void OTG_HS_IRQHandler(void)
{
  USBH_OTG_ISR_Handler(&USB_OTG_Core_dev);
  GPIO_ToggleBits(GPIOB, GPIO_Pin_9);
}


/**
  * @}
  */ 

/**
  * @}
  */ 


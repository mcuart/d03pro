/**
  ******************************************************************************
  * @file    usbd_desc.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    17-March-2018
  * @brief   This file provides the USBD descriptors and string formatting method.
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

/* Includes ------------------------------------------------------------------ */
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include "usbd_conf.h"
#include "usb_regs.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_DESC 
  * @brief USBD descriptors module
  * @{
  */

/** @defgroup USBD_DESC_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_DESC_Private_Defines
  * @{
  */
#define USBD_VID                        0x0483
//#define USBD_PID                        0x3256
#define USBD_PID                        0x5758
/** @defgroup USB_String_Descriptors
  * @{
  */
#define USBD_LANGID_STRING              0x409
#define USBD_MANUFACTURER_STRING        "STMicroelectronics"
#define USBD_PRODUCT_HS_STRING          "Composite HID CDC"
#define USBD_PRODUCT_FS_STRING          "Composite HID CDC"
#define USBD_CONFIGURATION_HS_STRING    "VCP Config"
#define USBD_INTERFACE_HS_STRING        "VCP Interface"

#define USBD_CONFIGURATION_FS_STRING    "VCP Config"
#define USBD_INTERFACE_FS_STRING        "VCP Interface"
/**
  * @}
  */


/** @defgroup USBD_DESC_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_DESC_Private_Variables
  * @{
  */

USBD_DEVICE USR_desc = {
  USBD_USR_DeviceDescriptor,
  USBD_USR_LangIDStrDescriptor,
  USBD_USR_ManufacturerStrDescriptor,
  USBD_USR_ProductStrDescriptor,
  USBD_USR_SerialStrDescriptor,
  USBD_USR_ConfigStrDescriptor,
  USBD_USR_InterfaceStrDescriptor,
  //添加MS OS 1.0的返回描述符函数指针
  USBD_USR_OSStrDescriptor,
  USBD_USR_ExtPropertiesFeatureDescriptor,
  USBD_USR_ExtCompatIDFeatureDescriptor
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ )      /* !< IAR Compiler */
#pragma data_alignment=4
#endif
#endif                          /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_DeviceDesc[USB_SIZ_DEVICE_DESC] __ALIGN_END = {
  0x12,                         /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  0x00,                         /* bcdUSB */
  0x02,
//  0xEF,                         /* bDeviceClass */
//  0x02,                         /* bDeviceSubClass */
//  0x01,                         /* bDeviceProtocol */
  //0xFF,                         /* bDeviceClass */
  0x00,                         /* 此处bDeviceClass必须为0x00才能同时枚举出来WINUSB和cunstomhid如果为0xFF则只能枚举出来WINUSB */
  0x00,                         /* bDeviceSubClass */
  0x00,                         /* bDeviceProtocol */  
  USB_OTG_MAX_EP0_SIZE,         /* bMaxPacketSize */
  LOBYTE(USBD_VID),             /* idVendor */
  HIBYTE(USBD_VID),             /* idVendor */
  LOBYTE(USBD_PID),             /* idVendor */
  HIBYTE(USBD_PID),             /* idVendor */
  0x00,                         /* bcdDevice rel. 2.00 */
  0x02,
  USBD_IDX_MFC_STR,             /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,         /* Index of product string */
  USBD_IDX_SERIAL_STR,          /* Index of serial number string */
  USBD_CFG_MAX_NUM              /* bNumConfigurations */
};                              /* USB_DeviceDescriptor */

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ )      /* !< IAR Compiler */
#pragma data_alignment=4
#endif
#endif                          /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC]
  __ALIGN_END = {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ )      /* !< IAR Compiler */
#pragma data_alignment=4
#endif
#endif                          /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
/* USB Standard Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_SIZ_STRING_LANGID] __ALIGN_END = {
  USB_SIZ_STRING_LANGID,
  USB_DESC_TYPE_STRING,
  LOBYTE(USBD_LANGID_STRING),
  HIBYTE(USBD_LANGID_STRING),
};

uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] = {
  USB_SIZ_STRING_SERIAL,
  USB_DESC_TYPE_STRING,
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#if defined ( __ICCARM__ )      /* !< IAR Compiler */
#pragma data_alignment=4
#endif
#endif                          /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USB_MAX_STR_DESC_SIZ] __ALIGN_END;

/**
  * @}
  */


/** @defgroup USBD_DESC_Private_FunctionPrototypes
  * @{
  */
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);
static void Get_SerialNum(void);
/**
  * @}
  */


/** @defgroup USBD_DESC_Private_Functions
  * @{
  */

/**
* @brief  USBD_USR_DeviceDescriptor 
*         return the device descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_USR_DeviceDescriptor(uint8_t speed, uint16_t * length)
{
  *length = sizeof(USBD_DeviceDesc);
  return (uint8_t *) USBD_DeviceDesc;
}

/**
* @brief  USBD_USR_LangIDStrDescriptor 
*         return the LangID string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_USR_LangIDStrDescriptor(uint8_t speed, uint16_t * length)
{
  *length = sizeof(USBD_LangIDDesc);
  return (uint8_t *) USBD_LangIDDesc;
}


/**
* @brief  USBD_USR_ProductStrDescriptor 
*         return the product string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_USR_ProductStrDescriptor(uint8_t speed, uint16_t * length)
{
  if (speed == 0)
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_PRODUCT_HS_STRING, USBD_StrDesc,
                   length);
  }
  else
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_PRODUCT_FS_STRING, USBD_StrDesc,
                   length);
  }
  return USBD_StrDesc;
}

/**
* @brief  USBD_USR_ManufacturerStrDescriptor 
*         return the manufacturer string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_USR_ManufacturerStrDescriptor(uint8_t speed, uint16_t * length)
{
  USBD_GetString((uint8_t *) (uint8_t *) USBD_MANUFACTURER_STRING, USBD_StrDesc,
                 length);
  return USBD_StrDesc;
}

/**
* @brief  USBD_USR_SerialStrDescriptor 
*         return the serial number string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_USR_SerialStrDescriptor(uint8_t speed, uint16_t * length)
{
  *length = USB_SIZ_STRING_SERIAL;

  /* Update the serial number string descriptor with the data from the unique
   * ID */
  Get_SerialNum();

  return (uint8_t *) USBD_StringSerial;
}

/**
* @brief  USBD_USR_ConfigStrDescriptor 
*         return the configuration string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_USR_ConfigStrDescriptor(uint8_t speed, uint16_t * length)
{
  if (speed == USB_OTG_SPEED_HIGH)
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_CONFIGURATION_HS_STRING,
                   USBD_StrDesc, length);
  }
  else
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_CONFIGURATION_FS_STRING,
                   USBD_StrDesc, length);
  }
  return USBD_StrDesc;
}


/**
* @brief  USBD_USR_InterfaceStrDescriptor 
*         return the interface string descriptor
* @param  speed : current device speed
* @param  length : pointer to data length variable
* @retval pointer to descriptor buffer
*/
uint8_t *USBD_USR_InterfaceStrDescriptor(uint8_t speed, uint16_t * length)
{
  if (speed == 0)
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_INTERFACE_HS_STRING,
                   USBD_StrDesc, length);
  }
  else
  {
    USBD_GetString((uint8_t *) (uint8_t *) USBD_INTERFACE_FS_STRING,
                   USBD_StrDesc, length);
  }
  return USBD_StrDesc;
}

/**
  * @brief  Create the serial number string descriptor 
  * @param  None 
  * @retval None
  */
static void Get_SerialNum(void)
{
  uint32_t deviceserial0, deviceserial1, deviceserial2;

  deviceserial0 = *(uint32_t *) DEVICE_ID1;
  deviceserial1 = *(uint32_t *) DEVICE_ID2;
  deviceserial2 = *(uint32_t *) DEVICE_ID3;

  deviceserial0 += deviceserial2;

  if (deviceserial0 != 0)
  {
    IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
    IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
  }
}

/**
  * @brief  Convert Hex 32Bits value into char 
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer 
  * @param  len: buffer length
  * @retval None
  */
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len)
{
  uint8_t idx = 0;

  for (idx = 0; idx < len; idx++)
  {
    if (((value >> 28)) < 0xA)
    {
      pbuf[2 * idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2 * idx] = (value >> 28) + 'A' - 10;
    }

    value = value << 4;

    pbuf[2 * idx + 1] = 0;
  }
}

__ALIGN_BEGIN uint8_t Composite_StringWinUSB[Composite_SIZ_STRING_WINUSB] __ALIGN_END = 
{
	Composite_SIZ_STRING_WINUSB,
	USB_DESC_TYPE_STRING,
	'M',0,'S',0,'F',0,'T',0,'1',0,'0',0,'0',0,
	USB_REQ_GET_OS_FEATURE_DESCRIPTOR
};
__ALIGN_BEGIN uint8_t USBD_CompatIDDesc[CUSTOM_SIZ_STRING_IDDesc]  __ALIGN_END = 
{
	///
	/// WCID descriptor
	///		
	0x28,0x00,0x00,0x00,											/* dwLength */
	0x00,0x01, 																/* bcdVersion */
	0x04,0x00, 																/* wIndex */
	0x01,																			/* bCount */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,				/* bReserved_7 */
	///
	/// WCID function descriptor
	///		
	0x00, 																		/* bFirstInterfaceNumber */
	0x01, 																		/* bReserved */
	/*WINUSB*/
	'W','I','N','U','S','B',0x00,0x00, 				/* cCID_8 */
	/*  */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  /* cSubCID_8 */
	0x00,0x00,0x00,0x00,0x00,0x00,            /* bReserved_6 */
};
	
__ALIGN_BEGIN uint8_t USBD_ExtPropertiesDesc[0x8E]  __ALIGN_END = 
{ 
//	///
//	/// WCID property descriptor
//	///
//	0x8e, 0x00, 0x00, 0x00,                           /* dwLength */
//	0x00, 0x01,                                       /* bcdVersion */
//	0x05, 0x00,                                       /* wIndex */
//	0x01, 0x00,                                       /* wCount */
//	///
//	/// registry propter descriptor
//	///
//	0x84, 0x00, 0x00, 0x00,                           /* dwSize */
//	0x01, 0x00, 0x00, 0x00,                           /* dwPropertyDataType */
//	0x28, 0x00,                                       /* wPropertyNameLength */
//	/* DeviceInterfaceGUID */
//	'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00,       /* wcName_20 */
//	'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00,       /* wcName_20 */
//	't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00,       /* wcName_20 */
//	'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00,       /* wcName_20 */
//	'U', 0x00, 'I', 0x00, 'D', 0x00, 0x00, 0x00,      /* wcName_20 */
//	0x4e, 0x00, 0x00, 0x00,                           /* dwPropertyDataLength */
//	/* {697A17F1-6ED1-4176-A9CE-0864804669B6} */
//	'{', 0x00, '6', 0x00, '9', 0x00, '7', 0x00,       /* wcData_39 */
//	'A', 0x00, '1', 0x00, '7', 0x00, 'F', 0x00,       /* wcData_39 */
//	'1', 0x00, '-', 0x00, '6', 0x00, 'E', 0x00,       /* wcData_39 */
//	'D', 0x00, '1', 0x00, '-', 0x00, '4', 0x00,       /* wcData_39 */
//	'1', 0x00, '7', 0x00, '6', 0x00, '-', 0x00,       /* wcData_39 */
//	'A', 0x00, '9', 0x00, 'C', 0x00, 'E', 0x00,       /* wcData_39 */
//	'-', 0x00, '0', 0x00, '8', 0x00, '6', 0x00,       /* wcData_39 */
//	'4', 0x00, '8', 0x00, '0', 0x00, '4', 0x00,       /* wcData_39 */
//	'6', 0x00, '6', 0x00, '9', 0x00, 'B', 0x00,       /* wcData_39 */
//	'6', 0x00, '}', 0x00, 0x00, 0x00,                 /* wcData_39 */
      0x8E, 0, 0, 0,  // length 246 byte
      0x00, 0x01,   // BCD version 1.0
      0x05, 0x00,   // Extended Property Descriptor Index(5)
      0x01, 0x00,   // number of section (1)
//; property section        
      0x84, 0x00, 0x00, 0x00,   // size of property section
      0x1, 0, 0, 0,   //; property data type (1)
      0x28, 0,        //; property name length (42)
      'D', 0,
      'e', 0,
      'v', 0,
      'i', 0,
      'c', 0,
      'e', 0,
      'I', 0,
      'n', 0,
      't', 0,
      'e', 0,
      'r', 0,
      'f', 0,
      'a', 0,
      'c', 0,
      'e', 0,
      'G', 0,
      'U', 0,
      'I', 0,
      'D', 0,
      0, 0,
      // D6805E56-0447-4049-9848-46D6B2AC5D28
      0x4E, 0, 0, 0,  // ; property data length
      '{', 0,
      '1', 0,
      '3', 0,
      'E', 0,
      'B', 0,
      '3', 0,
      '6', 0,
      '0', 0,
      'B', 0,
      '-', 0,
      'B', 0,
      'C', 0,
      '1', 0,
      'E', 0,
      '-', 0,
      '4', 0,
      '6', 0,
      'C', 0,
      'B', 0,
      '-', 0,
      'A', 0,
      'C', 0,
      '8', 0,
      'B', 0,
      '-', 0,
      'E', 0,
      'F', 0,
      '3', 0,
      'D', 0,
      'A', 0,
      '4', 0,
      '7', 0,
      'B', 0,
      '4', 0,
      '0', 0,
      '6', 0,
      '2', 0,
      '}', 0,
      0, 0,  
};



uint8_t *  USBD_USR_OSStrDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(Composite_StringWinUSB);
  return Composite_StringWinUSB;  
}
 
uint8_t *  USBD_USR_ExtPropertiesFeatureDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(USBD_ExtPropertiesDesc);
  return (uint8_t*)&USBD_ExtPropertiesDesc;  
}
 
uint8_t *  USBD_USR_ExtCompatIDFeatureDescriptor( uint8_t speed , uint16_t *length)
{
  *length =  sizeof(USBD_CompatIDDesc);
  return (uint8_t*)&USBD_CompatIDDesc;  
}
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**************************************************************************//**
* @file upload_flashloader.c
* @brief Handles programming with help of a flashloader
* @author Energy Micro AS
* @version 1.02
******************************************************************************
* @section License
* <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
* 4. The source and compiled code may only be used on Energy Micro "EFM32"
*    microcontrollers and "EFR4" radios.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
* obligation to support this Software. Energy Micro AS is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Energy Micro AS will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
*****************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include "dap.h"
#include "flashloader.h"
#include "utils.h"
#include "use_flashloader.h"



#include "GUI.h"

#include "config.h"
#include "aes.h"
#include  <includes.h>






//extern cJSON *root;

extern u8 mh;

#define RAM_START 0x20000000

/* Initializes a global flashLoaderState object 
* which is placed at the same location in memory 
* as in the flashloader itself. Using this
* we can easily get the address of each of the fields */
flashLoaderState_TypeDef *flState = (flashLoaderState_TypeDef *)STATE_LOCATION;
flashLoaderState_TypeDef *flState_F2_3_4 = (flashLoaderState_TypeDef *)STATE_LOCATION;
flashLoaderState_TypeDef *flState_F7 = (flashLoaderState_TypeDef *)0x20002000;
flashLoaderState_TypeDef *flState_F0_1 = (flashLoaderState_TypeDef *)0x20000A00;
flashLoaderState_TypeDef *flState_L011 = (flashLoaderState_TypeDef *)0x20000520;
//flashLoaderState_TypeDef *flState_L011 = (flashLoaderState_TypeDef *)0x20000A00;
flashLoaderState_TypeDef *flState_efm32 = (flashLoaderState_TypeDef *)0x20001000;




extern u32 USB_RequestOut;        // Request  Buffer Out Index
extern u32 USB_ResponseIn;        // Response Buffer In  Index
extern uint8_t  USB_Request [DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
extern uint8_t  USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Response Buffer
u32 tmp_v1=0;
u32 tmpB[10];

extern u8 global_error_flag;

static void writeToFlashloaderBuffer(SWD_HANDLE * pswd, uint32_t remoteAddr, uint32_t *localBuffer, u32 numWords);
static u8 writePagesWithFlashloader(SWD_HANDLE * pswd);
static u8 sendWriteOptionByteCmd(SWD_HANDLE * pswd , uint32_t addr,u32 numBytes);
static u8 sendWriteRollingCodeCmd(SWD_HANDLE * pswd);
static u8 sendWriteCodeBarCmd(SWD_HANDLE * pswd);
static u32 JiaMiSuanFa(u8 gongshi,u32 changshu, u8 D[]);
static u8 STM32_WriteUniqueIDJiaMi(SWD_HANDLE * pswd);
static u8 sendwritePageCmd(SWD_HANDLE *pswd, u32 block_start_address,u32 block_size);
/**********************************************************
* Uploads and runs the flashloader on the target
* The flashloader is written directly to the start
* of RAM. Then the PC and SP are loaded from the
* flashloader image. 
**********************************************************/
u8 uploadFlashloader(SWD_HANDLE *pswd, uint32_t *flImage, uint32_t size)
{
  u32 tmp_v=0;
  u32 w;
  uint32_t addr;
  uint32_t tarWrap;
  uint32_t numWords;
  
  
  numWords = size / 4;
  
  if ( numWords * 4 < size ) numWords++;
  
  if(resetAndHaltTarget(pswd)!=0)
  {
    return 1;
  }
  /* Get the TAR wrap-around period */
  //tarWrap = 0x000003FF;
  tarWrap = 0x0000007F;
  /* Enable autoincrement on TAR */
  //  Write_AP_CSW(AP_CSW_DEFAULT | AP_CSW_AUTO_INCREMENT|0x01000000); 
  Write_AP_CSW(pswd, AP_CSW_DEFAULT | AP_CSW_AUTO_INCREMENT); 
  for ( w=0; w<numWords; w++ ) 
  {
    /* Get address of current word */
    addr = RAM_START + w * 4;
    
    /* At the TAR wrap boundary we need to reinitialize TAR
    * since the autoincrement wraps at these */
    if ( (addr & tarWrap) == 0 )
    {
      writeAP_TAR(pswd, addr);
    }
    
    writeAP_data(pswd, flImage[w]);
    
  }
  
  
  for ( w=0; w<numWords; w++ ) 
  {
    /* Get address of current word */
    addr = RAM_START + w * 4;
    
    /* At the TAR wrap boundary we need to reinitialize TAR
    * since the autoincrement wraps at these */
    
    tmp_v=readMem(pswd, addr);
    if(tmp_v!=flImage[w])
    {
      tmp_v1=tmp_v;
    }
    
  }
  
  
  
  Write_AP_CSW(pswd, AP_CSW_DEFAULT);
  /* Load SP (Reg 13) from flashloader image */
  if(writeCpuReg(pswd, 13, flImage[0])!=0)
  {
    return 2;
  }
  
  /* Load PC (Reg 15) from flashloader image */
  if(writeCpuReg(pswd, 15, flImage[1])!=0)
  {
    return 3;
  }
  
  /*加载 XPSR防止因为flash中已经有零数据单片机进入arm模式，实际应该用thumb模式*/
  if(writeCpuReg(pswd, 16, 0x01000000)!=0)
  {
    return 4;
  }
  
  runTarget(pswd);
  
  
  return 0;
  
  
}


/**********************************************************
* Verifies that the flashloader is ready. Will throw an
* exception if the flashloader is not ready within a
* timeout. 
**********************************************************/
u32 verifyFlashloaderReady(SWD_HANDLE *pswd)
{
  uint32_t status;
  
  uint32_t timeout = FLASHLOADER_RETRY_COUNT;
  
  //  do {
  //    System_Delay_ms(5);
  //    status = readMem( (uint32_t)&(flState->flashLoaderStatus) );
  //    timeout--;
  //  } while ( status == FLASHLOADER_STATUS_NOT_READY  && timeout > 0 );
  //  
  
  do {
    //System_Delay_ms(5);
    // delay_ms(5);
    delay_ms(1);
    status = readMem(pswd, (uint32_t)&(flState->flashLoaderStatus));
    timeout--;
    if(global_error_flag!=0)
    {
      return 1;
    }
  } while ( status != FLASHLOADER_STATUS_READY  && timeout > 0 );
  
  
  if ( status == FLASHLOADER_STATUS_READY ) {
    // printf("Flashloader ready\n");
    return 0;
  } else {
    //printf("Flashloader not ready. Status: 0x%.8x\n", status);
    // RAISE(SWD_ERROR_FLASHLOADER_ERROR);
    return status ;
  }
}
/* Number of times to wait for flashloader */
//#define FLASHLOADER_RETRY_COUNT 1000
#define FLASHLOADER_RETRY_COUNT 4000
/**********************************************************
* Waits until the flashloader reports that it is ready
**********************************************************/
u8 waitForFlashloader(SWD_HANDLE *pswd, u32 retry)
{ 
  uint32_t status;
  u32 tmp_retry=retry;
  
  /* Wait until flashloader has acknowledged the command */
  do {
    //这里不能加延时，加了延时反而会卡死
    status = readMem(pswd, (uint32_t)&(flState->debuggerStatus));
    retry--;
    if(global_error_flag!=0)
    {
      return 1;
    }
  } while ( status != DEBUGGERCMD_NONE && retry > 0 );
  
  /* Wait until command has completed */
  retry = tmp_retry;
  do {
    //  delayUs(500);
    //  System_Delay_ms(10);
    delay_ms(1); 
    OS_ERR err;
    status = readMem(pswd, (uint32_t)&(flState->flashLoaderStatus));
    retry--;
    if(global_error_flag!=0)
    {
      return 2;
    }
  } while ( status == FLASHLOADER_STATUS_NOT_READY && retry > 0 );
  
  /* Raise an error if we timed out or flashloader has reported an error */
  if ( status == FLASHLOADER_STATUS_NOT_READY ) 
  {
    //    printf("Timed out while waiting for flashloader\n");
    //    RAISE(SWD_ERROR_FLASHLOADER_ERROR);
    return 3;
  } 
  else if ( status != FLASHLOADER_STATUS_READY ) 
  {
    //    printf("Flashloader returned error code %d\n", status);
    //    RAISE(SWD_ERROR_FLASHLOADER_ERROR);
    return 2;
  }
  return 0;
}


u8 waitForFlashloader_longtime(SWD_HANDLE *pswd, u32 retry)
{ 
  uint32_t status;
  u32 retry_store;
  retry_store=retry;
  /* Wait until flashloader has acknowledged the command */
  do {
    //这里不能加延时，加了延时反而会卡死
    status = readMem(pswd, (uint32_t)&(flState->debuggerStatus));
    retry--;
    if(global_error_flag!=0)
    {
      return 1;
    }
  } while ( status != DEBUGGERCMD_NONE && retry > 0 );
  
  /* Wait until command has completed */
  retry = retry_store;
  do {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
    //  delayUs(500);
    //    System_Delay_ms(10);
    delay_ms(10);    
    
    status = readMem(pswd, (uint32_t)&(flState->flashLoaderStatus));
    retry--;
    if(global_error_flag!=0)
    {
      return 2;
    }
  } while ( status != FLASHLOADER_STATUS_READY && retry > 0 );//这里因为MM32F031改进代替下一句
  //} while ( status == FLASHLOADER_STATUS_NOT_READY && retry > 0 );
  
  /* Raise an error if we timed out or flashloader has reported an error */
  if ( status == FLASHLOADER_STATUS_NOT_READY ) 
  {
    //    printf("Timed out while waiting for flashloader\n");
    //    RAISE(SWD_ERROR_FLASHLOADER_ERROR);
    return 3;
  } 
  else if ( status != FLASHLOADER_STATUS_READY ) 
  {
    //    printf("Flashloader returned error code %d\n", status);
    //    RAISE(SWD_ERROR_FLASHLOADER_ERROR);
    return 2;
  }
  return 0;
}



/**********************************************************
* Tells the flashloader to erase one page at the 
* given address.
**********************************************************/
u8 sendErasePageCmd(SWD_HANDLE *pswd, uint32_t addr,uint32_t block_size)
{
  writeMem(pswd, (uint32_t)&(flState->writeAddress1), addr);
  writeMem(pswd, (uint32_t)&(flState->numBytes1), block_size);
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_ERASE_PAGE);
  
  if(waitForFlashloader_longtime(pswd, 1600+800+800)!=0)//最大16秒等待时间
  {
    return 1;
  }
  return 0;
}

/**********************************************************
* Tells the flashloader to erase the entire flash
**********************************************************/
u8 sendMassEraseCmd(SWD_HANDLE *pswd)
{
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_MASS_ERASE);
  
  if(waitForFlashloader_longtime(pswd, 1600+800+800)!=0)//最大16秒等待时间
  {
    return 1;
  }
  
  return 0;
}

u8 TargetValEmpty=0xFF;

u8 sendWriteRollingCodeCmd(SWD_HANDLE * pswd)
{
  u32 stm32_rolling_code_value_array[1]={0};
  u8 rolling_code_array[4]={0,0,0,0};
  
  rolling_code_array[0]=cJSON_GetObjectItem(pswd->root, "rolling_code")->valuedouble;
  rolling_code_array[1]=((u32)cJSON_GetObjectItem(pswd->root, "rolling_code")->valuedouble)>>8;
  rolling_code_array[2]=((u32)cJSON_GetObjectItem(pswd->root, "rolling_code")->valuedouble)>>16;
  rolling_code_array[3]=((u32)cJSON_GetObjectItem(pswd->root, "rolling_code")->valuedouble)>>24;
  
  
  
  switch(TargetValEmpty)
  {
  case 0xFF:
    {
      if((u8)cJSON_GetObjectItem(pswd->root, "rolling_code_endian")->valueint==1)//大端模式
      {
        switch(cJSON_GetObjectItem(pswd->root, "rolling_code_byte_width")->valueint)
        {
        case 1:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]|0xFFFFFF00;
          break;
        case 2:
          stm32_rolling_code_value_array[0]= rolling_code_array[1]<<0|rolling_code_array[0]<<8|0xFFFF0000;
          break;
        case 3:
          stm32_rolling_code_value_array[0]= rolling_code_array[2]<<0|rolling_code_array[1]<<8|rolling_code_array[0]<<16|0xFF000000;
          break;
        case 4:
          stm32_rolling_code_value_array[0]= rolling_code_array[3]<<0|rolling_code_array[2]<<8|rolling_code_array[1]<<16|rolling_code_array[0]<<24;      
          break;
        }
      }
      else//小端模式
      {
        switch(cJSON_GetObjectItem(pswd->root, "rolling_code_byte_width")->valueint)
        {
        case 1:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]|0xFFFFFF00;
          break;
        case 2:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]|rolling_code_array[1]<<8|0xFFFF0000;
          break;
        case 3:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]|rolling_code_array[1]<<8|rolling_code_array[2]<<16|0xFF000000;
          break;
        case 4:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]|rolling_code_array[1]<<8|rolling_code_array[2]<<16|rolling_code_array[3]<<24;      
          break;
        }
      }
      
    }
    break;
    
  case 0x00:
    {
      if((u8)cJSON_GetObjectItem(pswd->root, "rolling_code_endian")->valueint==1)//大端模式
      {
        switch(cJSON_GetObjectItem(pswd->root, "rolling_code_byte_width")->valueint)
        {
        case 1:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]&0x000000FF;
          break;
        case 2:
          stm32_rolling_code_value_array[0]= (rolling_code_array[1]<<0|rolling_code_array[0]<<8)&0x0000FFFF;
          break;
        case 3:
          stm32_rolling_code_value_array[0]= (rolling_code_array[2]<<0|rolling_code_array[1]<<8|rolling_code_array[0]<<16)&0x00FFFFFF;
          break;
        case 4:
          stm32_rolling_code_value_array[0]= rolling_code_array[3]<<0|rolling_code_array[2]<<8|rolling_code_array[1]<<16|rolling_code_array[0]<<24;      
          break;
        }
      }
      else//小端模式
      {
        switch(cJSON_GetObjectItem(pswd->root, "rolling_code_byte_width")->valueint)
        {
        case 1:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]&0x000000FF;
          break;
        case 2:
          stm32_rolling_code_value_array[0]= (rolling_code_array[0]|rolling_code_array[1]<<8)&0x0000FFFF;
          break;
        case 3:
          stm32_rolling_code_value_array[0]= (rolling_code_array[0]|rolling_code_array[1]<<8|rolling_code_array[2]<<16)&0x00FFFFFF;
          break;
        case 4:
          stm32_rolling_code_value_array[0]= rolling_code_array[0]|rolling_code_array[1]<<8|rolling_code_array[2]<<16|rolling_code_array[3]<<24;      
          break;
        }
      }
      
    } 
    break;
  }
  
  
  
  uint32_t bufferLocation1 = readMem(pswd, (uint32_t)&(flState->bufferAddress1) );  
  writeToFlashloaderBuffer(pswd, bufferLocation1,stm32_rolling_code_value_array,1);
  
  
  writeMem(pswd, (uint32_t)&(flState->writeAddress1), (u32)cJSON_GetObjectItem(pswd->root, "rolling_code_address")->valuedouble);
  writeMem(pswd, (uint32_t)&(flState->numBytes1), 4);
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA1);
  
  if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
  {
    return 1;
  }
  
  return 0;
}




extern u8 bar_code[64];

u8 sendWriteCodeBarCmd(SWD_HANDLE * pswd)
{
  static u8 code_bar_array[128];//20250515二维码内容为16个ASCII字符占用128位
  u8 byte_width=0;
  byte_width=cJSON_GetObjectItem(pswd->root, "code_bar_bit_width")->valueint;
  
  
  switch(TargetValEmpty)
  {
  case 0xFF:
    {
      for(u8 i=0;i<128;i++)
      {
        code_bar_array[i]=0xFF;
      }
      
      if((u8)cJSON_GetObjectItem(pswd->root, "code_bar_endian")->valueint==1)//大端模式
      {
          for(u8 i=0;i<byte_width;i++)
          {
            code_bar_array[i]= bar_code[i];           
          }
      }
      else//小端模式
      {
          for(u8 i=0;i<byte_width;i++)
          {
            code_bar_array[i]= bar_code[byte_width-1-i];          
          }
      }
      
      
      if(byte_width%4)//不是4字节的倍数
      {
        byte_width=(byte_width/4 + 1)*4;
      }
      
      
      
    }
    break;
    
  case 0x00:
    {
      for(u8 i=0;i<128;i++)
      {
        code_bar_array[i]=0x00;
      }
      
    } 
    break;
  }
  
  
  
  uint32_t bufferLocation1 = readMem(pswd, (uint32_t)&(flState->bufferAddress1) );  
  writeToFlashloaderBuffer(pswd, bufferLocation1,(u32 *)code_bar_array,byte_width/4);
  
  
  writeMem(pswd, (uint32_t)&(flState->writeAddress1), (u32)cJSON_GetObjectItem(pswd->root, "code_bar_store_address")->valuedouble);
  writeMem(pswd, (uint32_t)&(flState->numBytes1), byte_width);
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA1);
  
  if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
  {
    return 1;
  }
  
  
  
  
  switch(TargetValEmpty)
  {
  case 0xFF:
    for(u8 i=0;i<128;i++)
    {
      code_bar_array[i]=0xFF;
    }
    code_bar_array[2]=byte_width;
    code_bar_array[3]=byte_width>>8;
    
    
    break;
    
    
  case 00:
    for(u8 i=0;i<128;i++)
    {
      code_bar_array[i]=0x00;
    }
    break;
  }
  
  uint32_t bufferLocation2 = readMem(pswd, (uint32_t)&(flState->bufferAddress2) );  
  writeToFlashloaderBuffer(pswd, bufferLocation2,(u32 *)code_bar_array,1);//1个32位数据
  
  
  writeMem(pswd, (uint32_t)&(flState->writeAddress2), ((u32)cJSON_GetObjectItem(pswd->root, "code_bar_store_address")->valuedouble)-4);
  writeMem(pswd, (uint32_t)&(flState->numBytes2), 4);//写4个字节
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA2);
  
  if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
  {
    return 1;
  }  
  
  
  return 0;
}


static u8 STM32_WriteUniqueIDJiaMi(SWD_HANDLE * pswd)
{
  u32 tmp;
  u32 stm32_jiami_id_code_value_array[1]={0};
  u8 JiaMiID[4]={0,0,0,0};
  u8 UID_array[12];//UID最大是12个字节
  u8 UID_array_normal[12];//UID最大是12个字节
  u8 gongshi_index=(u8)cJSON_GetObjectItem(pswd->root, "custom_secret_gongshi")->valueint;
  u32 changshu=(u32)cJSON_GetObjectItem(pswd->root, "custom_secret_changshu")->valuedouble;
  u32 jiami_id_write_address=(u32)cJSON_GetObjectItem(pswd->root, "custom_secret_address")->valuedouble;//自定义加密ID最终写入存放地址
  
  /////////////////////////////////////读出  
  
  //先读取unique id根据公式计算出加密结果-start
  tmp = readMem(pswd, (u32)cJSON_GetObjectItem(pswd->root, "unique_id_address")->valuedouble );//到指定的位置读取unique id 
  UID_array_normal[0]=tmp;
  UID_array_normal[1]=tmp>>8;  
  UID_array_normal[2]=tmp>>16;  
  UID_array_normal[3]=tmp>>24;  
  
  tmp = readMem(pswd, (u32)cJSON_GetObjectItem(pswd->root, "unique_id_address")->valuedouble +4);//到指定的位置读取unique id
  UID_array_normal[4]=tmp;
  UID_array_normal[5]=tmp>>8;  
  UID_array_normal[6]=tmp>>16;  
  UID_array_normal[7]=tmp>>24;  
  
  tmp = readMem(pswd, (u32)cJSON_GetObjectItem(pswd->root, "unique_id_address")->valuedouble +8 );//到指定的位置读取unique id
  UID_array_normal[8]=tmp;
  UID_array_normal[9]=tmp>>8;  
  UID_array_normal[10]=tmp>>16;  
  UID_array_normal[11]=tmp>>24;    
  
  
  for(u8 i=0;i<12;i++)
  {
    UID_array[i]=UID_array_normal[(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "custom_secret_dat"),i)->valuedouble];
  }
  
  
  /////////////////////////////////////计算  
  tmp=JiaMiSuanFa(gongshi_index,changshu,UID_array);//buf[123];//公式序号
  JiaMiID[0]=tmp;
  JiaMiID[1]=tmp>>8;
  JiaMiID[2]=tmp>>16;
  JiaMiID[3]=tmp>>24;
  
  
  switch(TargetValEmpty)
  {
  case 0xFF:
    {
      //先读取unique id根据公式计算出加密结果-end
      switch(cJSON_GetObjectItem(pswd->root, "custom_secret_zijieshu")->valueint)//存储字节数(小端模式)
      {
      case 1:
        stm32_jiami_id_code_value_array[0]= JiaMiID[0]|0xFFFFFF00; //存储字节数=1      
        break;
      case 2:
        stm32_jiami_id_code_value_array[0]= JiaMiID[1]<<8|JiaMiID[0]|0xFFFF0000;    //存储字节数=2
        break;
      case 3:
        stm32_jiami_id_code_value_array[0]= JiaMiID[2]<<16|JiaMiID[1]<<8|JiaMiID[0]|0xFF000000;      //存储字节数=3
        break;
      case 4:
        stm32_jiami_id_code_value_array[0]= JiaMiID[3]<<24|JiaMiID[2]<<16|JiaMiID[1]<<8|JiaMiID[0];     //存储字节数=4 
        break;
      }       
    }
    break;
    
  case 0x00:
    {
      //先读取unique id根据公式计算出加密结果-end
      switch(cJSON_GetObjectItem(pswd->root, "custom_secret_zijieshu")->valueint)//存储字节数(小端模式)
      {
      case 1:
        stm32_jiami_id_code_value_array[0]= JiaMiID[0]&0x000000FF; //存储字节数=1  
        break;
      case 2:
        stm32_jiami_id_code_value_array[0]= JiaMiID[1]<<8|JiaMiID[0]&0x0000FFFF;    //存储字节数=2
        break;
      case 3:
        stm32_jiami_id_code_value_array[0]= JiaMiID[2]<<16|JiaMiID[1]<<8|JiaMiID[0]&0x00FFFFFF;      //存储字节数=3
        break;
      case 4:
        stm32_jiami_id_code_value_array[0]= JiaMiID[3]<<24|JiaMiID[2]<<16|JiaMiID[1]<<8|JiaMiID[0];     //存储字节数=4 
        break;
      }   
    } 
    break;
  }
  
  
  
  
  /////////////////////////////////////写入 
  //将加密结果按照字节逐个写入指定的地址-start
  
  uint32_t bufferLocation1 = readMem(pswd, (uint32_t)&(flState->bufferAddress1) );//使用FLASH_LOADER获取目标芯片中RAM buffer的地址  
  writeToFlashloaderBuffer(pswd, bufferLocation1,stm32_jiami_id_code_value_array,1);//将计算出来的加密结果写入目标芯片的RAM buffer中
  
  
  writeMem(pswd, (uint32_t)&(flState->writeAddress1), jiami_id_write_address);
  writeMem(pswd, (uint32_t)&(flState->numBytes1), 4);
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA1);
  
  if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
  {
    return 1;
  }
  
  //将加密结果按照字节逐个写入指定的地址-end
  
  return 0;
  
}


extern u8 FileData[4096];
extern u8 buf[2048];

/**********************************************************
* Tells the flashloader to erase one page at the 
* given address.
**********************************************************/
static u8 sendWriteOptionByteCmd(SWD_HANDLE * pswd , uint32_t addr,u32 numBytes)
{
  u32 wordsToWrite = 0;
  stm32_mcu_info stm32_mcu;
  stm32_mcu.flash_loader_index=(u8)cJSON_GetObjectItem(pswd->root, "exe")->valueint;
  
  switch(stm32_mcu.flash_loader_index)
  {
  case 0x0D:
    {
      wordsToWrite = numBytes/ 4;
      
      
      if(wordsToWrite*4<numBytes)
      {
        wordsToWrite++;
      }
      
      
      for(u16 i=0;i<numBytes;i++)
      {
        FileData[i]=cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "option_byte"),i+1)->valuedouble;
      }        
    }
    break;
    
  default:
    {
      
      wordsToWrite = (numBytes<<1) / 4;
      
      
      if(wordsToWrite*4<(numBytes<<1))
      {
        wordsToWrite++;
      }
      
      for(u16 i=0;i<(numBytes<<1);i+=2)
      {
        FileData[i]=cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "option_byte"),1+(i/2))->valuedouble;
        FileData[i+1]=~FileData[i];
      }
      
      numBytes=numBytes<<1;//20240824 修复STM32F407VET6(STM32F407VGT6)选项字节FLASH_OPTCR被误写写保护
    }
    break;
  }
  
  
  
  
  
  uint32_t bufferLocation1 = readMem(pswd, (uint32_t)&(flState->bufferAddress1) );
  uint32_t bufferLocation2 = readMem(pswd, (uint32_t)&(flState->bufferAddress2) );
  /* Get size of target buffer */
  uint32_t bufferSize = readMem(pswd, (uint32_t)&(flState->bufferSize) );
  writeToFlashloaderBuffer(pswd, bufferLocation1,(u32 *)&FileData[0],wordsToWrite);
  
  
  writeMem(pswd, (uint32_t)&(flState->writeAddress1), addr);
  //writeMem((uint32_t)&(flState->numBytes1), wordsToWrite * 4);
  writeMem(pswd, (uint32_t)&(flState->numBytes1), numBytes);//20240824 修复STM32F407VET6(STM32F407VGT6)选项字节FLASH_OPTCR被误写写保护
  
  
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_OPTION_BYTE_DATA1);
  
  
  if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
  {
    return 1;
  }
  return 0;
}



/**********************************************************
* Writes a chunk of data to a buffer in the flashloader.
* This function does not make any checks or assumptions.
* It simply copies a number of words from the 
* local to the remote buffer.
* 
* @param remoteAddr
*    Address of the flashloader buffer at the target
* 
* @param localBuffer
*    The local buffer to write from
* 
* @param numWords
*    Number of words to write to buffer
**********************************************************/
static void writeToFlashloaderBuffer(SWD_HANDLE * pswd, uint32_t remoteAddr, uint32_t *localBuffer, u32 numWords)
{
  uint32_t bufferPointer = (uint32_t)remoteAddr;
  u32 curWord = 0;
  uint32_t tarWrap;
  
  /* Get the TAR wrap-around period */
  // tarWrap = 0x000003FF;
  tarWrap = 0x0000007F;
  
  /* Set auto increment on TAR to allow faster writes */
  //Write_AP_CSW(AP_CSW_DEFAULT | AP_CSW_AUTO_INCREMENT|0x01000000);
  Write_AP_CSW(pswd, AP_CSW_DEFAULT | AP_CSW_AUTO_INCREMENT);
  /* Initialize TAR with the start of buffer */
  writeAP_TAR(pswd, bufferPointer);
  
  //  /* Send up to one full buffer of data */    
  //  while ( curWord < numWords )
  //  {
  //    /* At the TAR wrap boundary we need to reinitialize TAR
  //    * since the autoincrement wraps at these */
  //    if ( (bufferPointer & tarWrap) == 0 )
  //    {
  //      writeAP_TAR(bufferPointer);
  //    }
  //
  //    
  //    /* Write one word */
  //    writeAP_data(localBuffer[curWord]);
  //    /* Increment local and remote pointers */
  //    bufferPointer += 4;
  //    curWord += 1;
  //  }
  
  /* Send up to one full buffer of data */    
  while (numWords )
  {
    /* At the TAR wrap boundary we need to reinitialize TAR
    * since the autoincrement wraps at these */
    if ( (bufferPointer & tarWrap) == 0 )
    {
      writeAP_TAR(pswd, bufferPointer);
    }
    
    
    /* Write one word */
    writeAP_data_block(pswd, localBuffer[curWord],&numWords);
    // writeAP_data_block_fast(localBuffer[curWord],&numWords);
    /* Increment local and remote pointers */
    bufferPointer += 4;
    curWord += 1;
    numWords-=1;
  }
  
  
  /* Disable auto increment on TAR */
  // Write_AP_CSW(AP_CSW_DEFAULT|0x01000000);  
  Write_AP_CSW(pswd, AP_CSW_DEFAULT);  
}


/**********************************************************
* Uses the flashloader to upload an application to 
* the target. This function will first upload the 
* flashloader, then communicate with the flashloader
* to send the program. If successful the application
* will be started when this function returns. 
* 
* @param verify
*    If true, verify the application after it has been 
*    written to flash
**********************************************************/




#pragma section="flashloader_f0_section"
#pragma section="flashloader_f1_section"
#pragma section="flashloader_f2_section"
#pragma section="flashloader_f3_section"
#pragma section="flashloader_f4_section" 
#pragma section="flashloader_f7_section"
#pragma section="flashloader_h7_section" 
#pragma section="flashloader_L0_section" 
#pragma section="flashloader_L1_section" 
#pragma section="flashloader_L4_section"
#pragma section="flashloader_g0_section" 
#pragma section="flashloader_mm32_f0_section"
#pragma section="flashloader_mm32_f1_section"
#pragma section="flashloader_efm32_section"
#pragma section="flashloader_e501_section"
#pragma section="flashloader_mm32f0010_section"
#pragma section="flashloader_gd32f1_section"
#pragma section="flashloader_G4_section" 
#include"ff.h"
static FIL FilSys;
static u8 FileData[4096]; 
static u32 ReadCount;
static char sp1[100];
extern u16 ImageNumber_store;
/**********************************************************
* Uploads a binary image (the firmware) to the flashloader.
**********************************************************/
u8 uploadImageToFlashloader(SWD_HANDLE * pswd)
{   
  // uint32_t numWords = size / 4;
  stm32_mcu_info stm32_mcu;
  stm32_mcu.flash_loader_index=(u8)cJSON_GetObjectItem(pswd->root, "exe")->valueint;
  uint32_t writeAddress = 0;
  u32 WriteBufferSize = 256;
  WriteBufferSize = readMem(pswd, (uint32_t)&(flState->bufferSize) );
  writeAddress = (u32)cJSON_GetObjectItem(pswd->root, "flash_base")->valuedouble;
  if(writeAddress==8000000)//兼容老版本电脑软件
  {
    writeAddress=0x8000000;
  }
  u32 aes_key=3000;
  
  bool useBuffer1 = true;
  
  
  
  sprintf(sp1,"0:/system/dat/DAT%03u.BIN",pswd->ImageNumber_store);
  if(!f_open(&FilSys, sp1,  FA_READ))//根据(buf[2]<<8)|buf[1]中的镜像号来进行读文件操作
  {
    if(f_read(&FilSys,FileData,WriteBufferSize,&ReadCount)!=FR_OK)
    {
      return 1;
    }
  }
  else
  {
    tf_card_warning();
  }
  
  
  
  u8 * p_Buffer = FileData;
  
  
  
  
  if( (u8)cJSON_GetObjectItem(pswd->root, "data_encrypt")->valueint==1)//AES加密
  {
    p_Buffer = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "aes_key"),5)->valuedouble;
    AES_Decrypt_Config_Init(16,aes_key);
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(p_Buffer);
      //AES_Test_Decrypt(p_Buffer, 16, 2000);
      p_Buffer += 16;
    }
  }
  
  
  /* Get the buffer location (where to temporary store data 
  * in target SRAM) from flashloader */
  uint32_t bufferLocation1 = readMem(pswd, (uint32_t)&(flState->bufferAddress1) );
  uint32_t bufferLocation2 = readMem(pswd, (uint32_t)&(flState->bufferAddress2) );
  
  /* Get size of target buffer */
  //下面这行疑似占用1.5秒
  //uint32_t bufferSize = readMem( (uint32_t)&(flState->bufferSize) );
  
  /* Round up to nearest word */
  // if ( numWords * 4 < size ) numWords++;
  
  
  /* Fill the buffer in RAM and tell the flashloader to write
  * this buffer to Flash. Since we are using two buffers
  * we can fill one buffer while the flashloader is using
  * the other. 
  */  
  while (ReadCount)//
  {
    /* Calculate the number of words to write */
    u32 wordsToWrite = ReadCount / 4;
    if(wordsToWrite*4<ReadCount)
    {
      wordsToWrite++;
      
      for(u16 i=ReadCount;i<wordsToWrite*4;i++)
      {
        if(stm32_mcu.flash_loader_index==0x10)
        {
          FileData[i]=0x00;
        }
        else
        {
          FileData[i]=0xFF;
        }
        
        
      }
    }
    
    
    /* Write one chunk to the currently active buffer */
    // writeToFlashloaderBuffer(useBuffer1 ? bufferLocation1 : bufferLocation2,&fwImage[curWord],wordsToWrite);
    /* Write one chunk to the currently active buffer */
    writeToFlashloaderBuffer(pswd, useBuffer1 ? bufferLocation1 : bufferLocation2,(u32 *)&FileData[0],wordsToWrite);
    
    
    
    /* Wait until flashloader is done writing to flash */
    if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
    {
      return 2;
    }
    
    
    /* Tell the flashloader to write the data to flash */
    if ( useBuffer1 ) 
    {      
      writeMem(pswd, (uint32_t)&(flState->numBytes1), wordsToWrite * 4);
      writeMem(pswd, (uint32_t)&(flState->writeAddress1), writeAddress);
      writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA1 );
    } 
    else
    {
      writeMem(pswd, (uint32_t)&(flState->numBytes2),  wordsToWrite * 4);
      writeMem(pswd, (uint32_t)&(flState->writeAddress2), writeAddress);
      writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA2 );      
    }
    
    /* Increase address */
    // curWord += wordsToWrite;
    writeAddress += wordsToWrite * 4;
    
    /* Flip buffers */
    useBuffer1 = !useBuffer1;
    
    
    
    if(ReadCount>=WriteBufferSize)//判断上一次读的数据是否够256
    {
      if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
      {
        return 9;
      }
      if(f_read(&FilSys,FileData,WriteBufferSize,&ReadCount)!=FR_OK)
      {
        return 10;
      }
      
      
      if( (u8)cJSON_GetObjectItem(pswd->root, "data_encrypt")->valueint==1)//AES加密
      {
        p_Buffer = FileData;
        
        
        for (u32 j = 0; j < ReadCount; j += 16)
        {
          //解密数据包
          AES_Decrypt_Calculate(p_Buffer);
          p_Buffer += 16;
        }
      }
      
      
      
    }
    else
    {
      ReadCount=0;//强制退出循环，
    }
    
    
    
  }
  f_close(&FilSys);
  /* Wait until the last flash write operation has completed */
  if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
  {
    return 3;
  }
  
  
  return 0;
}



u8 erasePagesWithFlashloader(SWD_HANDLE * pswd)
{
  u32 addr;
  
  sprintf(sp1,"0:/system/dat/SEC%03u.BIN",pswd->ImageNumber_store);
  if(!f_open(&FilSys, sp1,  FA_READ))//
  {
    if(f_read(&FilSys,FileData,8,&ReadCount)!=FR_OK)
    {
      return 1;
    }
    
    
    
    while (ReadCount)//
    {
      if(sendErasePageCmd(pswd, (FileData[0]|FileData[1]<<8|FileData[2]<<16|FileData[3]<<24),(FileData[4]|FileData[5]<<8|FileData[6]<<16|FileData[7]<<24))!=0)
      {
        return 3;
      }
      
      if(ReadCount>=8)//判断上一次读的数据是否够填充缓冲区的大小，如果足够则读一次。文件末尾检测
      {
        if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
        {
          return 9;
        }
        if(f_read(&FilSys,FileData,8,&ReadCount)!=FR_OK)//如果没有数据的话这里ReadCount会变为0，会自动退出while循环
        {
          return 10;
        }
      }
      else
      {
        ReadCount=0;//强制退出循环，
      }
    }
    
    
    f_close(&FilSys);
  }
  else
  {
    return 3;
  }
  
  
  return 0;
}

FIL FilSys_page;
u32 WriteBufferSize_page;
uint32_t bufferLocation1_page;
uint32_t bufferLocation2_page;
uint32_t file_start_address = 0;
static u8 sendwritePageCmd(SWD_HANDLE *pswd, u32 block_start_address,u32 block_size)
{
  bool useBuffer1 = true;
  u32 ReadCount_page=0;
  uint32_t writeAddress = block_start_address;
  
  uint32_t file_offset = block_start_address-file_start_address;
  uint32_t block_end_address = block_start_address+block_size;//页块结束地址
  
  
  sprintf(sp1,"0:/system/dat/DAT%03d.BIN",pswd->ImageNumber_store);
  if(!f_open(&FilSys_page, sp1,  FA_READ))//根据(buf[2]<<8)|buf[1]中的镜像号来进行读文件操作
  {
    if(f_lseek(&FilSys_page,file_offset)!=FR_OK)
    {
      return 9;
    }
    
    if(f_read(&FilSys_page,FileData,WriteBufferSize_page,&ReadCount_page)!=FR_OK)
    {
      return 1;
    }
    
    
    while (ReadCount_page)//
    {
      /* Calculate the number of words to write */
      u32 wordsToWrite = ReadCount_page / 4;
      if(wordsToWrite*4<ReadCount_page)
      {
        wordsToWrite++;
        
        for(u16 i=ReadCount_page;i<wordsToWrite*4;i++)
        {
          FileData[i]=0xFF;
        }
      }
      
      
      /* Write one chunk to the currently active buffer */
      // writeToFlashloaderBuffer(useBuffer1 ? bufferLocation1 : bufferLocation2,&fwImage[curWord],wordsToWrite);
      /* Write one chunk to the currently active buffer */
      writeToFlashloaderBuffer(pswd, useBuffer1 ? bufferLocation1_page : bufferLocation2_page,(u32 *)&FileData[0],wordsToWrite);
      
      
      
      /* Wait until flashloader is done writing to flash */
      if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
      {
        return 2;
      }
      
      
      /* Tell the flashloader to write the data to flash */
      if ( useBuffer1 ) 
      {      
        writeMem(pswd, (uint32_t)&(flState->numBytes1), wordsToWrite * 4);
        writeMem(pswd, (uint32_t)&(flState->writeAddress1), writeAddress);
        writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA1 );
      } 
      else
      {
        writeMem(pswd, (uint32_t)&(flState->numBytes2),  wordsToWrite * 4);
        writeMem(pswd, (uint32_t)&(flState->writeAddress2), writeAddress);
        writeMem(pswd, (uint32_t)&(flState->debuggerStatus), DEBUGGERCMD_WRITE_DATA2 );      
      }
      
      /* Increase address */
      // curWord += wordsToWrite;
      writeAddress += wordsToWrite * 4;
      
      /* Flip buffers */
      useBuffer1 = !useBuffer1;
      
      
      if((ReadCount_page>=WriteBufferSize_page)&&(writeAddress<block_end_address))//判断上一次读的数据是否够填充缓冲区的大小，如果足够则读一次。文件末尾检测
      {
        if(f_lseek(&FilSys_page,FilSys_page.fptr)!=FR_OK)
        {
          return 9;
        }
        if(f_read(&FilSys_page,FileData,WriteBufferSize_page,&ReadCount_page)!=FR_OK)
        {
          return 10;
        }
      }
      else
      {
        ReadCount_page=0;//强制退出循环，
      }
    }
    
    
    f_close(&FilSys_page);
    /* Wait until the last flash write operation has completed */
    if(waitForFlashloader(pswd, FLASHLOADER_RETRY_COUNT)!=0)
    {
      return 3;
    }
  }
  else
  {
    tf_card_warning();
    return 3;
  }
  return 0;
}



static u8 writePagesWithFlashloader(SWD_HANDLE * pswd)
{   
  
  uint32_t writeAddress = 0;
  uint32_t block_start_address = 0;
  uint32_t block_size = 0;
  uint32_t block_end_address = 0;
  uint32_t file_offset = 0;
  u8 sector_buffer[8];
  
  WriteBufferSize_page = readMem(pswd, (uint32_t)&(flState->bufferSize) );
  bufferLocation1_page = readMem(pswd, (uint32_t)&(flState->bufferAddress1) );
  bufferLocation2_page = readMem(pswd, (uint32_t)&(flState->bufferAddress2) );
  
  
  //整个文件的起始地址
  file_start_address = (u32)cJSON_GetObjectItem(pswd->root, "flash_base")->valuedouble;
  
  
  sprintf(sp1,"0:/system/dat/SEC%03u.BIN",pswd->ImageNumber_store);
  if(!f_open(&FilSys, sp1,  FA_READ))//
  {
    if(f_read(&FilSys,sector_buffer,8,&ReadCount)!=FR_OK)
    {
      return 1;
    }
    
    while (ReadCount)//
    {
      if(sendwritePageCmd(pswd, sector_buffer[0]|sector_buffer[1]<<8|sector_buffer[2]<<16|sector_buffer[3]<<24,sector_buffer[4]|sector_buffer[5]<<8|sector_buffer[6]<<16|sector_buffer[7]<<24)!=0)
      {
        return 3;
      }
      
      if(ReadCount>=8)//判断上一次读的数据是否够填充缓冲区的大小，如果足够则读一次。文件末尾检测
      {
        if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
        {
          return 9;
        }
        if(f_read(&FilSys,sector_buffer,8,&ReadCount)!=FR_OK)//如果没有数据的话这里ReadCount会变为0，会自动退出while循环
        {
          return 10;
        }
      }
      else
      {
        ReadCount=0;//强制退出循环，
      }
    }
    
    
    f_close(&FilSys);
  }
  else
  {
    return 3;
  }
  
  return 0;
}







extern u16 displayposition;


void HardRestPinRest(SWD_HANDLE * pswd)
{
  PIN_nRESET_LOW(pswd);
  
  //  System_Delay_ms(10);
  delay_ms(10);
  
  PIN_nRESET_HIGH(pswd);
}



u8 flashWithFlashloader(SWD_HANDLE *pswd)
{
  u32 optionbyte_address=0;
  u8 res_value=0;
  u32 verifyFlashloaderReadyResult=0;
  stm32_mcu_info stm32_mcu;
  
  
  //UCOS-III必须定义为全局变量？否则反复下载容易导致HardFault_Handler
  u32 *p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f1_section");
  u32 flashloaderSize = __section_size("flashloader_f1_section");  
  
  stm32_mcu.flash_loader_index=(u8)cJSON_GetObjectItem(pswd->root, "exe")->valueint;//;
  
  switch(stm32_mcu.flash_loader_index)
  {
  case 0x00:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f0_section");
    flashloaderSize = __section_size("flashloader_f0_section");
    flState = flState_F0_1;
    optionbyte_address=0x1FFFF800;
    break;
    
  case 0x01:
    if(strcmp(cJSON_GetObjectItem(pswd->root, "chip_type")->valuestring, "GD32") == 0)
    {
      p_flashloader_section_begin=(u32 *)__section_begin("flashloader_gd32f1_section");
      flashloaderSize = __section_size("flashloader_gd32f1_section");
    }
    else
    {
      p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f1_section");
      flashloaderSize = __section_size("flashloader_f1_section");
    }
    
    
    
    flState = flState_F0_1;
    optionbyte_address=0x1FFFF800;
    break;
    
  case 0x02:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f2_section");
    flashloaderSize = __section_size("flashloader_f2_section");
    flState = flState_F2_3_4;
    optionbyte_address=0x40023C14;
    
    break;
    
  case 0x03:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f3_section");
    flashloaderSize = __section_size("flashloader_f3_section");
    flState = flState_F2_3_4;
    optionbyte_address=0x1FFFF800;
    break;
  case 0x04:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f4_section");
    flashloaderSize = __section_size("flashloader_f4_section");
    
    flState = flState_F2_3_4;
    optionbyte_address=0x40023C14;
    break;
    
    
  case 0x07:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f7_section");
    flashloaderSize = __section_size("flashloader_f7_section");
    
    
    flState = flState_F7;
    optionbyte_address=0x40023C14;
    break;
    
    
  case 0x08:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_h7_section");
    flashloaderSize = __section_size("flashloader_h7_section");
    
    
    flState = flState_F7;
    optionbyte_address=0x40023C14;
    break;
    
  case 0x0A:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_f0_section");
    flashloaderSize = __section_size("flashloader_f0_section");
    flState = flState_F0_1;
    optionbyte_address=0x1FFFF800;
    break;
    
  case 0x0B:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_g0_section");
    flashloaderSize = __section_size("flashloader_g0_section");
    
    flState = flState_F0_1;
    optionbyte_address=0x40022020;
    break;
    
  case 0x0C:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_e501_section");
    flashloaderSize = __section_size("flashloader_e501_section");
    flState = flState_F0_1;
    optionbyte_address=0x1FFFF800;
    break;
    
  case 0x0D:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_G4_section");
    flashloaderSize = __section_size("flashloader_G4_section");
    
    flState = flState_F2_3_4;
    optionbyte_address=0x40022020;
    break;    
    
    
    
  case 0x10:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_L0_section");
    flashloaderSize = __section_size("flashloader_L0_section");
    
    flState = flState_L011;
    optionbyte_address=0x1FF80000;
    break;
    
    
  case 0x11:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_L1_section");
    flashloaderSize = __section_size("flashloader_L1_section");
    
    flState = flState_F0_1;
    optionbyte_address=0x1FF80000;
    break;    
    
    
  case 0x14:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_L4_section");
    flashloaderSize = __section_size("flashloader_L4_section");
    
    flState = flState_F2_3_4;
    optionbyte_address=0x40022020;
    break;    
    
    
  case 0x15:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_mm32_f0_section");
    flashloaderSize = __section_size("flashloader_mm32_f0_section");
    flState = flState_F0_1;
    optionbyte_address=0x1FFFF800;
    break;
    
  case 0x16:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_mm32_f1_section");
    flashloaderSize = __section_size("flashloader_mm32_f1_section");
    flState = flState_F0_1;
    optionbyte_address=0x1FFFF800;
    break;
    
  case 0x17:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_efm32_section");
    flashloaderSize = __section_size("flashloader_efm32_section");
    flState = flState_efm32;
    optionbyte_address=0x1FFFF800;
    break;   
    
    
  case 0x18:
    p_flashloader_section_begin=(u32 *)__section_begin("flashloader_mm32f0010_section");
    flashloaderSize = __section_size("flashloader_mm32f0010_section");
    flState = flState_L011;
    optionbyte_address=0x1FFFF800;
    break;
  }
  
  
  
  
  
  writeMem(pswd, (uint32_t)&(flState->flashLoaderStatus),0);
  writeMem(pswd, (uint32_t)&(flState->debuggerStatus),0);
  
  
  
  if(uploadFlashloader(pswd, p_flashloader_section_begin, flashloaderSize)!=0)
  {
    return 101;
  }
  /* Check that flashloader is ready */
  verifyFlashloaderReadyResult=verifyFlashloaderReady(pswd);
  
  if(verifyFlashloaderReadyResult!=0)
  {
    return 1;
  }
  
  
  
  mh=3;
  draw_progress_bar(mh);
  GUI_Exec();
  
  if((u8)cJSON_GetObjectItem(pswd->root, "mass_erase")->valueint==1)
  {
    if(sendMassEraseCmd(pswd)!=0)
    {
      return 2;
    }
    display_clear_chip_ok();
  }
  
  mh=7;
  draw_progress_bar(mh);
  GUI_Exec();
  
  if((u8)cJSON_GetObjectItem(pswd->root, "page_erase")->valueint==1)
  {
    if(erasePagesWithFlashloader(pswd)!=0)
    {
      return 2;
    }
    display_erase_sector_ok();
  }
  
  mh=7;
  draw_progress_bar(mh);
  GUI_Exec();
  //  
  /////////////////////////////////////
  // if(stm32_mcu.flash_loader_index>0x14)
  extern u8 re_connect;
  if(re_connect==1||(stm32_mcu.flash_loader_index>0x14))
  {
    if(EnterProgramMode(pswd)==0)
    {
      writeMem(pswd, (uint32_t)&(flState->flashLoaderStatus),0);
      writeMem(pswd, (uint32_t)&(flState->debuggerStatus),0);
      
      
      
      if(uploadFlashloader(pswd, p_flashloader_section_begin, flashloaderSize)!=0)
      {
        return 101;
      }
      /* Check that flashloader is ready */
      verifyFlashloaderReadyResult=verifyFlashloaderReady(pswd);
      
      if(verifyFlashloaderReadyResult!=0)
      {
        return 1;
      }
    }
    else
    {
      return 2;    
    }    
  }
  
  
  /////////////////////////////////////  
  
  
  TargetValEmpty = (u8)cJSON_GetObjectItem(pswd->root, "blank_value")->valueint;//读取擦除过后的默认的空的值--0xFF,--0x00
  
  
  
  if((u8)cJSON_GetObjectItem(pswd->root, "write_flash")->valueint==1)
  {
    if((u8)cJSON_GetObjectItem(pswd->root, "page_erase")->valueint)
    {
      if(writePagesWithFlashloader(pswd)!=0)
      {
        return 3;
      }
      display_write_page_flash_ok();
    }
    else
    {
      if(uploadImageToFlashloader(pswd)!=0)
      {
        return 3;
      }
      display_write_flash_ok();
    }
    
  }
  
  
  
  
  
  
  
  if((u8)cJSON_GetObjectItem(pswd->root, "verify_flash")->valueint==1)
  {
    extern u32 verify_failed_address;
    verify_failed_address=verifyFirmware(pswd);
    if(verify_failed_address!=0)
    {
      display_verify_flash_different();
      return 7;
    }
    display_verify_flash_ok();
  }
  
  
  if((u8)cJSON_GetObjectItem(pswd->root, "write_rolling_code")->valueint==1)
  {
    if(sendWriteRollingCodeCmd(pswd)!=0)
    {
      return 4;
    }
    // display_WriteRollingCode_ok();
  }
  
  
  if((u8)cJSON_GetObjectItem(pswd->root, "custom_secret")->valueint==1)
  {
    if(STM32_WriteUniqueIDJiaMi(pswd)!=0)
    {
      return 5;
    }
    // display_write_UniqueIDJiaMi_ok();
  }
  
  
  
  
    if((u8)cJSON_GetObjectItem(pswd->root, "enable_code_bar")->valueint==1)//扫码枪
  {
    if(pswd->code_bar_plug==1)
    {
      if(sendWriteCodeBarCmd(pswd)!=0)
      {
        return 4;
      }
      display_WriteCodeBar_ok();
    }
    else
    {
      display_WriteCodeBar_ignore();
    }

  }
  
  mh=18;
  draw_progress_bar(mh);
  
  if((u8)cJSON_GetObjectItem(pswd->root, "write_option_byte")->valueint==1)
  {
    res_value=sendWriteOptionByteCmd(pswd,optionbyte_address,cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "option_byte"),0)->valuedouble);
    if(res_value!=0)
    {
      return 5;
    }
    display_write_option_byte_ok();
  }
  
  
  
  /* Calculate the total time spent flashing */
  //time = (DWT->CYCCNT - startTime)/7200000;
  /* Reset target to start application */
  // if(resetAndHaltTarget()!=0)
  {
    //return 6;
  }
  
  
  
  
  switch(cJSON_GetObjectItem(pswd->root, "reset_type")->valueint==1)
  {
  case 0:
    resetTarget(pswd);
    break;
    
  case 1:
    resetTarget(pswd);
    break;
    
  case 2:
    break;
    
  }
  
  
  
  
  mh=23;
  draw_progress_bar(mh);
  GUI_Exec();
  //  extern u32 startTime;
  //  extern u32 times_elapsed_100ms;
  // times_elapsed_100ms = (DWT->CYCCNT - startTime)/7200000;
  
  //times_elapsed();
  return 0;
}




u8 C[4];


static void Algorithm_00(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_01(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0]; 
  OutPut[1] = C[1] | D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] | D[3]; 
}

static void Algorithm_02(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] + D[1]; 
  OutPut[2] = C[2] + D[2]; 
  OutPut[3] = C[3] + D[3]; 
}

static void Algorithm_03(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] ^ D[1]; 
  OutPut[2] = C[2] ^ D[2]; 
  OutPut[3] = C[3] ^ D[3];
}

static void Algorithm_04(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] & D[2]; 
  OutPut[3] = C[3] & D[3]; 
}

static void Algorithm_05(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] - D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] - D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_06(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] * D[2]; 
  OutPut[3] = C[3] / D[3]; 
}

static void Algorithm_07(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_08(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_09(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] + D[5]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_10(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] ^ D[5]; 
  OutPut[2] = C[2] | D[2] ^ D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_11(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] + D[9]; 
  OutPut[2] = C[2] | D[2] ^ D[6] + D[10]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_12(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[2] | D[2] ^ D[6] ^ D[10]; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_13(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[4] + D[8]; 
  OutPut[1] = D[1] ^ D[5] | D[9]; 
  OutPut[2] = D[2] ^ D[6] ^ D[10]; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_14(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3]; 
  OutPut[1] = D[4] ^ D[5] | D[6]; 
  OutPut[2] = D[7] ^ D[8] ^ D[9]; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_15(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_16(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_17(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_18(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3];
}

static void Algorithm_19(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_20(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_21(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[0] | D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_22(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] | D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_23(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_24(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_25(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] & D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_26(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] ; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[3] ; 
}

static void Algorithm_27(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_28(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_29(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_30(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}



static void Algorithm_31(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}


static void Algorithm_32(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
  OutPut[1] = C[1] & D[0] & D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2] ^ D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
  OutPut[3] = C[3] & D[0] & D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_33(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] &  D[1] & D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] - D[1] & D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] & D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] & D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_34(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] & D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] & D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] & D[2] & D[3] ^ D[4] ^ D[5] ^ D[6] & D[7] & D[8] ^ D[9] ^ D[10] & D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] & D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_35(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] & D[4] & D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] & D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_36(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_37(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_38(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] & D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_39(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] | D[4]+ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_40(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]+ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_41(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4]+ D[5] | D[6] ^ D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_42(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_43(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[4] + D[8] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[1] = D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_44(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[1] = D[4] ^ D[5] | D[6]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_45(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_46(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] | D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_47(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[1] = C[1] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_48(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3];
}

static void Algorithm_49(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_50(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11];
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_51(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[0] | D[0] - D[1] & D[2] + D[3] - D[4] + D[5] + D[6] | D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_52(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] & D[1] ^ D[2] & D[3] & D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] | D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11]; 
}

static void Algorithm_53(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] & D[1] & D[2] & D[3] & D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_54(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] & D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_55(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] & D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11];
  OutPut[3] = C[3]; 
}

static void Algorithm_56(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] & D[2] & D[3] ^ D[4] & D[5] | D[6] & D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] ; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[3] ; 
}

static void Algorithm_57(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] & D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] & D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_58(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_59(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] & D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_60(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] & D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}


static void Algorithm_61(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] & D[5] ^ D[6] ^ D[7]; 
  OutPut[1] = C[1] | D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] | D[3]; 
}

static void Algorithm_62(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] + D[1]; 
  OutPut[2] = C[2] + D[2]; 
  OutPut[3] = C[3] + D[3]; 
}

static void Algorithm_63(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] ^ D[1]; 
  OutPut[2] = C[2] ^ D[2]; 
  OutPut[3] = C[3] ^ D[3];
}

static void Algorithm_64(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] & D[2]; 
  OutPut[3] = C[3] & D[3]; 
}

static void Algorithm_65(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] - D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] - D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_66(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = C[2] * D[2] ^ D[3] ^ D[4] + D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] ^ D[11]; 
  OutPut[3] = C[3] / D[3]; 
}

static void Algorithm_67(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] + D[3] & D[4] + D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] % D[3];
}

static void Algorithm_68(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[3] - D[4] + D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2]; 
  OutPut[3] = C[3] - D[3]; 
}

static void Algorithm_69(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] ^ D[4]; 
  OutPut[1] = C[1] ^ D[1] + D[5]; 
  OutPut[2] = C[2] | D[2] + D[6]; 
  OutPut[3] = C[3] - D[3] + D[7] ^ D[8] + D[9] + D[10] ^ D[11]; 
}

static void Algorithm_70(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] ^ D[5] + D[6] + D[7] ^ D[8] + D[9] - D[10] - D[11]; 
  OutPut[2] = C[2] | D[2] ^ D[6]; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_71(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] + D[9]; 
  OutPut[2] = C[2] | D[2] ^ D[6] + D[10]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_72(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[1] = C[1] & D[1] ^ D[5] | D[9]; 
  OutPut[2] = C[2] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11];
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_73(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11];
  OutPut[1] = D[1] ^ D[5] | D[9]; 
  OutPut[2] = D[2] ^ D[6] ^ D[10]; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_74(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3]; 
  OutPut[1] = D[4] ^ D[5] | D[6] + D[7] - D[8] + D[9]; 
  OutPut[2] = D[7] ^ D[8] ^ D[9]; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_75(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_76(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_77(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1] & D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_78(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3];
}

static void Algorithm_79(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_80(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_81(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_82(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] + D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6]; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11]; 
}

static void Algorithm_83(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] & D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_84(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] ; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
}

static void Algorithm_85(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] & D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_86(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[3] ; 
}

static void Algorithm_87(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1] & D[0] | D[1] ^ D[2] & D[3] | D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_88(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_89(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] - D[3] + D[4] + D[5] ^ D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1] + D[2] - D[3] + D[4] + D[5] + D[6] + D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_90(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] - D[1] ^ D[2] ^ D[3] ^ D[4] - D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] - D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
}


static void Algorithm_91(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_92(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1] - D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_93(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_94(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_95(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] + D[8] + D[9] + D[10] + D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9]; 
  OutPut[3] = C[3] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
}

static void Algorithm_96(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[3] ^ D[4] ^ D[5] | D[6] ; 
  OutPut[3] = C[3] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
}


static void Algorithm_97(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] ; 
  OutPut[1] = C[1] + D[3] ^ D[4] ^ D[5] | D[6] | D[7]; 
  OutPut[2] = C[2] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_98(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[1] + D[2] + D[3] + D[4] + D[5] + D[6] + D[7] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2] + D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] + D[3] ^ D[4] ^ D[5] | D[6] ; 
}

static void Algorithm_99(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = D[0] + D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_100(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[1] = C[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_101(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] ^ D[2] & D[3] ^ D[4]; 
  OutPut[1] = C[1] | D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] | D[3]; 
}

static void Algorithm_102(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] + D[2]; 
  OutPut[3] = C[3] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11];  
}

static void Algorithm_103(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11]; 
  OutPut[2] = C[2] ^ D[2]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_104(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] ^ D[3];
}

static void Algorithm_105(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] - D[0]; 
  OutPut[1] = C[1] - D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_106(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2] & D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] / D[3]; 
}

static void Algorithm_107(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = C[2] | D[2] & D[4] ^ D[5] | D[6] ^ D[7] & D[8] ^ D[9] ^ D[10] ^ D[11]; 
  OutPut[3] = C[3] % D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ;
}

static void Algorithm_108(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0]; 
  OutPut[1] = C[1] & D[1]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_109(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = C[1] & D[1] + D[5]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_110(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] + D[4]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7]; 
}

static void Algorithm_111(u8 *D,u8 *OutPut) 
{  
  OutPut[0] = C[0] + D[0] + D[4] + D[8]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
}

static void Algorithm_112(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] + D[0] - D[4] + D[8]; 
  OutPut[3] = C[3] - D[3] + D[7] + D[11]; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] - D[3] + D[7] & D[11]; 
}

static void Algorithm_113(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[4] + D[8]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[3] + D[7] & D[11]; 
}

static void Algorithm_114(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3]; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[10] + D[11] ; 
}

static void Algorithm_115(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[2] + D[3] - D[0] + D[2] + D[3]; 
  OutPut[1] = C[1] + D[3] ^ D[4] + D[5] + D[6]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_116(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6]; 
  OutPut[1] = C[1] + D[3] ^ D[4] + D[5] + D[6]; 
  OutPut[2] = C[2] + D[3] - D[4] ^ D[5] + D[6]; 
  OutPut[3] = C[3] ^ D[3] - D[4] + D[5] + D[6]; 
}

static void Algorithm_117(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] & D[5] + D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2] - D[4] + D[5]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_118(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] ^ D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] & D[5] + D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3] - D[7] + D[8];
}

static void Algorithm_119(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_120(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = D[0] - D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] - D[5] | D[6] - D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_121(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] ^ D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] - D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[0] | D[0] - D[1] - D[2] - D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[0] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[0] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] - D[9] ^ D[10] ^ D[11] ; 
}

static void Algorithm_122(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] | D[0] | D[1] ^ D[2] - D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] | D[0] + D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] | D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[3] = C[3] | D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_123(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] ^ D[0] | D[1] ^ D[2] - D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] ^ D[0] - D[1] - D[2] + D[3] - D[4] & D[5] - D[6] + D[7] - D[8] + D[9] - D[10] & D[11] ; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3] ^ D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_124(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] & D[1] - D[2] - D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] & D[6] ^ D[7] & D[8] ^ D[9] - D[10] ^ D[11] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_125(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] & D[5] | D[6] ^ D[7] & D[8] & D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1]; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}

static void Algorithm_126(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] | D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] ; 
  OutPut[2] = C[2] & D[2] ; 
  OutPut[3] = C[3] & D[0] | D[1] ^ D[2] ^ D[3] ^ D[4] ^ D[5] | D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] & D[11] ; 
}

static void Algorithm_127(u8 *D,u8 *OutPut) 
{ 
  OutPut[0] = C[0] & D[0] | D[1] ^ D[2] & D[3] ^ D[4] ^ D[5] & D[6] | D[7] ^ D[8] | D[9] ^ D[10] ^ D[11] ; 
  OutPut[1] = C[1] & D[0] & D[1] - D[2] + D[3] - D[4] + D[5] + D[6] + D[7] - D[8] + D[9] - D[10] + D[11] ; 
  OutPut[2] = C[2]; 
  OutPut[3] = C[3]; 
}




static u32 JiaMiSuanFa(u8 gongshi,u32 changshu, u8 D[])
{
  
  u8 Result[4];
  
  
  
  
  C[0]=changshu;
  C[1]=changshu>>8;
  C[2]=changshu>>16;  
  C[3]=changshu>>24;
  
  
  switch(gongshi)
  {
  case 0:
    Algorithm_00(D,Result);
    
    break;
    
  case 1:
    Algorithm_01(D,Result);
    break;
    
  case 2:
    Algorithm_02(D,Result);
    break;
    
  case 3:
    Algorithm_03(D,Result);   
    break; 
    
  case 4:
    Algorithm_04(D,Result);  
    break;
    
  case 5:
    Algorithm_05(D,Result);
    break;
    
  case 6:
    Algorithm_06(D,Result);
    break;
    
  case 7:
    Algorithm_07(D,Result);
    break;
    
  case 8:
    Algorithm_08(D,Result);
    break;
    
  case 9:
    Algorithm_09(D,Result);
    break;
    
  case 10:
    Algorithm_10(D,Result);
    break; 
    
  case 11:
    Algorithm_11(D,Result);
    break;
    
  case 12:
    Algorithm_12(D,Result);
    break;
    
  case 13:
    Algorithm_13(D,Result);
    break;
    
  case 14:
    Algorithm_14(D,Result);
    break;
    
  case 15:
    Algorithm_15(D,Result);
    break;
    
  case 16:
    Algorithm_16(D,Result);
    break;
    
  case 17:
    Algorithm_17(D,Result);
    break; 
    
  case 18:
    Algorithm_18(D,Result);
    break;
    
  case 19:
    Algorithm_19(D,Result);
    break;
    
  case 20:
    Algorithm_20(D,Result);
    break;
    
  case 21:
    Algorithm_21(D,Result);
    break;
    
  case 22:
    Algorithm_22(D,Result);
    break;
    
  case 23:
    Algorithm_23(D,Result);   
    break; 
    
  case 24:
    Algorithm_24(D,Result);  
    break;
    
  case 25:
    Algorithm_25(D,Result);
    break;
    
  case 26:
    Algorithm_26(D,Result);
    break;
    
  case 27:
    Algorithm_27(D,Result);
    break;
    
  case 28:
    Algorithm_28(D,Result);
    break;
    
  case 29:
    Algorithm_29(D,Result);
    break;
    
  case 30:
    Algorithm_30(D,Result);
    break; 
    
  case 31:
    Algorithm_31(D,Result);
    break;
    
  case 32:
    Algorithm_32(D,Result);
    break;
    
  case 33:
    Algorithm_33(D,Result);
    break;
    
  case 34:
    Algorithm_34(D,Result);
    break;
    
  case 35:
    Algorithm_35(D,Result);
    break;
    
  case 36:
    Algorithm_36(D,Result);
    break;
    
  case 37:
    Algorithm_37(D,Result);
    break; 
    
  case 38:
    Algorithm_38(D,Result);
    break;
    
  case 39:
    Algorithm_39(D,Result);
    break;
    
  case 40:
    Algorithm_40(D,Result);
    break;
    
  case 41:
    Algorithm_41(D,Result);
    break;
    
  case 42:
    Algorithm_42(D,Result);
    break;
    
  case 43:
    Algorithm_43(D,Result);   
    break; 
    
  case 44:
    Algorithm_44(D,Result);  
    break;
    
  case 45:
    Algorithm_45(D,Result);
    break;
    
  case 46:
    Algorithm_46(D,Result);
    break;
    
  case 47:
    Algorithm_47(D,Result);
    break;
    
  case 48:
    Algorithm_48(D,Result);
    break;
    
  case 49:
    Algorithm_49(D,Result);
    break;
    
  case 50:
    Algorithm_50(D,Result);
    break; 
    
  case 51:
    Algorithm_51(D,Result);
    break;
    
  case 52:
    Algorithm_52(D,Result);
    break;
    
  case 53:
    Algorithm_53(D,Result);
    break;
    
  case 54:
    Algorithm_54(D,Result);
    break;
    
  case 55:
    Algorithm_55(D,Result);
    break;
    
  case 56:
    Algorithm_56(D,Result);
    break;
    
  case 57:
    Algorithm_57(D,Result);
    break; 
    
  case 58:
    Algorithm_58(D,Result);
    break;
    
  case 59:
    Algorithm_59(D,Result);
    break;
    
  case 60:
    Algorithm_60(D,Result);
    break;
    
  case 61:
    Algorithm_61(D,Result);
    break;
    
  case 62:
    Algorithm_62(D,Result);
    break;
    
  case 63:
    Algorithm_63(D,Result);   
    break; 
    
  case 64:
    Algorithm_64(D,Result);  
    break;
    
  case 65:
    Algorithm_65(D,Result);
    break;
    
  case 66:
    Algorithm_66(D,Result);
    break;
    
  case 67:
    Algorithm_67(D,Result);
    break;
    
  case 68:
    Algorithm_68(D,Result);
    break;
    
  case 69:
    Algorithm_69(D,Result);
    break;
    
  case 70:
    Algorithm_70(D,Result);
    break; 
    
  case 71:
    Algorithm_71(D,Result);
    break;
    
  case 72:
    Algorithm_72(D,Result);
    break;
    
  case 73:
    Algorithm_73(D,Result);
    break;
    
  case 74:
    Algorithm_74(D,Result);
    break;
    
  case 75:
    Algorithm_75(D,Result);
    break;
    
  case 76:
    Algorithm_76(D,Result);
    break;
    
  case 77:
    Algorithm_77(D,Result);
    break; 
    
  case 78:
    Algorithm_78(D,Result);
    break;
    
  case 79:
    Algorithm_79(D,Result);
    break;
    
  case 80:
    Algorithm_80(D,Result);
    break;
    
  case 81:
    Algorithm_81(D,Result);
    break;
    
  case 82:
    Algorithm_82(D,Result);
    break;
    
  case 83:
    Algorithm_83(D,Result);   
    break; 
    
  case 84:
    Algorithm_84(D,Result);  
    break;
    
  case 85:
    Algorithm_85(D,Result);
    break;
    
  case 86:
    Algorithm_86(D,Result);
    break;
    
  case 87:
    Algorithm_87(D,Result);
    break;
    
  case 88:
    Algorithm_88(D,Result);
    break;
    
  case 89:
    Algorithm_89(D,Result);
    break;
    
  case 90:
    Algorithm_90(D,Result);
    break; 
    
  case 91:
    Algorithm_91(D,Result);
    break;
    
  case 92:
    Algorithm_92(D,Result);
    break;
    
  case 93:
    Algorithm_93(D,Result);
    break;
    
  case 94:
    Algorithm_94(D,Result);
    break;
    
  case 95:
    Algorithm_95(D,Result);
    break;
    
  case 96:
    Algorithm_96(D,Result);
    break;
    
  case 97:
    Algorithm_97(D,Result);
    break; 
    
  case 98:
    Algorithm_98(D,Result);
    break;
    
  case 99:
    Algorithm_99(D,Result);
    break;
    
  case 100:
    Algorithm_100(D,Result);
    break;
    
  case 101:
    Algorithm_101(D,Result);
    break;
    
  case 102:
    Algorithm_102(D,Result);
    break;
    
  case 103:
    Algorithm_103(D,Result);   
    break; 
    
  case 104:
    Algorithm_104(D,Result);  
    break;
    
  case 105:
    Algorithm_105(D,Result);
    break;
    
  case 106:
    Algorithm_106(D,Result);
    break;
    
  case 107:
    Algorithm_107(D,Result);
    break;
    
  case 108:
    Algorithm_108(D,Result);
    break;
    
  case 109:
    Algorithm_109(D,Result);
    break;
    
  case 110:
    Algorithm_110(D,Result);
    break; 
    
  case 111:
    Algorithm_111(D,Result);
    break;
    
  case 112:
    Algorithm_112(D,Result);
    break;
    
  case 113:
    Algorithm_113(D,Result);
    break;
    
  case 114:
    Algorithm_114(D,Result);
    break;
    
  case 115:
    Algorithm_115(D,Result);
    break;
    
  case 116:
    Algorithm_116(D,Result);
    break;
    
  case 117:
    Algorithm_117(D,Result);
    break; 
    
  case 118:
    Algorithm_118(D,Result);
    break;
    
  case 119:
    Algorithm_119(D,Result);
    break;
    
  case 120:
    Algorithm_120(D,Result);
    break;
  case 121:
    Algorithm_121(D,Result);
    break;
    
  case 122:
    Algorithm_122(D,Result);
    break;
    
  case 123:
    Algorithm_123(D,Result);
    break;
    
  case 124:
    Algorithm_124(D,Result);
    break;
    
  case 125:
    Algorithm_125(D,Result);
    break;
    
  case 126:
    Algorithm_126(D,Result);
    break;
    
  case 127:
    Algorithm_127(D,Result);
    break; 
  }
  
  return (Result[3]<<24|Result[2]<<16|Result[1]<<8|Result[0]);
}





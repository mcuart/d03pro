//#include "stm32f10x.h"
#include "SWD_main.h"
#include "DAP_config.h"
#include "main.h"
#include "DAP.h"
#include "flashloader.h"
#include "stdbool.h"
#include "utils.h"
#include "use_flashloader.h"
#include <cJSON/cJSON.h>
#include "config.h"





extern u8 unlock_chip_ok_flag;
uint8_t  USB_Request [DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
uint8_t  USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Response Buffer






u32 tmp=0;
u8 CancelRDP_F0(SWD_HANDLE *pswd);
u8 CancelRDP_F1(SWD_HANDLE *pswd);
u8 CancelRDP_F2(SWD_HANDLE *pswd);
u8 CancelRDP_F3(SWD_HANDLE *pswd);
u8 CancelRDP_F4(SWD_HANDLE *pswd);
u8 CancelRDP_F7(SWD_HANDLE *pswd);
u8 CancelRDP_H7(SWD_HANDLE *pswd);
u8 CancelRDP_E23(SWD_HANDLE *pswd);
u8 CancelRDP_L0(SWD_HANDLE *pswd);
u8 CancelRDP_L1(SWD_HANDLE *pswd);
u8 CancelRDP_L4(SWD_HANDLE *pswd);
u8 CancelRDP_G4(SWD_HANDLE *pswd);
u8 CancelRDP_G0(SWD_HANDLE *pswd);
u8 CancelRDP_E501(SWD_HANDLE *pswd);
u8 CancelRDP_mm32f0010(SWD_HANDLE *pswd);

u8 unlockflash(SWD_HANDLE *pswd);



extern u8 global_error_flag;




u8 SWD_Auto_detect_start_1(SWD_HANDLE *pswd)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
  u32 tmp=0;
  
  hardResetTarget(pswd);
  USB_Request[USB_RequestOut][0]=ID_DAP_Connect;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  // System_Delay_ms(200);
  
  
  tmp=18000000;
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Clock;
  
  USB_Request[USB_RequestOut][1]=tmp;
  USB_Request[USB_RequestOut][2]=tmp>>8;
  USB_Request[USB_RequestOut][3]=tmp>>16;
  USB_Request[USB_RequestOut][4]=tmp>>24;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_TransferConfigure;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=200;
  USB_Request[USB_RequestOut][3]=1;
  USB_Request[USB_RequestOut][4]=50;
  USB_Request[USB_RequestOut][5]=0;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWD_Configure;
  USB_Request[USB_RequestOut][1]=0;
  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=16;
  USB_Request[USB_RequestOut][2]=0x9E;
  USB_Request[USB_RequestOut][3]=0xE7; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=8;
  USB_Request[USB_RequestOut][2]=0x00;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x02;//读DP 0x01BA01477
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=0x1E;
  USB_Request[USB_RequestOut][3]=0x00;
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT 0
  //  USB_Request[USB_RequestOut][4]=0x00;
  //  USB_Request[USB_RequestOut][5]=0x00;
  //  USB_Request[USB_RequestOut][6]=0x00;
  //  USB_Request[USB_RequestOut][7]=0x00;  
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x04;//写DP-CTRL/STAT
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00;
  USB_Request[USB_RequestOut][6]=0x00;
  USB_Request[USB_RequestOut][7]=0x50;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x06;//读DP-CTRL/STAT
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=0x1E;
  //  USB_Request[USB_RequestOut][3]=0x00;
  //  USB_Request[USB_RequestOut][4]=0x00;
  //  USB_Request[USB_RequestOut][5]=0x00; 
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT 15
  //  USB_Request[USB_RequestOut][4]=0xF0;
  //  USB_Request[USB_RequestOut][5]=0x00;
  //  USB_Request[USB_RequestOut][6]=0x00;
  //  USB_Request[USB_RequestOut][7]=0x00;  
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  
  //  
  //  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x0F;//读AHB-AP ID 1;770011
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  //////////////////////////////////////////////////////////////////////////////
  //  
  //  
  if(((USB_Response[0][2])&0x0C)!=0x00)
  {
    return 1;
  }
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT [7-4]=0000   选中AHB-AP BANK0  
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00;
  USB_Request[USB_RequestOut][6]=0x00;
  USB_Request[USB_RequestOut][7]=0x00;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
  
  /********************数据通信位宽设置为32bit位宽*************************/  
  Write_AP_CSW(pswd, AP_CSW_DEFAULT);
  
  
  /********************Halts the target CPU*************************/  
  resetAndHaltTarget_20180327(pswd);
  WriteAbort_Select(pswd);
  return 0;
}



u8 SWD_Auto_detect_start_2(SWD_HANDLE * pswd)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
  u32 tmp=0;
  
  hardResetTarget(pswd);
  USB_Request[USB_RequestOut][0]=2;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  // System_Delay_ms(200);
  
  
  tmp=18000000;
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Clock;
  
  USB_Request[USB_RequestOut][1]=tmp;
  USB_Request[USB_RequestOut][2]=tmp>>8;
  USB_Request[USB_RequestOut][3]=tmp>>16;
  USB_Request[USB_RequestOut][4]=tmp>>24;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_TransferConfigure;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=200;
  USB_Request[USB_RequestOut][3]=1;
  USB_Request[USB_RequestOut][4]=50;
  USB_Request[USB_RequestOut][5]=0;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWD_Configure;
  USB_Request[USB_RequestOut][1]=0;
  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=16;
  USB_Request[USB_RequestOut][2]=0x9E;
  USB_Request[USB_RequestOut][3]=0xE7; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=8;
  USB_Request[USB_RequestOut][2]=0x00;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x02;//读DP 0x01BA01477
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=0x1E;
  USB_Request[USB_RequestOut][3]=0x00;
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT 0
  //  USB_Request[USB_RequestOut][4]=0x00;
  //  USB_Request[USB_RequestOut][5]=0x00;
  //  USB_Request[USB_RequestOut][6]=0x00;
  //  USB_Request[USB_RequestOut][7]=0x00;  
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x04;//写DP-CTRL/STAT
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00;
  USB_Request[USB_RequestOut][6]=0x00;
  USB_Request[USB_RequestOut][7]=0x50;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x06;//读DP-CTRL/STAT
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=0x1E;
  //  USB_Request[USB_RequestOut][3]=0x00;
  //  USB_Request[USB_RequestOut][4]=0x00;
  //  USB_Request[USB_RequestOut][5]=0x00; 
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT 15
  //  USB_Request[USB_RequestOut][4]=0xF0;
  //  USB_Request[USB_RequestOut][5]=0x00;
  //  USB_Request[USB_RequestOut][6]=0x00;
  //  USB_Request[USB_RequestOut][7]=0x00;  
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  
  //  
  //  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x0F;//读AHB-AP ID 1;770011
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  //////////////////////////////////////////////////////////////////////////////
  //  
  //  
  if(((USB_Response[0][2])&0x0C)!=0x00)
  {
    return 1;
  }
  
  
  
  return 0;
}



u8 SWD_Auto_detect_end(SWD_HANDLE * pswd)
{
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT 0
  //  USB_Request[USB_RequestOut][4]=0x00;
  //  USB_Request[USB_RequestOut][5]=0x00;
  //  USB_Request[USB_RequestOut][6]=0x00;
  //  USB_Request[USB_RequestOut][7]=0x00;  
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  
  //  
  //  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x04;//写DP-CTRL/STAT
  //  USB_Request[USB_RequestOut][4]=0x00;
  //  USB_Request[USB_RequestOut][5]=0x00;
  //  USB_Request[USB_RequestOut][6]=0x00;
  //  USB_Request[USB_RequestOut][7]=0x50;  
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  
  //  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x06;//读DP-CTRL/STAT
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  
  //  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=0x1E;
  //  USB_Request[USB_RequestOut][3]=0x00;
  //  USB_Request[USB_RequestOut][4]=0x00;
  //  USB_Request[USB_RequestOut][5]=0x00; 
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  
  //  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT 15
  //  USB_Request[USB_RequestOut][4]=0xF0;
  //  USB_Request[USB_RequestOut][5]=0x00;
  //  USB_Request[USB_RequestOut][6]=0x00;
  //  USB_Request[USB_RequestOut][7]=0x00;  
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  
  //  
  //  
  //  
  //  
  //  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //  USB_Request[USB_RequestOut][1]=0;
  //  USB_Request[USB_RequestOut][2]=1;
  //  USB_Request[USB_RequestOut][3]=0x0F;//读AHB-AP ID 14770011
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  //////////////////////////////////////////////////////////////////////////////
  //  do
  //  {
  //    USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //    USB_Request[USB_RequestOut][1]=0;
  //    USB_Request[USB_RequestOut][2]=1;
  //    USB_Request[USB_RequestOut][3]=0x0F;//读AHB-AP ID 14770011
  //    DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //    WriteAbort_Select(pswd);
  //    System_Delay_ms(10);
  //  }while(USB_Response[0][1]!=0);
  //  
  //  return 0;
  
  
  
  
//  while(SWD_Auto_detect_start_2()==0)
//  {
//    System_Delay_ms(30);
//  }
  
  if(SWD_Auto_detect_start_2(pswd)==0)
  {
      return 0;//有芯片 芯片还未取走
  }
  else
  {
      return 1;//无芯片 芯片已经拿走
  }

  
}



u8 SWD_end(SWD_HANDLE *pswd)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index

  
  USB_Request[USB_RequestOut][0]=ID_DAP_Disconnect; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  return 0;
}


u8 SWD_main(SWD_HANDLE * pswd)
{
  stm32_mcu_info stm32_mcu;
  u32 tmp=0;
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
  
  
  hardResetTarget(pswd);
  USB_Request[USB_RequestOut][0]=2;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);

  
  tmp=18000000;
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Clock;
  
  USB_Request[USB_RequestOut][1]=tmp;
  USB_Request[USB_RequestOut][2]=tmp>>8;
  USB_Request[USB_RequestOut][3]=tmp>>16;
  USB_Request[USB_RequestOut][4]=tmp>>24;
  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_TransferConfigure;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=200;
  USB_Request[USB_RequestOut][3]=1;
  USB_Request[USB_RequestOut][4]=50;
  USB_Request[USB_RequestOut][5]=0;
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWD_Configure;
  USB_Request[USB_RequestOut][1]=0;
  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=16;
  USB_Request[USB_RequestOut][2]=0x9E;
  USB_Request[USB_RequestOut][3]=0xE7; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=8;
  USB_Request[USB_RequestOut][2]=0x00;
  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x02;//读DP 0x01BA01477
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  /////
  u32 idcode=USB_Response[0][6]<<24|USB_Response[0][5]<<16|USB_Response[0][4]<<8|USB_Response[0][3];
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=0x1E;
  USB_Request[USB_RequestOut][3]=0x00;
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  

  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x04;//写DP-CTRL/STAT
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00;
  USB_Request[USB_RequestOut][6]=0x00;
  USB_Request[USB_RequestOut][7]=0x50;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x06;//读DP-CTRL/STAT
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  

  if(((USB_Response[0][2])&0x0C)!=0x00)
  {
    return 1;
  }
  
  
  display_detect_chip_id_code_ok(idcode);
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT [7-4]=0000   选中AHB-AP BANK0  
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00;
  USB_Request[USB_RequestOut][6]=0x00;
  USB_Request[USB_RequestOut][7]=0x00;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
  
  /********************数据通信位宽设置为32bit位宽*************************/  
  Write_AP_CSW(pswd, AP_CSW_DEFAULT);
  
  
  /********************Halts the target CPU*************************/  
  //  haltTarget();
  resetAndHaltTarget_20180327(pswd);
  WriteAbort_Select(pswd);

  
  
  unlock_chip_ok_flag=0;
  
  stm32_mcu.flash_loader_index=(u8)cJSON_GetObjectItem(pswd->root, "exe")->valueint;//;
  if(unlockflash(pswd)!=0)
  {
    display_error_code(stm32_mcu.flash_loader_index+0xA0000000);
    return 2;
  }
  
  
  if(flashWithFlashloader(pswd)!=0)
  {
    display_error_code(stm32_mcu.flash_loader_index+0xB0000000);
    return 3;
  }
  
  
  return 0;
}


u8 re_connect=0;

#define FLASH_FLAG_BSY                 ((uint32_t)0x00000001)  /*!< FLASH Busy flag */
u8 unlockflash(SWD_HANDLE * pswd)
{
  stm32_mcu_info stm32_mcu;
  u32 index=stm32_mcu.flash_loader_index=(u8)cJSON_GetObjectItem(pswd->root, "exe")->valueint;//;
  u8 ret=0;
  re_connect=0;
  switch(index)
  {
  case 0x00:
    ret=CancelRDP_F0(pswd);
    
    break;
  case 0x01:
    
    ret=CancelRDP_F1(pswd);
    break;
  case 0x02:
    ret=CancelRDP_F2(pswd);    
    
    break;
  case 0x03:
    ret=CancelRDP_F3(pswd);
    
    break;
  case 0x04:
    ret=CancelRDP_F4(pswd); 
    
    break;
  case 0x05:
    
    readMem(pswd, 0x40023C14); 
    break;
  case 0x06:
    readMem(pswd, 0x5200201C); 
    
    break;
  case 0x07:
    
    ret=CancelRDP_F7(pswd); 
    break;
  case 0x08:
    // readMem(pswd, 0x40023C1C);    
    ret=CancelRDP_H7(pswd);
    break;
  case 0x09:
    readMem(pswd, 0x40022020);
    
    break;
    
  case 0x0A:
    
    ret=CancelRDP_E23(pswd); 
    break;    
    
  case 0x0B:
    
    ret=CancelRDP_G0(pswd); 
    break;    
    
  case 0x0C:
    
    ret=CancelRDP_E501(pswd); 
    break;  
    
    
  case 0x0D:
    
    ret=CancelRDP_G4(pswd); 
    break;     
    
  case 0x10:
    
    ret=CancelRDP_L0(pswd); 
    break;
    
  case 0x11:
    
    ret=CancelRDP_L1(pswd); 
    break;    
    
  case 0x14:
    
    ret=CancelRDP_L4(pswd); 
    break;
    
  case 0x15://MM32F0
    
    
    break;
    
  case 0x16://MM32F1
    
    
    break;
    
  case 0x18://mm32f0010
    ret=CancelRDP_mm32f0010(pswd);
    
    break;       
    
    
  default:
    
    
    break;
    
  }
  
  if(ret)
  {
    return 1;
  }
  
  if(EnterProgramMode(pswd)!=0)
  {
    return 2;
  }
  writeMem(pswd, (uint32_t)&(SCB->AIRCR),0x05fa0004);//这一句可以清除错误特别重要比如lockup
  if(EnterProgramMode(pswd)!=0)//这里和前面的那个重复目的是为了解决GD32芯片解锁FLASH清除FLASH以后开始写FLASH的时候报错。如果没有这句将会导致能清除芯片但是写flash过程开始就会报错。失败。这个是深圳客户板子发现的
  {
    return 2;
  }
  
  
  return 0;
}


u8 CancelRDP_F0(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  // hardResetTarget_1();
  
#define RDPRT_Mask               ((uint32_t)0x00000002)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x4002201C);
  if((tmp&RDPRT_Mask)==RDPRT_Mask)
  { 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0xcdef89ab);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xcdef89ab);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000220);//FLASH_CR  选择选项字节擦除功能
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000260);//FLASH_CR  启动选项字节擦除功能
    WriteAbort_Select(pswd);
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
    //  System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000210); 
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1FFFF800);//写AHB-AP Transfer Address TAR
    Write_AP_CSW(pswd, 0x22000001);//写AHB-AP Control and Status Word      Word (16-bits)
    writeAP_data(pswd, 0x000000AA);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    Write_AP_CSW(pswd, 0x22000002);//写AHB-AP Control and Status Word      Word (32-bits)
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000200); 
    WriteAbort_Select(pswd);
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00002200); 
    WriteAbort_Select(pswd);
    
    
    display_unlock_chip_ok();
    return 0;
    
  }
  
  return 0;
}




u8 CancelRDP_F1(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  //  hardResetTarget_1();
#define RDPRT_Mask               ((uint32_t)0x00000002)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x4002201C);
  if((tmp&RDPRT_Mask)==RDPRT_Mask)
  {
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0xcdef89ab);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xcdef89ab);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000220);//FLASH_CR 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000260);//FLASH_CR  
    WriteAbort_Select(pswd);
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);//FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
//      System_Delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000210); 
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1FFFF800);//写AHB-AP Transfer Address TAR
    // coresight_value=Read_AP_CSW();
    Write_AP_CSW(pswd, 0x22000001);//写AHB-AP Control and Status Word      Word (16-bits)
    //  coresight_value=Read_AP_CSW();
    writeAP_data(pswd, 0x000000A5);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);
    //  coresight_value=Read_AP_CSW();
    Write_AP_CSW(pswd, 0x22000002);//写AHB-AP Control and Status Word      Word (32-bits)
    //  coresight_value=Read_AP_CSW();
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);//FLASH_SR
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
//      System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000240); 
    WriteAbort_Select(pswd);
    
    display_unlock_chip_ok();
    return 0;
  }
  
  return 0;
}



u8 CancelRDP_E501(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  //  hardResetTarget_1();
#define RDPRT_Mask               ((uint32_t)0x00000002)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x4002201C);
  if((tmp&RDPRT_Mask)==RDPRT_Mask)
  {
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0xcdef89ab);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xcdef89ab);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000220);//FLASH_CR 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000260);//FLASH_CR  
    WriteAbort_Select(pswd);
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
    //  System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000210); 
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1FFFF800);//写AHB-AP Transfer Address TAR
    
    writeAP_data(pswd, 0xFFFF5AA5);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1FFFF804);//写AHB-AP Transfer Address TAR
    
    writeAP_data(pswd, 0xFFFFFFFF);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);
    //  coresight_value=Read_AP_CSW();
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
    //  System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    
    
    
    
    
    
    display_unlock_chip_ok();
    return 0;
  }
  
  return 0;
}




u8 CancelRDP_mm32f0010(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  //  hardResetTarget_1();
#define RDPRT_Mask               ((uint32_t)0x00000002)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  //  tmp=readMem(pswd, 0x4002201C);
  //  if((tmp&RDPRT_Mask)==RDPRT_Mask)
  {
    re_connect=1;
    
    return 0;
  }
  
  return 0;
}

u8 CancelRDP_F2(SWD_HANDLE *pswd)
{
  u32 timeout=0;
#define RDPRT_Mask               ((uint32_t)0x0000FF00)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OPTCR
  tmp=readMem(pswd, 0x40023C14);
  if((tmp&RDPRT_Mask)!=0x0000AA00)
  {
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C04,0x45670123);//FLASH_KEYR1
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C04,0xcdef89ab);//FLASH_KEYR2
    WriteAbort_Select(pswd);
    
    //tmp=readMem(pswd, 0x40023C14);//读FLASH_OPTCR 正常应该=0x0fff00ed   说明OPTLOCK=1
    WriteDP_AP_Address_Data(pswd, 0x40023C08,0x08192A3B);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C08,0x4C5D6E7F);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    
    //tmp=readMem(pswd, 0x40023C14);//读FLASH_OPTCR 正常应该=0x0fff00ec   说明OPTLOCK=0
    WriteDP_AP_Address_Data(pswd, 0x40023C14,0x0fffaaeC);//FLASH_OPTCR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C14,0x0fffaaee);//FLASH_OPTCR
    WriteAbort_Select(pswd);
    timeout=16000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40023C0C);//读FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
    //  System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&0x00010000)==0x00010000)&&timeout);
    if(timeout==0)
    {
      return 2;
    }
    display_unlock_chip_ok();
    return 0;
  }
  return 0;
}


u8 CancelRDP_F3(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
#define RDPRT_Mask               ((uint32_t)0x00000002)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x4002201C);
  if((tmp&RDPRT_Mask)==RDPRT_Mask)
  { 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0xcdef89ab);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xcdef89ab);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000220);//FLASH_CR  选择选项字节擦除功能
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000260);//FLASH_CR  启动选项字节擦除功能
    WriteAbort_Select(pswd);
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
    //  System_Delay_ms(1);
      delay_ms(1);
      timeout--;
      
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000210); 
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1FFFF800);//写AHB-AP Transfer Address TAR
    Write_AP_CSW(pswd, 0x22000001);//写AHB-AP Control and Status Word      Word (16-bits)
    writeAP_data(pswd, 0x000000AA);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    Write_AP_CSW(pswd, 0x22000002);//写AHB-AP Control and Status Word      Word (32-bits)
    timeout=1000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(10);
      delay_ms(10);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 4;
    }
    
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000200); 
    WriteAbort_Select(pswd);
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00002200); 
    WriteAbort_Select(pswd);
  }
  return 0;
}

u8 CancelRDP_F4(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
  
#define RDPRT_Mask               ((uint32_t)0x0000FF00)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OPTCR
  tmp=readMem(pswd, 0x40023C14);
  if((tmp&RDPRT_Mask)!=0x0000AA00)
  {
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C04,0x45670123);//FLASH_KEYR1
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C04,0xcdef89ab);//FLASH_KEYR2
    WriteAbort_Select(pswd);
    
    //  tmp=readMem(pswd, 0x40023C14);//读FLASH_OPTCR 正常应该=0x0fff00ed   说明OPTLOCK=1
    WriteDP_AP_Address_Data(pswd, 0x40023C08,0x08192A3B);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C08,0x4C5D6E7F);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    
    //tmp=readMem(pswd, 0x40023C14);//读FLASH_OPTCR 正常应该=0x0fff00ec   说明OPTLOCK=0
    WriteDP_AP_Address_Data(pswd, 0x40023C14,0x0fffaaeC);//FLASH_OPTCR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C14,0x0fffaaee);//FLASH_OPTCR
    WriteAbort_Select(pswd);
    timeout=16000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40023C0C);//读FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
    //  System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&0x00010000)==0x00010000)&&timeout);
    if(timeout==0)
    {
      return 2;
    }
    
    display_unlock_chip_ok();
    return 0;
    
  }
  return 0;
}



u8 CancelRDP_F7(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
#define RDPRT_Mask               ((uint32_t)0x0000FF00)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OPTCR
  tmp=readMem(pswd, 0x40023C14);
  if((tmp&RDPRT_Mask)!=0x0000AA00)
  {
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C04,0x45670123);//FLASH_KEYR1
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C04,0xcdef89ab);//FLASH_KEYR2
    WriteAbort_Select(pswd);
    
    //tmp=readMem(pswd, 0x40023C14);//读FLASH_OPTCR 正常应该=0x0fff00ed   说明OPTLOCK=1
    WriteDP_AP_Address_Data(pswd, 0x40023C08,0x08192A3B);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C08,0x4C5D6E7F);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    
    //  tmp=readMem(pswd, 0x40023C18);//读FLASH_OPTCR1 正常应该=0xFF7F0080   
    WriteDP_AP_Address_Data(pswd, 0x40023C18,0xFF7F0080);//FLASH_OPTCR1
    WriteAbort_Select(pswd);
    
    
    // tmp=readMem(pswd, 0x40023C14);//读FLASH_OPTCR 正常应该=0xFFFFAAFE   说明OPTLOCK=0
    WriteDP_AP_Address_Data(pswd, 0x40023C14,0xFFFFAAFE);//FLASH_OPTCR
    WriteAbort_Select(pswd);
    timeout=16000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      
      tmp=readMem(pswd, 0x40023C0C);//读FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
    //  System_Delay_ms(1);
      delay_ms(1);
      
      timeout--;
    }while(((tmp&0x00010000)==0x00010000)&&timeout);
    if(timeout==0)
    {
      return 2;
    }
    
    display_unlock_chip_ok();
    return 0;
  }
  return 0;
}

u8 CancelRDP_H7(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
#define RDPRT_Mask               ((uint32_t)0x0000FF00)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OPTCR
  tmp=readMem(pswd, 0x5200201C);//读FLASH_OPTSR_CUR
  if((tmp&RDPRT_Mask)!=0x0000AA00)
  {
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002004,0x45670123);//FLASH_KEYR1
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002004,0xcdef89ab);//FLASH_KEYR1
    WriteAbort_Select(pswd);
    
    //tmp=readMem(pswd, 0x40023C14);//读FLASH_OPTCR 正常应该=0x0fff00ed   说明OPTLOCK=1
    WriteDP_AP_Address_Data(pswd, 0x52002008,0x08192A3B);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002008,0x4C5D6E7F);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    
    tmp=readMem(pswd, 0x5200200C);//读FLASH_CR1   
    tmp=readMem(pswd, 0x52002018);//读FLASH_OPTCR
    tmp=readMem(pswd, 0x5200210C);//读FLASH_CR2  
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002104,0x45670123);//FLASH_KEYR2
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002104,0xcdef89ab);//FLASH_KEYR2
    WriteAbort_Select(pswd);
    
    
    tmp=readMem(pswd, 0x5200210C);//读FLASH_CR2  
    tmp=readMem(pswd, 0x52002010);//读FLASH_SR1
    
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002014,0xffffffff);//写FLASH_CCR1=0xffffffff
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002114,0xffffffff);//写FLASH_CCR2=0xffffffff
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002024,0xffffffff);//写FLASH_OPTCCR=0xffffffff
    
    
    
    
    
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002020,0x03d6aaf0);//写FLASH_OPTSR_PRG=0x03d6aaf0 
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x5200202c,0x000000ff);//写FLASH_PRAR_PRG1=0x000000ff
    
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002034,0x000000ff);//写FLASH_SCAR_PRG1=0x00000fff
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x5200203c,0x000000ff);//写FLASH_WPSN_PRG1R=0x000000ff  
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002044,0x1ff00800);//写FLASH_BOOT_PRGR=0x1ff00800    
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x5200212c,0x000000ff);//写FLASH_PRAR_PRG2=0x000000ff   
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x52002134,0x000000ff);//写FLASH_SCAR_PRG2=0x00000fff    
    
    
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x5200213c,0x000000ff);//写FLASH_WPSN_PRG2R=0x000000ff      
    
    
    
    
    // FLASH_OPTSR_PRG
    WriteDP_AP_Address_Data(pswd, 0x52002018,0x00000002);//写FLASH_OPTCR=0x00000002  令OPTSTART=1 触发开始编程选项字节
    WriteAbort_Select(pswd);
    timeout=16000+8000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x58004800,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      
      tmp=readMem(pswd, 0x5200201C);//读FLASH_OPTSR_CUR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      
      timeout--;
    }while(((tmp&0x00000001)==0x00000001)&&timeout);
    if(timeout==0)
    {
      return 2;
    }
    
    display_unlock_chip_ok();
    return 0;
  }
  return 0;
}






u8 CancelRDP_L0(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
  // hardResetTarget_1();
  
#define RDPRT_Mask               ((uint32_t)0x000000FF)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  //  tmp=readMem(pswd, 0x4002201C);
  // tmp1=readMem(pswd, 0x40022020);
  //  if(((tmp&RDPRT_Mask)!=0x000000AA))
  { 
    /* Unlocking FLASH_PECR register access*/
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C,0x89ABCDEF);//FLASH_PEKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C,0x02030405);//FLASH_PEKEYR
    
    
    /* Unlocking the option bytes block access */
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022014,0xFBEAD9C8);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022014,0x24252627);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40022018);//FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1ff80000);//写AHB-AP Transfer Address TAR
    writeAP_data(pswd, 0xff5500AA);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40022018);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////////////////////        
    ///////////////////////////////////////////////////////////////////////////////////////////    
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1ff80004);//写AHB-AP Transfer Address TAR
    writeAP_data(pswd, 0xff870078);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40022018);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
      //System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    
    
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1ff80008);//写AHB-AP Transfer Address TAR
    writeAP_data(pswd, 0xFFFF0000);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40022018);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    //    
    //    WriteAbort_Select(pswd);
    //    writeAP_TAR(pswd, 0x1ff8000C);//写AHB-AP Transfer Address TAR
    //    writeAP_data(pswd, 0xFFFF0000);//写AHB-AP Data Read/Write DRW
    //    WriteAbort_Select(pswd);    
    //    
    //    timeout=10000;
    //    do
    //    {
    //      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
    //      WriteAbort_Select(pswd);
    //      tmp=readMem(pswd, 0x40022018);
    //      if(global_error_flag!=0)
    //      {
    //        return 3;
    //      }
    //      WriteAbort_Select(pswd);
    //      System_Delay_ms(1);
    //      timeout--;
    //    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    //    if(timeout==0)
    //    {
    //      return 4;
    //    }    
    
    ///////////////////////////////////////////////////////////////////////////////////////////        
    ///////////////////////////////////////////////////////////////////////////////////////////        
    WriteDP_AP_Address_Data(pswd, 0x40022004,0x00040000);//OBL_LAUNCH=1强制加载已经去除读保护的选项字节
    WriteAbort_Select(pswd);
    
    
    display_unlock_chip_ok();
    return 0;
    
  }
  
  return 0;
}



u8 CancelRDP_L1(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
#define RDPRT_Mask               ((uint32_t)0x000000FF)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  //  tmp=readMem(pswd, 0x40023C1C);
  // tmp1=readMem(pswd, 0x40022020);
  // if(((tmp&RDPRT_Mask)!=0x000000AA))
  { 
    /* Unlocking FLASH_PECR register access*/
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C0C,0x89ABCDEF);//FLASH_PEKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C0C,0x02030405);//FLASH_PEKEYR
    
    
    /* Unlocking the option bytes block access */
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C14,0xFBEAD9C8);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40023C14,0x24252627);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40023C18);//FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1ff80000);//写AHB-AP Transfer Address TAR
    writeAP_data(pswd, 0xff5500AA);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40023C18);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////////////////////        
    ///////////////////////////////////////////////////////////////////////////////////////////    
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1ff80004);//写AHB-AP Transfer Address TAR
    writeAP_data(pswd, 0xff0700F8);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40023C18);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
      //System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    
    
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1ff80008);//写AHB-AP Transfer Address TAR
    writeAP_data(pswd, 0xFFFF0000);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40023C18);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    //    
    //    WriteAbort_Select(pswd);
    //    writeAP_TAR(pswd, 0x1ff8000C);//写AHB-AP Transfer Address TAR
    //    writeAP_data(pswd, 0xFFFF0000);//写AHB-AP Data Read/Write DRW
    //    WriteAbort_Select(pswd);    
    //    
    //    timeout=10000;
    //    do
    //    {
    //      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
    //      WriteAbort_Select(pswd);
    //      tmp=readMem(pswd, 0x40022018);
    //      if(global_error_flag!=0)
    //      {
    //        return 3;
    //      }
    //      WriteAbort_Select(pswd);
    //      System_Delay_ms(1);
    //      timeout--;
    //    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    //    if(timeout==0)
    //    {
    //      return 4;
    //    }    
    
    ///////////////////////////////////////////////////////////////////////////////////////////        
    ///////////////////////////////////////////////////////////////////////////////////////////        
    WriteDP_AP_Address_Data(pswd, 0x40023C04,0x00040000);//OBL_LAUNCH=1强制加载已经去除读保护的选项字节
    WriteAbort_Select(pswd);
    
    
    display_unlock_chip_ok();
    return 0;
    
  }
  
  return 0;
}



u8 CancelRDP_L4(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
  // hardResetTarget_1();
  
#define RDPRT_Mask               ((uint32_t)0x000000FF)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x40022020);
  // tmp1=readMem(pswd, 0x40022020);
  if(((tmp&RDPRT_Mask)!=0x000000AA))
  { 
    /* Unlocking FLASH_KEYR register access*/
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xCDEF89AB);//FLASH_KEYR
    
    
    /* Unlocking the option bytes block access */
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C,0x08192A3B);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C, 0x4C5D6E7F);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    
    
    
    //  WriteDP_AP_Address_Data(pswd, 0x40022020,0xFFEFF8AA);//FLASH_OPTR
    WriteDP_AP_Address_Data(pswd, 0x40022020,0xFF8FF8AA);//FLASH_OPTR
    
    WriteAbort_Select(pswd);
    
    
    
    WriteDP_AP_Address_Data(pswd, 0x40022014,0x00020000);//FLASH_CR
    WriteAbort_Select(pswd);   
    
    timeout=16000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40022010);//FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&0x00010000)==0x00010000)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    WriteDP_AP_Address_Data(pswd, 0x40022014,0x08000000);//FLASH_CR
    WriteAbort_Select(pswd);   
    
    display_unlock_chip_ok();
    return 0;
    
  }
  
  return 0;
}







u8 CancelRDP_E23(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  // hardResetTarget_1();
  
#define RDPRT_Mask               ((uint32_t)0x00000002)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x4002201C);
  if((tmp&RDPRT_Mask)==RDPRT_Mask)
  { 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022004,0xcdef89ab);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xcdef89ab);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000220);//FLASH_CR  选择选项字节擦除功能
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000260);//FLASH_CR  启动选项字节擦除功能
    WriteAbort_Select(pswd);
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
      //System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000210); 
    WriteAbort_Select(pswd);
    writeAP_TAR(pswd, 0x1FFFF800);//写AHB-AP Transfer Address TAR
    //   Write_AP_CSW(0x22000001);//写AHB-AP Control and Status Word      Word (16-bits)
    writeAP_data(pswd, 0x00FF5AA5);//写AHB-AP Data Read/Write DRW
    WriteAbort_Select(pswd);    
    //    Write_AP_CSW(0x22000002);//写AHB-AP Control and Status Word      Word (32-bits)
    timeout=10000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x4002200C);
      if(global_error_flag!=0)
      {
        return 3;
      }
      WriteAbort_Select(pswd);
     // System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&FLASH_FLAG_BSY)==FLASH_FLAG_BSY)&&timeout);
    if(timeout==0)
    {
      return 4;
    }
    
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00000200); 
    WriteAbort_Select(pswd);
    
    WriteDP_AP_Address_Data(pswd, 0x40022010,0x00002200); 
    WriteAbort_Select(pswd);
    
    
    display_unlock_chip_ok();
    return 0;
    
  }
  
  return 0;
}



u8 CancelRDP_G0(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  
  // hardResetTarget_1();
  
#define RDPRT_Mask               ((uint32_t)0x000000FF)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x40022020);
  // tmp1=readMem(pswd, 0x40022020);
  if(((tmp&RDPRT_Mask)!=0x000000AA))
  { 
    /* Unlocking FLASH_KEYR register access*/
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xCDEF89AB);//FLASH_KEYR
    
    
    /* Unlocking the option bytes block access */
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C,0x08192A3B);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C, 0x4C5D6E7F);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    
    
    
    WriteDP_AP_Address_Data(pswd, 0x40022020,0xFFEFF8AA);//FLASH_OPTR
    WriteAbort_Select(pswd);
    
    
    
    WriteDP_AP_Address_Data(pswd, 0x40022014,0x00020000);//FLASH_CR
    WriteAbort_Select(pswd);   
    
    timeout=16000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40022010);//FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
      //System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&0x00010000)==0x00010000)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    WriteDP_AP_Address_Data(pswd, 0x40022014,0x08000000);//FLASH_CR
    WriteAbort_Select(pswd);   
    
    display_unlock_chip_ok();
    return 0;
    
  }
  
  return 0;
}



u8 CancelRDP_G4(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  u8 OPTR_DBANK=0;
  // hardResetTarget_1();
  
#define RDPRT_Mask               ((uint32_t)0x000000FF)
  /*检测optionbyte，如果有保护则解除保护*/
  //读FLASH_OBR  
  tmp=readMem(pswd, 0x40022020);
  // tmp1=readMem(pswd, 0x40022020);
  if(((tmp&RDPRT_Mask)!=0x000000AA))
  { 
    /* Unlocking FLASH_KEYR register access*/
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_KEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x40022008,0xCDEF89AB);//FLASH_KEYR
    
    
    /* Unlocking the option bytes block access */
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C,0x08192A3B);//FLASH_OPTKEYR
    WriteAbort_Select(pswd);
    WriteDP_AP_Address_Data(pswd, 0x4002200C, 0x4C5D6E7F);//FLASH_OPTKEYR 
    WriteAbort_Select(pswd);
    
    
    
    //  WriteDP_AP_Address_Data(pswd, 0x40022020,0xFFEFF8AA);//FLASH_OPTR
    OPTR_DBANK=cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "option_byte"),3)->valuedouble;
    if(OPTR_DBANK&0x40==0)
      WriteDP_AP_Address_Data(pswd, 0x40022020,0xFFAFF8AA);//FLASH_OPTR      DBANK=0   
    else
      WriteDP_AP_Address_Data(pswd, 0x40022020,0xFFEFF8AA);//FLASH_OPTR      DBANK=1
    
    
    
    WriteAbort_Select(pswd);
    
    
    
    WriteDP_AP_Address_Data(pswd, 0x40022014,0x00020000);//FLASH_CR
    WriteAbort_Select(pswd);   
    
    timeout=16000;
    do
    {
      WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
      WriteAbort_Select(pswd);
      tmp=readMem(pswd, 0x40022010);//FLASH_SR
      if(global_error_flag!=0)
      {
        return 1;
      }
      WriteAbort_Select(pswd);
      //System_Delay_ms(1);
      delay_ms(1);
      timeout--;
    }while(((tmp&0x00010000)==0x00010000)&&timeout);
    
    if(timeout==0)
    {
      return 2;
    }
    
    WriteDP_AP_Address_Data(pswd, 0x40022014,0x08000000);//FLASH_CR
    WriteAbort_Select(pswd);   
    
    display_unlock_chip_ok();
    return 0;
    
  }
  else
  {
    
    
    
    if((u8)cJSON_GetObjectItem(((SWD_HANDLE *)pswd)->root, "write_option_byte")->valueint==1)     
    {
      /* Unlocking FLASH_KEYR register access*/
      WriteAbort_Select(pswd);
      WriteDP_AP_Address_Data(pswd, 0x40022008,0x45670123);//FLASH_KEYR
      WriteAbort_Select(pswd);
      WriteDP_AP_Address_Data(pswd, 0x40022008,0xCDEF89AB);//FLASH_KEYR
      
      
      /* Unlocking the option bytes block access */
      WriteAbort_Select(pswd);
      WriteDP_AP_Address_Data(pswd, 0x4002200C,0x08192A3B);//FLASH_OPTKEYR
      WriteAbort_Select(pswd);
      WriteDP_AP_Address_Data(pswd, 0x4002200C, 0x4C5D6E7F);//FLASH_OPTKEYR 
      WriteAbort_Select(pswd);
      
      
      OPTR_DBANK=cJSON_GetArrayItem(cJSON_GetObjectItem(((SWD_HANDLE *)pswd)->root, "option_byte"),3)->valuedouble;
      if((OPTR_DBANK&0x40)==0)
        WriteDP_AP_Address_Data(pswd, 0x40022020,0xFFAFF8AA);//FLASH_OPTR      DBANK=0   
      else
        WriteDP_AP_Address_Data(pswd, 0x40022020,0xFFEFF8AA);//FLASH_OPTR      DBANK=1
      
      WriteAbort_Select(pswd);
      
      
      
      WriteDP_AP_Address_Data(pswd, 0x40022014,0x00020000);//FLASH_CR
      WriteAbort_Select(pswd);   
      
      timeout=16000;
      do
      {
        WriteDP_AP_Address_Data(pswd, 0x40003000,0x0000AAAA);//IWDG_KR
        WriteAbort_Select(pswd);
        tmp=readMem(pswd, 0x40022010);//FLASH_SR
        if(global_error_flag!=0)
        {
          return 1;
        }
        WriteAbort_Select(pswd);
        //System_Delay_ms(1);
        delay_ms(1);
        timeout--;
      }while(((tmp&0x00010000)==0x00010000)&&timeout);
      
      if(timeout==0)
      {
        return 2;
      }
      
      WriteDP_AP_Address_Data(pswd, 0x40022014,0x08000000);//FLASH_CR
      WriteAbort_Select(pswd);   
      
      display_unlock_chip_ok();
      return 0;         
    }
    
  }
  
  return 0;
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

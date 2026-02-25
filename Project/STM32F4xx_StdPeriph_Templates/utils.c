#include "utils.h"
#include "use_flashloader.h"
#include "config.h"
#include <cJSON/cJSON.h>


//#include "includes.h"
/*uC/OS-III*********************************************************************************************/
#include "os.h"
#include "cpu.h"







extern uint8_t  USB_Request [DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
extern uint8_t  USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Response Buffer
extern u32 tmp;

u8 global_error_flag=0;

u8 waitForRegReady(SWD_HANDLE *pswd);
void hardResetTarget_1(SWD_HANDLE *pswd);




u8 EnterProgramMode(SWD_HANDLE *pswd)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
  u32 tmp=0;
  
  //hardResetTarget();
  USB_Request[USB_RequestOut][0]=2;
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  // System_Delay_ms(50);
  
  tmp=18000000;
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Clock;
  
  USB_Request[USB_RequestOut][1]=tmp;
  USB_Request[USB_RequestOut][2]=tmp>>8;
  USB_Request[USB_RequestOut][3]=tmp>>16;
  USB_Request[USB_RequestOut][4]=tmp>>24;
  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_TransferConfigure;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=200;
  USB_Request[USB_RequestOut][3]=1;
  USB_Request[USB_RequestOut][4]=50;
  USB_Request[USB_RequestOut][5]=0;
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWD_Configure;
  USB_Request[USB_RequestOut][1]=0;
  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=16;
  USB_Request[USB_RequestOut][2]=0x9E;
  USB_Request[USB_RequestOut][3]=0xE7; 
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=51;
  USB_Request[USB_RequestOut][2]=0xFF;
  USB_Request[USB_RequestOut][3]=0xFF;
  USB_Request[USB_RequestOut][4]=0xFF;
  USB_Request[USB_RequestOut][5]=0xFF;
  USB_Request[USB_RequestOut][6]=0xFF;
  USB_Request[USB_RequestOut][7]=0xFF;
  USB_Request[USB_RequestOut][8]=0xFF;  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_SWJ_Sequence;
  USB_Request[USB_RequestOut][1]=8;
  USB_Request[USB_RequestOut][2]=0x00;
  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x02;//读DP 0x01BA01477
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=0x1E;
  USB_Request[USB_RequestOut][3]=0x00;
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00; 
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
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
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x06;//读DP-CTRL/STAT
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  
  
  
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
  //  USB_Request[USB_RequestOut][3]=0x0F;//读AHB-AP ID 14770011
  //  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //  //////////////////////////////////////////////////////////////////////////////
  //  u16 timeout=0;
  //  timeout=100;
  //  do
  //  {
  //    USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  //    USB_Request[USB_RequestOut][1]=0;
  //    USB_Request[USB_RequestOut][2]=1;
  //    USB_Request[USB_RequestOut][3]=0x0F;//读AHB-AP ID 14770011
  //    DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  //    WriteAbort_Select();
  //    System_Delay_ms(10);
  //    timeout--;
  //  }while((USB_Response[0][2]!=0x01)&&timeout);
  //  if(timeout==0)
  //  {
  //    return 1;
  //  }
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT [7-4]=0000   选中AHB-AP BANK0  
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00;
  USB_Request[USB_RequestOut][6]=0x00;
  USB_Request[USB_RequestOut][7]=0x00;  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
  
  /********************数据通信位宽设置为32bit位宽*************************/  
  Write_AP_CSW(pswd, AP_CSW_DEFAULT);
  
  
  /********************Halts the target CPU*************************/  
  haltTarget(pswd);
  
  
  WriteAbort_Select(pswd);
  
  return 0;
}



void WriteAbort_Select(SWD_HANDLE *pswd)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
  
  USB_Request[USB_RequestOut][0]=ID_DAP_WriteABORT;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=0x1E;
  USB_Request[USB_RequestOut][3]=0x00;
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00; 
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);  
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x08;//写DP SELECT 0
  USB_Request[USB_RequestOut][4]=0x00;
  USB_Request[USB_RequestOut][5]=0x00;
  USB_Request[USB_RequestOut][6]=0x00;
  USB_Request[USB_RequestOut][7]=0x00;  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
}


/**********************************************************
* Resets the target CPU by using the AIRCR register. 
* The target will be halted immediately when coming
* out of reset. Does not reset the debug interface.
**********************************************************/
#define DEBUG_EVENT_TIMEOUT 200

u8 resetAndHaltTarget(SWD_HANDLE *pswd)
{
  uint32_t dhcsr;
  int timeout = DEBUG_EVENT_TIMEOUT;
  
  /* Halt target first. This is necessary before settingthe VECTRESET bit */
  haltTarget(pswd);
  /* Set halt-on-reset bit */
  writeMem(pswd, (uint32_t)&(CoreDebug->DEMCR),CoreDebug_DEMCR_VC_CORERESET_Msk);
  /* Clear exception state and reset target */
  writeMem(pswd, (uint32_t)&(SCB->AIRCR),0x05FA0004);
  
  // hardResetTarget_1();
  
  /* Wait for target to reset */
  do { 
//    System_Delay_ms(10);
delay_ms(10);
    timeout--;
    dhcsr = readMem(pswd, (uint32_t)&(CoreDebug->DHCSR));
    if(global_error_flag!=0)
    {
      return 1;
    }
  } while ( dhcsr & CoreDebug_DHCSR_S_RESET_ST_Msk );
  
  
  
  
  /* Check if we timed out */
  dhcsr = readMem(pswd, (uint32_t)&(CoreDebug->DHCSR));
  if ( dhcsr & CoreDebug_DHCSR_S_RESET_ST_Msk ) 
  {
    // RAISE(SWD_ERROR_TIMEOUT_WAITING_RESET);
  }
  
  /* Verify that target is halted */
  if ( !(dhcsr & CoreDebug_DHCSR_S_HALT_Msk) ) 
  {
    // RAISE(SWD_ERROR_TARGET_NOT_HALTED);
  }
  
  if(global_error_flag!=0)
  {
    return 2;
  }
  
  return 0;
}


u8 resetAndHaltTarget_20180327(SWD_HANDLE * pswd)
{
  uint32_t dhcrState;
  u16 timeout = 50;
  
  writeMem(pswd, (uint32_t)&(CoreDebug->DHCSR), 0xA05F0003);
  
  
  do {
//    System_Delay_ms(1);
delay_ms(1);    

    timeout--;
    dhcrState = readMem(pswd, (uint32_t)&(CoreDebug->DHCSR));
    if(global_error_flag!=0)
    {
      return 1;
    }
  } while ( !(dhcrState & CoreDebug_DHCSR_S_HALT_Msk) && timeout);
  
  if(timeout==0)
  {
    //   return 2;
  }
  
  
  /* Set halt-on-reset bit */
  writeMem(pswd, (uint32_t)&(CoreDebug->DEMCR),CoreDebug_DEMCR_VC_CORERESET_Msk);
  
  
  
  timeout = 5;
  /* Wait for target to reset */
  do { 
//    System_Delay_ms(1);
  delay_ms(1);  

    timeout--;
    dhcrState = readMem(pswd, (uint32_t)&(CoreDebug->DHCSR));
    if(global_error_flag!=0)
    {
      return 3;
    }
  } while ( (dhcrState & CoreDebug_DHCSR_S_RESET_ST_Msk)&&timeout);
  
  if(timeout==0)
  {
    // return 4;
  }
  hardResetTarget_1(pswd);
  
  
  timeout = 1000;
  do {
//    System_Delay_ms(1);
delay_ms(1);    

    timeout--;
    dhcrState = readMem(pswd, (uint32_t)&(CoreDebug->DHCSR));
    if(global_error_flag!=0)
    {
      return 5;
    }
  } while ( !(dhcrState & CoreDebug_DHCSR_S_HALT_Msk) && timeout);
  
  
  if(timeout==0)
  {
    return 6;
  }
  
  
  return 0;
}


/**********************************************************
* Resets the target CPU by using the AIRCR register. 
* Does not reset the debug interface
**********************************************************/
void resetTarget(SWD_HANDLE *pswd)
{
  /* Clear the VC_CORERESET bit */
  writeMem(pswd, (uint32_t)&(CoreDebug->DEMCR), 0);
  
  /* Do a dummy read of sticky bit to make sure it is cleared */
  readMem(pswd, (uint32_t)&(CoreDebug->DHCSR));
  readMem(pswd, (uint32_t)&(CoreDebug->DHCSR));
  
  /* Reset CPU */
  writeMem(pswd, (uint32_t)&(SCB->AIRCR), 0x05FA0006);
}

/**********************************************************
* Performs a pin reset on the target
**********************************************************/
void hardResetTarget(SWD_HANDLE * pswd)
{
  PIN_nRESET_LOW(pswd);
}


void hardResetTarget_1(SWD_HANDLE * pswd)
{
  // PIN_nRESET_LOW();
  
  // System_Delay_ms(100);
  
  PIN_nRESET_HIGH(pswd);
}
/**********************************************************
* Reads one word from internal memory
* 
* @param addr 
*    The address to read from
* 
* @returns 
*    The value at @param addr
**********************************************************/


uint32_t readMem(SWD_HANDLE *pswd, uint32_t addr)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
  u32 ret;
  /***********读RAM起始的3个地址***********/  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x05;//写AHB-AP Transfer Address TAR
  USB_Request[USB_RequestOut][4]=addr;
  USB_Request[USB_RequestOut][5]=addr>>8;
  USB_Request[USB_RequestOut][6]=addr>>16;
  USB_Request[USB_RequestOut][7]=addr>>24;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x0F;//读AHB-AP Data Read/Write DRW
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
  
  ret=USB_Response[0][6]<<24|USB_Response[0][5]<<16|USB_Response[0][4]<<8|USB_Response[0][3];
  //  if(((USB_Response[0][2])!=0x01)&&((USB_Response[0][2])&0x02)!=0x02)
  //  {
  //    global_error_flag=1;
  //  }
  //  else
  //  {
  //      global_error_flag=0;
  //  }
  if(((USB_Response[0][2])&0x0C)!=0x00)
  {
    global_error_flag=1;
  }
  else
  {
    global_error_flag=0;
  }
  return ret;
}

void writeMem(SWD_HANDLE *pswd, u32 address,u32 data)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x05;//写AHB-AP Transfer Address TAR
  USB_Request[USB_RequestOut][4]=address;
  USB_Request[USB_RequestOut][5]=address>>8;
  USB_Request[USB_RequestOut][6]=address>>16;
  USB_Request[USB_RequestOut][7]=address>>24;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x0D;//写AHB-AP Data Read/Write DRW
  USB_Request[USB_RequestOut][4]=data;
  USB_Request[USB_RequestOut][5]=data>>8;
  USB_Request[USB_RequestOut][6]=data>>16;
  USB_Request[USB_RequestOut][7]=data>>24; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
}
/**********************************************************
* Halts the target CPU
**********************************************************/
/* Number of times to retry reading status registers while
* waiting for a debug event (such as a halt of soft reset) */
void haltTarget(SWD_HANDLE *pswd)
{
  int timeout = DEBUG_EVENT_TIMEOUT;
  
  uint32_t dhcrState;
  
  writeMem(pswd, (uint32_t)&(CoreDebug->DHCSR), 0xA05F0003);
  
  
  do {
    dhcrState = readMem(pswd, (uint32_t)&CoreDebug->DHCSR);
    timeout--;
  } while ( !(dhcrState & CoreDebug_DHCSR_S_HALT_Msk) && timeout > 0 );
  
  if ( !(dhcrState & CoreDebug_DHCSR_S_HALT_Msk) ) {
    //RAISE(SWD_ERROR_TIMEOUT_HALT);
  }
}


/**********************************************************
* Lets the target CPU run freely (stops halting)
**********************************************************/
void runTarget(SWD_HANDLE *pswd)
{
  writeMem(pswd, (uint32_t)&(CoreDebug->DHCSR), 0xA05F0001);
}


/**********************************************************
* Writes a value to a CPU register in the target.
* 
* @param reg
*   The register number to write to
* 
* @param value
*   The value to write to the register
**********************************************************/
u8 writeCpuReg(SWD_HANDLE *pswd, int reg, uint32_t value)
{
  //  /* Wait until debug register is ready to accept new data */
  if(waitForRegReady(pswd)!=0)
  {
    return 1;
  }
  writeMem(pswd, (uint32_t)&(CoreDebug->DCRDR), value);
  writeMem(pswd, (uint32_t)&(CoreDebug->DCRSR), 0x10000 | (int)reg);
  
  return 0;
}


/**********************************************************
* Waits for the REGRDY bit in DCRSR. This bit indicates
* that the DCRDR/DCRSR registers are ready to accept
* new data. 
**********************************************************/
u8 waitForRegReady(SWD_HANDLE *pswd)
{
  u32 timeout=0;
  uint32_t dhcsr;
  timeout=1000;
  do {
    dhcsr = readMem(pswd, (uint32_t)&CoreDebug->DHCSR);
 //   System_Delay_ms(1);
delay_ms(1);  

    timeout--;
    if(global_error_flag!=0)
    {
      return 1;
    }
  } while ( (!(dhcsr & CoreDebug_DHCSR_S_REGRDY_Msk))&&timeout);
  
  if(timeout==0)
  {
    return 2;
  }
  return 0;
}


void Write_AP_CSW(SWD_HANDLE *pswd, u32 data)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
    
  /********************写AHB-AP Control and Status Word*************************/  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x01;
  USB_Request[USB_RequestOut][4]=data;
  USB_Request[USB_RequestOut][5]=data>>8;
  USB_Request[USB_RequestOut][6]=data>>16;
  USB_Request[USB_RequestOut][7]=data>>24;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
}


//u32 Read_AP_CSW(void)
//{
//  /********************写AHB-AP Control and Status Word*************************/  
//  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
//  USB_Request[USB_RequestOut][1]=0;
//  USB_Request[USB_RequestOut][2]=1;
//  USB_Request[USB_RequestOut][3]=0x03;
//  DAP_ProcessCommand(USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]);
//  return USB_Response[0][6]<<24|USB_Response[0][5]<<16|USB_Response[0][4]<<8|USB_Response[0][3];
//}


void WriteDP_AP_Address_Data(SWD_HANDLE *pswd, u32 address,u32 data)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
    
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x05;//写AHB-AP Transfer Address TAR
  USB_Request[USB_RequestOut][4]=address;
  USB_Request[USB_RequestOut][5]=address>>8;
  USB_Request[USB_RequestOut][6]=address>>16;
  USB_Request[USB_RequestOut][7]=address>>24;  
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
  
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x0D;//写AHB-AP Data Read/Write DRW
  USB_Request[USB_RequestOut][4]=data;
  USB_Request[USB_RequestOut][5]=data>>8;
  USB_Request[USB_RequestOut][6]=data>>16;
  USB_Request[USB_RequestOut][7]=data>>24; 
  DAP_ProcessCommand(pswd,USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
}


/**********************************************************
* Writes to one of the four AP registers in the currently
* selected AP bank.
* 
* @param reg[in]
*    The register number [0-3] to write to
* 
* @param data[in]
*    Value to write to the register
* 
**********************************************************/
void writeAP_TAR(SWD_HANDLE *pswd, uint32_t address)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
    
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x05;//写AHB-AP Transfer Address TAR
  USB_Request[USB_RequestOut][4]=address;
  USB_Request[USB_RequestOut][5]=address>>8;
  USB_Request[USB_RequestOut][6]=address>>16;
  USB_Request[USB_RequestOut][7]=address>>24;  
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
}


void writeAP_data(SWD_HANDLE *pswd, uint32_t data)
{
  u32 USB_RequestOut=0;        // Request  Buffer Out Index
  u32 USB_ResponseIn=0;        // Response Buffer In  Index
    
  
  USB_Request[USB_RequestOut][0]=ID_DAP_Transfer;
  USB_Request[USB_RequestOut][1]=0;
  USB_Request[USB_RequestOut][2]=1;
  USB_Request[USB_RequestOut][3]=0x0D;//写AHB-AP Data Read/Write DRW
  USB_Request[USB_RequestOut][4]=data;
  USB_Request[USB_RequestOut][5]=data>>8;
  USB_Request[USB_RequestOut][6]=data>>16;
  USB_Request[USB_RequestOut][7]=data>>24; 
  DAP_ProcessCommand(pswd, USB_Request[USB_RequestOut], USB_Response[USB_ResponseIn]); 
}


//void writeAP_data_block(uint32_t data,u32 * numWordsCount)
//{
//  uint32_t  request_value;
//  uint32_t  response_value;
//  uint32_t  retry;
//  
//  request_value=0x0D;//写AHB-AP Data Read/Write DRW
//  
//  retry = DAP_Data.transfer.retry_count;
//  
//  
//  do
//  {
//    response_value = SWD_Transfer(request_value, &data);
//  } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
//  if (response_value != DAP_TRANSFER_OK) goto end;
//  
//  if(*numWordsCount==1)
//  {
//    // Check last write
//    retry = DAP_Data.transfer.retry_count;
//    do
//    {
//      response_value = SWD_Transfer(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
//    } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
//  }
//  
//  
//  
//end:
//  ;
//  
//}



void writeAP_data_block(SWD_HANDLE *pswd, uint32_t data,u32 * numWordsCount)
{
  uint32_t  request_value;
  uint32_t  response_value;
  uint32_t  retry;
  
  request_value=0x0D;//写AHB-AP Data Read/Write DRW
  
  retry = DAP_Data.transfer.retry_count;
  
  
  do
  {
    response_value = SWD_Transfer(pswd, request_value, &data);
  } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
  if (response_value != DAP_TRANSFER_OK) goto end;
  
  if(*numWordsCount==1)
  {
    // Check last write
    retry = DAP_Data.transfer.retry_count;
    do
    {
      response_value = SWD_Transfer(pswd, DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
    } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
  }
  
  
  
end:
  ;
  
}



void writeAP_data_block_fast(uint32_t data,u32 * numWordsCount)
{
 
//  uint32_t  response_value;
//  uint32_t  retry;
//  retry = DAP_Data.transfer.retry_count;
//  do
//  {
//    response_value = SWD_Transfer_fast1(&data);
//  } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
//  if (response_value != DAP_TRANSFER_OK) goto end;
//  
//  if(*numWordsCount==1)
//  {
//    // Check last write
//    retry = DAP_Data.transfer.retry_count;
//    do
//    {
//      response_value = SWD_Transfer(DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
//    } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
//  }
//  
//  
//  
//end:
//  ;
  
}





void readAP_data_block(SWD_HANDLE * pswd, uint32_t *data)
{
  uint32_t  request_value;
  uint32_t  response_value;
  uint32_t  retry;
  
  request_value=0x0F;//读AHB-AP Data Read/Write DRW
  
  retry = DAP_Data.transfer.retry_count;
  
  
  do
  {
    response_value = SWD_Transfer(pswd, request_value, data);
  } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
  if (response_value != DAP_TRANSFER_OK) goto end;
  
//  if(*numWordsCount==1)
//  {
//    // Check last write
//    retry = DAP_Data.transfer.retry_count;
//    do
//    {
//      response_value = SWD_Transfer(pswd, DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
//    } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
//  }
//  
  
  
end:
  ;
  
}





#include"ff.h"
static FIL FilSys;
static u8 FileData[4096];
extern u16 ImageNumber_store;
static char sp1[100];
u32 verifyFirmware(SWD_HANDLE * pswd)
{
  u32 blk=0;
  u32 aes_key=3000;
  u32 ReadCount=0;
  u8 *Point=FileData;
  uint32_t tarWrap = 0x0000007F;
  uint32_t numWords = 0;
  uint32_t i = 0;
  uint32_t j = 0;
  uint32_t k = 0;
  uint32_t addr;
  uint32_t value=0;
  uint32_t verifyAddress = 0;
  verifyAddress = (u32)cJSON_GetObjectItem(pswd->root, "flash_base")->valuedouble;
  if(verifyAddress==8000000)//兼容老版本电脑软件
  {
    verifyAddress=0x8000000;
  }
  
  sprintf(sp1,"0:/system/dat/DAT%03u.BIN",pswd->ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  
  
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 1;
  }
  
  
  
  if( (u8)cJSON_GetObjectItem(pswd->root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(pswd->root, "aes_key"),5)->valuedouble;
    
    AES_Decrypt_Config_Init(16,aes_key);
    
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
    
    Point=FileData;
  }
  
  Write_AP_CSW(pswd, AP_CSW_DEFAULT | AP_CSW_AUTO_INCREMENT);
  addr=verifyAddress;//校验起始地址
  while(ReadCount)
  {
    Point=FileData;
    j=0;
    //N76E003_aprom_read_multi_byte_continue(ReadBuff,ReadCount); 
    numWords = ReadCount/ 4;
    writeAP_TAR(pswd, addr);
    readAP_data_block(pswd, &value);
    for ( i=0; i<numWords; i++ ) 
    {
      /* TAR must be initialized at every TAR wrap boundary
      * because the autoincrement wraps around at these */
      if ( (addr & tarWrap) == 0 )
      {
        //writeAP(AP_TAR, addr);
        writeAP_TAR(pswd, addr);
        /* Do one dummy read. Subsequent reads will return the 
        * correct result. */
        // readAP(AP_DRW, &value);
        readAP_data_block(pswd, &value);
      }
      
      /* Read the value from addr */
      //readAP(AP_DRW, &value);
      readAP_data_block(pswd, &value);
      if((*(u32 *)(Point+j))!=value)
      {
        for(k=0;k<4;k++)
        {
          if((*(u8 *)(Point+j+k))!=(u8)value)
          {
            return addr;
          }
          value=value>>8;
          addr+=1;
        }
        
       // return addr;
      }
      /* Get current address */
      addr += 4;
      j+=4;
    }
    
    if(ReadCount%4)//有不满4个字节的字节存在
    {
            /* TAR must be initialized at every TAR wrap boundary
      * because the autoincrement wraps around at these */
      if ( (addr & tarWrap) == 0 )
      {
        //writeAP(AP_TAR, addr);
        writeAP_TAR(pswd, addr);
        /* Do one dummy read. Subsequent reads will return the 
        * correct result. */
        // readAP(AP_DRW, &value);
        readAP_data_block(pswd, &value);
      }
      

      readAP_data_block(pswd, &value);
      for(i=0;i<ReadCount%4;i++)
      {
        if((*(u8 *)(Point+j+i))!=(u8)value)
        {
          return addr;
        }
        value=value>>8;
        addr+=1;
      }
      
    }
    
    
    
    ///////////////////////////////////////////////开始校验FLASH     
    
//    for(u16 i=0;i<ReadCount;i++)
//    {
//      if(*(Point+i)!=*(ReadBuff+i))
//      {
//        return 2;
//      }
//    }
    ///////////////////////////////////////////校验结束    
    
  //  blk+=ReadCount;
    
    
    if(ReadCount>=1024)
    {
      if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
      {
        return 9;
      }
      if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
      {
        return 10;
      }
      
      
      if( (u8)cJSON_GetObjectItem(pswd->root, "data_encrypt")->valueint==1)//AES加密
      {
        Point = FileData;
        
        
        for (u32 j = 0; j < ReadCount; j += 16)
        {
          //解密数据包
          AES_Decrypt_Calculate(Point);
          Point += 16;
        }
      }
    }
    else
    {
      ReadCount=0;//强制退出循环，
      
    }
  }
  Write_AP_CSW(pswd, AP_CSW_DEFAULT);  
  f_close(&FilSys);
  
  
  return 0;
}
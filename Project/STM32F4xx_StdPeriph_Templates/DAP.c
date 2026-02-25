/******************************************************************************
* @file     DAP.c
* @brief    CMSIS-DAP Commands
* @version  V1.00
* @date     31. May 2012
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

#include <string.h>
#include "DAP_config.h"
#include "DAP.h"


#define DAP_FW_VER      "10.21"   // Firmware Version


#if (DAP_PACKET_SIZE < 64)
#error "Minimum Packet Size is 64"
#endif
#if (DAP_PACKET_SIZE > 32768)
#error "Maximum Packet Size is 32768"
#endif
#if (DAP_PACKET_COUNT < 1)
#error "Minimum Packet Count is 1"
#endif
#if (DAP_PACKET_COUNT > 255)
#error "Maximum Packet Count is 255"
#endif


// Clock Macros

#define MAX_SWJ_CLOCK(delay_cycles)	( CPU_CLOCK / 2 / (IO_PORT_WRITE_CYCLES + delay_cycles))
#define CLOCK_DELAY(swj_clock)		((CPU_CLOCK / 2 / swj_clock) - IO_PORT_WRITE_CYCLES)


DAP_Data_t DAP_Data;           // DAP Data
volatile uint8_t    DAP_TransferAbort;  // Trasfer Abort Flag


#ifdef DAP_VENDOR
const char DAP_Vendor [] = DAP_VENDOR;
#endif

#ifdef DAP_PRODUCT
const char DAP_Product[] = DAP_PRODUCT;
#endif

#ifdef DAP_SER_NUM
const char DAP_SerNum [] = DAP_SER_NUM;
#endif

const char DAP_FW_Ver [] = DAP_FW_VER;

#if TARGET_DEVICE_FIXED
const char TargetDeviceVendor [] = TARGET_DEVICE_VENDOR;
const char TargetDeviceName   [] = TARGET_DEVICE_NAME;
#endif


// Get DAP Information
//   id:      info identifier
//   info:    pointer to info data
//   return:  number of bytes in info data
static uint8_t DAP_Info(uint8_t id, uint8_t *info)
{
  uint8_t length = 0;
  
  DEBUG("函数名字DAP_Info 形参: uint8_t id=%02X\n", id);
  
  switch (id)
  {
  case DAP_ID_VENDOR:
#ifdef DAP_VENDOR
    memcpy(info, DAP_Vendor, sizeof(DAP_Vendor));
    length = sizeof(DAP_Vendor);
#endif
    break;
  case DAP_ID_PRODUCT:
#ifdef DAP_PRODUCT
    memcpy(info, DAP_Product, sizeof(DAP_Product));
    length = sizeof(DAP_Product);
#endif
    break;
  case DAP_ID_SER_NUM:
#ifdef DAP_SER_NUM
    memcpy(info, DAP_SerNum, sizeof(DAP_SerNum));
    length = sizeof(DAP_SerNum);
#endif
    break;
  case DAP_ID_FW_VER:
    memcpy(info, DAP_FW_Ver, sizeof(DAP_FW_Ver));
    length = sizeof(DAP_FW_Ver);
    break;
  case DAP_ID_DEVICE_VENDOR:
#if TARGET_DEVICE_FIXED
    memcpy(info, TargetDeviceVendor, sizeof(TargetDeviceVendor));
    length = sizeof(DAP_Target_Device);
#endif
    break;
  case DAP_ID_DEVICE_NAME:
#if TARGET_DEVICE_FIXED
    memcpy(info, TargetDeviceName, sizeof(TargetDeviceName));
    length = sizeof(DAP_Target_Device);
#endif
    break;
  case DAP_ID_CAPABILITIES:
    info[0] =	((DAP_SWD  != 0) ? (1 << 0) : 0) |
      ((DAP_JTAG != 0) ? (1 << 1) : 0);
    length = 1;
    break;
  case DAP_ID_PACKET_SIZE:
    info[0] = (uint8_t)(DAP_PACKET_SIZE >> 0);
    info[1] = (uint8_t)(DAP_PACKET_SIZE >> 8);
    length = 2;
    break;
  case DAP_ID_PACKET_COUNT:
    info[0] = DAP_PACKET_COUNT;
    length = 1;
    break;
  }
  
  return (length);
}


// Timer Functions

#if ((DAP_SWD != 0) || (DAP_JTAG != 0))

// Start Timer
static void TIMER_START (uint32_t usec)
{
  SysTick->VAL  = 0;
  SysTick->LOAD = usec * CPU_CLOCK / 1000000;
  SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
}

// Stop Timer
static void TIMER_STOP (void)
{
  SysTick->CTRL = 0;
}

// Check if Timer expired
static uint32_t TIMER_EXPIRED (void)
{
  return ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) ? 1 : 0);
}

#endif


// Delay for specified time
//    delay:  delay time in ms
void Delayms(uint32_t delay)
{
  delay *= (CPU_CLOCK / 1000 + (DELAY_SLOW_CYCLES-1)) / DELAY_SLOW_CYCLES;
  PIN_DELAY_SLOW(delay);
}


// Process Delay command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
static uint32_t DAP_Delay(uint8_t *request, uint8_t *response) {
  uint32_t delay;
  
  delay  = *(request + 0) | (*(request + 1) << 8);
  delay *= (CPU_CLOCK / 1000000 + (DELAY_SLOW_CYCLES-1)) / DELAY_SLOW_CYCLES;
  
  PIN_DELAY_SLOW(delay);
  
  *response = DAP_OK;
  return (1);
}




// Process Connect command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
static uint32_t DAP_Connect(SWD_HANDLE *pswd, uint8_t *request, uint8_t *response)
{
  uint32_t port;
  
  if (*request == DAP_PORT_AUTODETECT)
  {
    port = DAP_DEFAULT_PORT;
  }
  else
  {
    port = *request;
  }
  
  switch (port)
  {
  case DAP_PORT_SWD:
    DAP_Data.debug_port = DAP_PORT_SWD;
    PORT_SWD_SETUP(pswd);
    break;
  }
  
  *response = port;
  return (1);
}


// Process Disconnect command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
static uint32_t DAP_Disconnect(SWD_HANDLE *pswd, uint8_t *response)
{
  DAP_Data.debug_port = DAP_PORT_DISABLED;
  PORT_OFF(pswd);
  
  *response = DAP_OK;
  return (1);
}


// Process Reset Target command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
static uint32_t DAP_ResetTarget(uint8_t *response)
{
  *(response + 1) = RESET_TARGET();
  *(response + 0) = DAP_OK;
  DEBUG("DAP_RESET: %02X\n", *(response + 1));
  return (2);
}


// Process SWJ Pins command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
static uint32_t DAP_SWJ_Pins(uint8_t *request, uint8_t *response)
{
//  uint8_t value;
//  uint8_t select;
//  uint32_t wait;
//  
//  value	= 	 *(request + 0);
//  select	=	 *(request + 1);
//  wait	=	(*(request + 2) <<  0) |
//    (*(request + 3) <<  8) |
//      (*(request + 4) << 16) |
//        (*(request + 5) << 24);
//  
//  DEBUG("DAP_SWJ_Pins: 选择%04X 数值%04X 等待%04X", select, value, wait);
//  
//  if (select & (1 << DAP_SWJ_SWCLK_TCK))
//  {
//    if (value & (1 << DAP_SWJ_SWCLK_TCK))
//      PIN_SWCLK_TCK_SET();
//    else
//      PIN_SWCLK_TCK_CLR();
//  }
//  
//  if (select & (1 << DAP_SWJ_SWDIO_TMS))
//  {
//    if (value & (1 << DAP_SWJ_SWDIO_TMS))
//      PIN_SWDIO_TMS_SET();
//    else
//      PIN_SWDIO_TMS_CLR();
//  }
//  
//  
//  if (select & (1 << DAP_SWJ_nTRST))
//    PIN_nTRST_OUT(value >> DAP_SWJ_nTRST);
//  
//  if (select & (1 << DAP_SWJ_nRESET))
//    PIN_nRESET_OUT(value >> DAP_SWJ_nRESET);
//  
//  if (wait)
//  {
//    if (wait > 3000000)
//      wait = 3000000;
//    TIMER_START(wait);
//    do {
//      if (select & (1 << DAP_SWJ_SWCLK_TCK))
//      {
//        if ((value >> DAP_SWJ_SWCLK_TCK) ^ PIN_SWCLK_TCK_IN())
//          continue;
//      }
//      if (select & (1 << DAP_SWJ_SWDIO_TMS))
//      {
//        if ((value >> DAP_SWJ_SWDIO_TMS) ^ PIN_SWDIO_TMS_IN())
//          continue;
//      }
//      if (select & (1 << DAP_SWJ_nTRST))
//      {
//        if ((value >> DAP_SWJ_nTRST) ^ PIN_nTRST_IN())
//          continue;
//      }
//      if (select & (1 << DAP_SWJ_nRESET))
//      {
//        if ((value >> DAP_SWJ_nRESET) ^ PIN_nRESET_IN())
//          continue;
//      }
//      break;
//    } while (!TIMER_EXPIRED());
//    TIMER_STOP();
//  }
//  
//  value = (PIN_SWCLK_TCK_IN() << DAP_SWJ_SWCLK_TCK) |(PIN_SWDIO_TMS_IN() << DAP_SWJ_SWDIO_TMS) |(PIN_nTRST_IN()<< DAP_SWJ_nTRST)|(PIN_nRESET_IN()<< DAP_SWJ_nRESET);
//  *response = (uint8_t)value;
//  return (1);
}



// Process SWJ Clock command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
static uint32_t DAP_SWJ_Clock(uint8_t *request, uint8_t *response)
{
  uint32_t clock;
  uint32_t delay;
  
  clock = (*(request + 0) <<  0) |
    (*(request + 1) <<  8) |
      (*(request + 2) << 16) |
        (*(request + 3) << 24);
  
  
  if (clock == 0)
  {
    *response = DAP_ERROR;
    return (1);
  }
  
  if (clock >= MAX_SWJ_CLOCK(DELAY_FAST_CYCLES))
  {
    DAP_Data.fast_clock  = 1;
    DAP_Data.clock_delay = 1;
  }
  else
  {
    DAP_Data.fast_clock  = 0;
    
    delay = (CPU_CLOCK / 2 + (clock - 1)) / clock;
    if (delay > IO_PORT_WRITE_CYCLES)
    {
      delay -= IO_PORT_WRITE_CYCLES;
      delay  = (delay + (DELAY_SLOW_CYCLES - 1)) / DELAY_SLOW_CYCLES;
    }
    else
    {
      delay  = 1;
    }
    
    DAP_Data.clock_delay = delay;
  }
  
  *response = DAP_OK;
  return (1);
}
#endif


// Process SWJ Sequence command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
#if ((DAP_SWD != 0) || (DAP_JTAG != 0))
static uint32_t DAP_SWJ_Sequence(SWD_HANDLE *pswd, uint8_t *request, uint8_t *response)
{
  uint32_t count;
  
  count = *request++;
  if (count == 0)
    count = 256;
  
  
  SWJ_Sequence(pswd, count, request);
  
  *response = DAP_OK;
  return (1);
}
#endif


// Process SWD Configure command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
#if (DAP_SWD != 0)
static uint32_t DAP_SWD_Configure(uint8_t *request, uint8_t *response)
{
  uint8_t value;
  
  value = *request;
  DAP_Data.swd_conf.turnaround  = (value & 0x03) + 1;
  DAP_Data.swd_conf.data_phase  = (value & 0x04) ? 1 : 0;
  
  
  *response = DAP_OK;
  return (1);
}
#endif


// Process SWD Abort command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
#if (DAP_SWD != 0)
static uint32_t DAP_SWD_Abort(SWD_HANDLE *pswd, uint8_t *request, uint8_t *response)
{
  uint32_t data;
  
  
  
  if (DAP_Data.debug_port != DAP_PORT_SWD)
  {
    *response = DAP_ERROR;
    return (1);
  }
  
  // Load data (Ignore DAP index)
  data =	(*(request+1) <<  0) |
    (*(request+2) <<  8) |
      (*(request+3) << 16) |
        (*(request+4) << 24);
  
  
  // Write Abort register
  SWD_Transfer(pswd, DP_ABORT, &data);
  *response = DAP_OK;
  
  return (1);
}
#endif





// Process Transfer Configure command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
static uint32_t DAP_TransferConfigure(uint8_t *request, uint8_t *response)
{
  DAP_Data.transfer.idle_cycles = *(request + 0);
  DAP_Data.transfer.retry_count = *(request + 1) | (*(request + 2) << 8);
  DAP_Data.transfer.match_retry = *(request + 3) | (*(request + 4) << 8);
  
  *response = DAP_OK;
  return (1);
}


// Process SWD Transfer command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
#if (DAP_SWD != 0)
static uint32_t DAP_SWD_Transfer(SWD_HANDLE *pswd, uint8_t *request, uint8_t *response)
{
  uint8_t  request_count;
  uint8_t  request_value;
  uint8_t  response_count;
  uint8_t  response_value;
  uint8_t  *response_head;
  uint32_t  post_read;
  uint32_t  check_write;
  uint32_t  match_value;
  uint16_t  match_retry;
  uint16_t  retry;
  uint32_t  data;
  
  response_count = 0;
  response_value = 0;
  response_head  = response;
  response      += 2;
  
  DAP_TransferAbort = 0;
  
  post_read   = 0;
  check_write = 0;
  
  request++;            // Ignore DAP index
  
  request_count = *request++;
  
  while (request_count--)
  {
    DEBUG("*****************request_count= %d\n", request_count);
    request_value = *request++;
    
    if (request_value & DAP_TRANSFER_RnW)
    {
      // Read register
      if (post_read)//过去的一次读？
      {
        // Read was posted before
        retry = DAP_Data.transfer.retry_count;
        if ((request_value & (DAP_TRANSFER_APnDP | DAP_TRANSFER_MATCH_VALUE)) == DAP_TRANSFER_APnDP)
        {
          // Read previous AP data and post next AP read
          do
          {
            response_value = SWD_Transfer(pswd, request_value, &data);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
        }
        else
        {
          // Read previous AP data
          do
          {
            response_value = SWD_Transfer(pswd, DP_RDBUFF | DAP_TRANSFER_RnW, &data);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          post_read = 0;
        }
        if (response_value != DAP_TRANSFER_OK)
          break;
        // Store previous AP data
        *response++ = (uint8_t) data;
        *response++ = (uint8_t)(data >>  8);
        *response++ = (uint8_t)(data >> 16);
        *response++ = (uint8_t)(data >> 24);
      }
      if (request_value & DAP_TRANSFER_MATCH_VALUE)
      {
        // Read with value match
        match_value =	(*(request+0) <<  0) |
          (*(request+1) <<  8) |
            (*(request+2) << 16) |
              (*(request+3) << 24);
        request += 4;
        match_retry = DAP_Data.transfer.match_retry;
        if (request_value & DAP_TRANSFER_APnDP)
        {
          // Post AP read
          retry = DAP_Data.transfer.retry_count;
          do
          {
            response_value = SWD_Transfer(pswd, request_value, NULL);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          if (response_value != DAP_TRANSFER_OK) break;
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////////				
        do
        {
          // Read register until its value matches or retry counter expires
          retry = DAP_Data.transfer.retry_count;
          do
          {
            response_value = SWD_Transfer(pswd, request_value, &data);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          
          if (response_value != DAP_TRANSFER_OK)
            break;
        } while (((data & DAP_Data.transfer.match_mask) != match_value) && match_retry-- && !DAP_TransferAbort);
        if ((data & DAP_Data.transfer.match_mask) != match_value)
        {
          response_value |= DAP_TRANSFER_MISMATCH;
        }
        if (response_value != DAP_TRANSFER_OK) break;
      }
      else
      {
        // Normal read
        retry = DAP_Data.transfer.retry_count;
        if (request_value & DAP_TRANSFER_APnDP)
        {
          // Read AP register
          if (post_read == 0)
          {
            // Post AP read
            do
            {
              response_value = SWD_Transfer(pswd, request_value, NULL);
            } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
            if (response_value != DAP_TRANSFER_OK) break;
            post_read = 1;//一次读
          }
        }
        else
        {
          // Read DP register
          do
          {
            response_value = SWD_Transfer(pswd, request_value, &data);
          } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
          if (response_value != DAP_TRANSFER_OK) 
            break;
          // Store data
          *response++ = (uint8_t) data;
          *response++ = (uint8_t)(data >>  8);
          *response++ = (uint8_t)(data >> 16);
          *response++ = (uint8_t)(data >> 24);
        }
      }
      check_write = 0;
    }
    else
    {
      // Write register
      if (post_read)
      {
        // Read previous data
        retry = DAP_Data.transfer.retry_count;
        do
        {
          response_value = SWD_Transfer(pswd, DP_RDBUFF | DAP_TRANSFER_RnW, &data);
        } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
        
        if (response_value != DAP_TRANSFER_OK)
          break;
        // Store previous data
        *response++ = (uint8_t) data;
        *response++ = (uint8_t)(data >>  8);
        *response++ = (uint8_t)(data >> 16);
        *response++ = (uint8_t)(data >> 24);
        post_read = 0;
      }
      // Load data
      data =	(*(request+0) <<  0) |
        (*(request+1) <<  8) |
          (*(request+2) << 16) |
            (*(request+3) << 24);
      request += 4;
      if (request_value & DAP_TRANSFER_MATCH_MASK)
      {
        // Write match mask
        DAP_Data.transfer.match_mask = data;
        response_value = DAP_TRANSFER_OK;
      }
      else
      {
        // Write DP/AP register
        retry = DAP_Data.transfer.retry_count;
        do
        {
          response_value = SWD_Transfer(pswd, request_value, &data);
        } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
        
        if (response_value != DAP_TRANSFER_OK)
          break;
        check_write = 1;
      }
    }
    response_count++;
    if (DAP_TransferAbort)
      break;
  }
  
  if (response_value == DAP_TRANSFER_OK)
  {
    if (post_read)
    {
      // Read previous data
      retry = DAP_Data.transfer.retry_count;
      do
      {
        response_value = SWD_Transfer(pswd, DP_RDBUFF | DAP_TRANSFER_RnW, &data);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) goto end;
      // Store previous data
      *response++ = (uint8_t) data;
      *response++ = (uint8_t)(data >>  8);
      *response++ = (uint8_t)(data >> 16);
      *response++ = (uint8_t)(data >> 24);
    }
    else if (check_write)
    {
      // Check last write
      retry = DAP_Data.transfer.retry_count;
      do
      {
        response_value = SWD_Transfer(pswd, DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
    }
  }
  
end:
  *(response_head + 0) = (uint8_t)response_count;
  *(response_head + 1) = (uint8_t)response_value;
  
  return (response - response_head);
}
#endif




// Process SWD Transfer Block command and prepare response
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
#if (DAP_SWD != 0)
static uint32_t DAP_SWD_TransferBlock(SWD_HANDLE *pswd, uint8_t *request, uint8_t *response)
{
  uint32_t  request_count;
  uint32_t  request_value;
  uint32_t  response_count;
  uint32_t  response_value;
  uint8_t  *response_head;
  uint32_t  retry;
  uint32_t  data;
  
  DEBUG("oˉêy??×?DAP_SWD_TransferBlock:\n");
  
  response_count = 0;
  response_value = 0;
  response_head  = response;
  response      += 3;
  
  DAP_TransferAbort = 0;
  
  request++;            // Ignore DAP index
  
  request_count = *request | (*(request+1) << 8);
  request += 2;
  if (request_count == 0) goto end;
  
  request_value = *request++;
  if (request_value & DAP_TRANSFER_RnW)
  {
    // Read register block
    if (request_value & DAP_TRANSFER_APnDP)
    {
      // Post AP read
      retry = DAP_Data.transfer.retry_count;
      do
      {
        response_value = SWD_Transfer(pswd, request_value, NULL);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) goto end;
    }
    while (request_count--)
    {		
      // Read DP/AP register
      if ((request_count == 0) && (request_value & DAP_TRANSFER_APnDP))
      {
        // Last AP read
        request_value = DP_RDBUFF | DAP_TRANSFER_RnW;
      }
      retry = DAP_Data.transfer.retry_count;
      do
      {
        response_value = SWD_Transfer(pswd, request_value, &data);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) goto end;
      // Store data
      *response++ = (uint8_t) data;
      *response++ = (uint8_t)(data >>  8);
      *response++ = (uint8_t)(data >> 16);
      *response++ = (uint8_t)(data >> 24);
      response_count++;
    }
  }
  else
  {		
    // Write register block
    while (request_count--)
    {
      
      // Load data
      data =	(*(request+0) <<  0) |
        (*(request+1) <<  8) |
          (*(request+2) << 16) |
            (*(request+3) << 24);
      request += 4;
      // Write DP/AP register
      retry = DAP_Data.transfer.retry_count;
      
      
      do
      {
        response_value = SWD_Transfer(pswd, request_value, &data);
      } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
      if (response_value != DAP_TRANSFER_OK) goto end;
      response_count++;
    }
    // Check last write
    retry = DAP_Data.transfer.retry_count;
    do
    {
      response_value = SWD_Transfer(pswd, DP_RDBUFF | DAP_TRANSFER_RnW, NULL);
    } while ((response_value == DAP_TRANSFER_WAIT) && retry-- && !DAP_TransferAbort);
  }
  
end:
  *(response_head+0) = (uint8_t)(response_count >> 0);
  *(response_head+1) = (uint8_t)(response_count >> 8);
  *(response_head+2) = (uint8_t) response_value;
  
  return (response - response_head);
}
#endif




// Process DAP Vendor command and prepare response
// Default function (can be overridden)
//   request:  pointer to request data
//   response: pointer to response data
//   return:   number of bytes in response
__weak uint32_t DAP_ProcessVendorCommand(uint8_t *request, uint8_t *response)
{
  DEBUG("DAP_ProcessVendorCommand:\n");
  *response = ID_DAP_Invalid;
  return (1);
}




uint32_t DAP_ProcessCommand(SWD_HANDLE *pswd, uint8_t *request, uint8_t *response)
{
  uint32_t num;
  
  if ((*request >= ID_DAP_Vendor0) && (*request <= ID_DAP_Vendor31))
  {
    return DAP_ProcessVendorCommand(request, response);
  }
  
  *response++ = *request;
  
  switch (*request++)
  {
  case ID_DAP_Info:
    num = DAP_Info(*request, response + 1);
    *response = num;
    return (2 + num);
  case ID_DAP_LED:
    //  num = DAP_LED(request, response);
    break;
  case ID_DAP_Connect:
    num = DAP_Connect(pswd, request, response);
    break;
  case ID_DAP_Disconnect://断开SWD连接20241026
    num = DAP_Disconnect(pswd, response);
    break;
  case ID_DAP_Delay:
    num = DAP_Delay(request, response);
    break;
  case ID_DAP_ResetTarget:
    num = DAP_ResetTarget(response);
    break;
    
  case ID_DAP_SWJ_Pins:
    num = DAP_SWJ_Pins(request, response);
    break;
  case ID_DAP_SWJ_Clock:
    num = DAP_SWJ_Clock(request, response);
    break;
  case ID_DAP_SWJ_Sequence://发送SWD时序20241026
    num = DAP_SWJ_Sequence(pswd, request, response);
    break;
    
    
  case ID_DAP_SWD_Configure://双路参数一样无需设置？
    num = DAP_SWD_Configure(request, response);
    break;
    
    
    
  case ID_DAP_JTAG_Sequence:
  case ID_DAP_JTAG_Configure:
  case ID_DAP_JTAG_IDCODE:
    *response = DAP_ERROR;
    return (2);
    
  case ID_DAP_TransferConfigure://双路参数一样无需设置？
    num = DAP_TransferConfigure(request, response);
    break;
    
  case ID_DAP_Transfer://发送SWD数据20241026
    switch (DAP_Data.debug_port)
    {
    case DAP_PORT_SWD:
      num = DAP_SWD_Transfer (pswd, request, response);
      break;
      
    default:
      *(response+0) = 0;    // Response count
      *(response+1) = 0;    // Response value
      num = 2;
    }
    break;
    
  case ID_DAP_TransferBlock://没有使用到20241027
    switch (DAP_Data.debug_port)
    {
    case DAP_PORT_SWD:
      num = DAP_SWD_TransferBlock (pswd, request, response);
      break;
    default:
      *(response+0) = 0;    // Response count [7:0]
      *(response+1) = 0;    // Response count[15:8]
      *(response+2) = 0;    // Response value
      num = 3;
    }
    break;
    
  case ID_DAP_WriteABORT://发送退出命令20241027
    switch (DAP_Data.debug_port)
    {
#if (DAP_SWD != 0)
    case DAP_PORT_SWD:
      num = DAP_SWD_Abort (pswd, request, response);
      break;
#endif
    default:
      *response = DAP_ERROR;
      return (2);
    }
    break;
    
  default:
    *(response-1) = ID_DAP_Invalid;
    return (1);
  }
  return (1 + num);
}



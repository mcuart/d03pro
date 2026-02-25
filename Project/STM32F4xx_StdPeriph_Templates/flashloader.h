#ifndef FLASHLOADER_H
#define FLASHLOADER_H

#include <stdint.h>

#define STATE_LOCATION 0x20001000

/* This is the flashloader status, set by the flashloader itself. */
#define FLASHLOADER_STATUS_NOT_READY 0xA5A50000
#define FLASHLOADER_STATUS_READY     0xA5A50001

#define FLASHLOADER_STATUS_ERROR_HARDFAULT    0xA5A50064
#define FLASHLOADER_STATUS_ERROR_NMI          0xA5A50065

#define FLASHLOADER_STATUS_ERROR_TIMEOUT      0xA5A50066
#define FLASHLOADER_STATUS_ERROR_LOCKED       0xA5A50067
#define FLASHLOADER_STATUS_ERROR_INVALIDADDR  0xA5A50068
#define FLASHLOADER_STATUS_ERROR_UNALIGNED    0xA5A50069
#define FLASHLOADER_STATUS_ERROR_UNKNOWN      0xA5A5006A

#define DEBUGGERCMD_NOT_CONNECTED   0x5A5A0000
#define DEBUGGERCMD_NONE            0x5A5A0001
#define DEBUGGERCMD_ERASE_PAGE      0x5A5A0002
#define DEBUGGERCMD_WRITE_DATA1     0x5A5A0003
#define DEBUGGERCMD_WRITE_DATA2     0x5A5A0004
#define DEBUGGERCMD_MASS_ERASE      0x5A5A0005


#define DEBUGGERCMD_WRITE_OPTION_BYTE_DATA1     0x5A5A0006
#define DEBUGGERCMD_WRITE_OPTION_BYTE_DATA2     0x5A5A0007

typedef struct
{
  volatile uint32_t flashLoaderStatus;  /* Set by the flashloader */
  volatile uint32_t debuggerStatus;     /* Set by debugger. */ 
  volatile uint32_t bufferSize;         /* Size of available buffers */
  volatile uint32_t bufferAddress1;     /* Address of buffer 1 */
  volatile uint32_t bufferAddress2;     /* Address of buffer 2 */
  volatile uint32_t writeAddress1;       /* Start address to write */
  volatile uint32_t writeAddress2;       /* Start address to write */
  volatile uint32_t numBytes1;          /* Number of bytes to write to flash from buffer 1 */
  volatile uint32_t numBytes2;          /* Number of bytes to write to flash from buffer 2 */
  volatile uint32_t pageSize;           /* Size of one flash page */
  volatile uint32_t sramSize;           /* Size of SRAM */
  volatile uint32_t flashSize;          /* Size of internal flash */
  volatile uint32_t partFamily;         /* EFM32 Device Family Id */
  volatile uint32_t prodRev;            /* Production rev. */
  
} flashLoaderState_TypeDef;

#endif
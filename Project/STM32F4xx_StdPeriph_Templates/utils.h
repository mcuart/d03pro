#ifndef _UTILS_H_
#define _UTILS_H_

#include "DAP.h"
#include "DAP_config.h"
#include "main.h"


#include "swd_core.h"

/* Value to write to AIRCR in order to do a soft
 * reset of the target */
#define AIRCR_RESET_CMD 0x05FA0006




u8 EnterProgramMode(SWD_HANDLE *pswd);
void GPIO_Config(void);
void WriteAbort_Select(SWD_HANDLE *pswd);
void haltTarget(SWD_HANDLE *pswd);
u8 resetAndHaltTarget(SWD_HANDLE *pswd);
void hardResetTarget(SWD_HANDLE * pswd);
u32 readMem(SWD_HANDLE *pswd, uint32_t addr);
void writeMem(SWD_HANDLE *pswd, u32 address,u32 data);
void Write_AP_CSW(SWD_HANDLE *pswd, u32 data);
void writeAP_TAR(SWD_HANDLE *pswd, uint32_t address);
void writeAP_data(SWD_HANDLE *pswd, uint32_t data);
void writeAP_data_block(SWD_HANDLE *pswd, uint32_t data,u32 * numWordsCount);
void WriteDP_AP_Address_Data(SWD_HANDLE *pswd, u32 address,u32 data);
void resetTarget(SWD_HANDLE *pswd);
void runTarget(SWD_HANDLE *pswd);
u8 writeCpuReg(SWD_HANDLE *pswd, int reg, uint32_t value);
u8 resetAndHaltTarget_20180327(SWD_HANDLE *pswd);
#endif
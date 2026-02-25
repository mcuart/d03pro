/**************************************************************************//**
 * @file upload_flashloader.h
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
#ifndef _USE_FLASHLOADER_H_
#define _USE_FLASHLOADER_H_
#include "DAP.h"
#include <cJSON/cJSON.h>

#include "swd_core.h"


/* Number of times to wait for flashloader */
#define FLASHLOADER_RETRY_COUNT 1000


/* Bit fields for the CSW register */
#define AP_CSW_32BIT_TRANSFER   0x02
#define AP_CSW_AUTO_INCREMENT   0x10
#define AP_CSW_MASTERTYPE_DEBUG (1 << 29)
#define AP_CSW_HPROT            (1 << 25)
#define AP_CSW_DEFAULT (AP_CSW_32BIT_TRANSFER | AP_CSW_MASTERTYPE_DEBUG | AP_CSW_HPROT)



u8 uploadFlashloader(SWD_HANDLE *pswd, uint32_t *addr, uint32_t size);

void checkFlashloader(void);

void erasePage(uint32_t addr);

u8 uploadImageToFlashloader(SWD_HANDLE * pswd);
u8 flashWithFlashloader(SWD_HANDLE * pswd);
u8 erasePagesWithFlashloader(SWD_HANDLE * pswd);
u32 verifyFlashloaderReady(SWD_HANDLE *pswd);
u8 waitForFlashloader_longtime(SWD_HANDLE *pswd, u32 retry);
#endif
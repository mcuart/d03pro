#ifndef __SWD_MAIN_H__
#define __SWD_MAIN_H__


//#include "stm32f10x.h"
#include "stm32f4xx.h"
#include "DAP_config.h"
#include "main.h"
#include "DAP.h"
#include "flashloader.h"
#include "stdbool.h"
#include "utils.h"
#include "use_flashloader.h"
#include <cJSON/cJSON.h>
#include "swd_core.h"



u8 SWD_main(SWD_HANDLE * pswd);
u8 SWD_Auto_detect_start_1(SWD_HANDLE * pswd);
u8 SWD_Auto_detect_end(SWD_HANDLE * pswd);

#endif  /* __SWD_MAIN_H__ */
/**
******************************************************************************
* @file    DCMI/DCMI_CameraExample/main.c 
* @author  MCD Application Team
* @version V1.8.1
* @date    27-January-2022
* @brief   Main program body
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
#include "main.h"
#include "config.h"
#include "swim_core.h"



#include "ff.h"

#include <cJSON/cJSON.h>
//cJSON需要堆至少0x1200否则会解析失败HEAP>=0x1200
#include "aes.h"



#include "Swim_TIM4_CH1_CH2_TIM3_CH3_A.h"//SWIM_1上排
#include "Swim_TIM2_CH1_CH2_TIM5_CH3_B.h"//SWIM_0下排


#include "SWD_main.h"



#include "spi_flash.h"



/*uC/OS-III*********************************************************************************************/
#include  <includes.h>


//device
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"

#include "usbd_desc_msc.h"
#include "usbd_msc_core.h"


//host
#include "usbh_hid_core.h"
#include "usbh_usr.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
* @{
*/
extern USBD_Class_cb_TypeDef USBD_HID_CDC_cb;


/** @addtogroup STM32F4xx_StdPeriph_Examples
* @{
*/

/** @addtogroup DCMI_CameraExample
* @{
*/
GUI_FONT GUI_Fontsongti12;             /* GUI_FONT structure in RAM */
static GUI_XBF_DATA XBF_Data;        /* GUI_XBF_DATA structure in RAM */

//这个是使用SPI读取字库的信息，第一个是偏移地址，第二个是读取长度，最后一个是存储的buff
static int _cbGetData(U32 Off, U16 NumBytes, void * pVoid, void * pBuffer) 
{
  sFLASH_ReadBuffer(pBuffer, Off, NumBytes);
  // SPI_Flash_Read(Off, NumBytes, pBuffer);	//这是存储位置从0开始，如果不一样，需在偏移上加上位置
  return (0);
}

//建立这个字体
void CreateXBF_Font(void) 
{
  GUI_XBF_CreateFont(&GUI_Fontsongti12,                              /* Pointer to GUI_FONT structure */
                     &XBF_Data,                         /* Pointer to GUI_XBF_DATA structure */
                     GUI_XBF_TYPE_PROP,    /* Font type to be created */
                     _cbGetData,                         /* Pointer to callback function */
                     NULL);                                  /* Pointer to be passed to callback */
}


/* Private typedef -----------------------------------------------------------*/
sytem_parameter system_set_data_r;


union
{
  u32 time_data_all;
  struct time_data
  {
    u32 milli:4;//100毫秒
    u32 seconds:6;//秒
    u32 minutes:6;//分钟
    u32 hours:14;//小时16384
    u32 day:2;//
  }time_data_bit;
}time_data_r;


/* Private define ------------------------------------------------------------*/

#define GUI_ID_CHANGE0       (GUI_ID_USER + 5)
#define GUI_ID_CHANGE       (GUI_ID_USER + 6)
#define GUI_ID_MENU       (GUI_ID_USER + 7)
#define GUI_ID_OFFLINE_WRITE       (GUI_ID_USER + 8)
#define GUI_ID_IMAGE_UPDATA       (GUI_ID_USER + 9)
#define GUI_ID_SYSTEM_SET       (GUI_ID_USER + 10)
#define GUI_ID_UDISK_MODE       (GUI_ID_USER + 11)

#define GUI_ID_PRE   (GUI_ID_USER + 12)
#define GUI_ID_NEXT       (GUI_ID_USER + 1)
#define GUI_ID_LISTBOX       (GUI_ID_USER + 13)


#define GUI_ID_TEXT0      (GUI_ID_USER + 14)
#define GUI_ID_TEXT1      (GUI_ID_USER + 15)
#define GUI_ID_TEXT2      (GUI_ID_USER + 16)
#define GUI_ID_TEXT3      (GUI_ID_USER + 17)
#define GUI_ID_TEXT4      (GUI_ID_USER + 18)
#define GUI_ID_TEXT5      (GUI_ID_USER + 19)
#define GUI_ID_TEXT6      (GUI_ID_USER + 20)
#define GUI_ID_TEXT7      (GUI_ID_USER + 21)
#define GUI_ID_TEXT8      (GUI_ID_USER + 22)
#define GUI_ID_TEXT9      (GUI_ID_USER + 23)
#define GUI_ID_TEXT10       (GUI_ID_USER + 24)
#define GUI_ID_TEXT11       (GUI_ID_USER + 25)
#define GUI_ID_TEXT12       (GUI_ID_USER + 26)
#define GUI_ID_TEXT13       (GUI_ID_USER + 27)
#define GUI_ID_TEXT14       (GUI_ID_USER + 28)
#define GUI_ID_TEXT15       (GUI_ID_USER + 29)
#define GUI_ID_TEXT16       (GUI_ID_USER + 30)
#define GUI_ID_TEXT17       (GUI_ID_USER + 31)
#define GUI_ID_TEXT18       (GUI_ID_USER + 32)
#define GUI_ID_TEXT19       (GUI_ID_USER + 33)
#define GUI_ID_TEXT20       (GUI_ID_USER + 34)
#define GUI_ID_TEXT21       (GUI_ID_USER + 35)
#define GUI_ID_TEXT22       (GUI_ID_USER + 36)
#define GUI_ID_TEXT23       (GUI_ID_USER + 37)
#define GUI_ID_TEXT24       (GUI_ID_USER + 38)
#define GUI_ID_TEXT25       (GUI_ID_USER + 39)
#define GUI_ID_TEXT26       (GUI_ID_USER + 40)
#define GUI_ID_TEXT27       (GUI_ID_USER + 41)




#define GUI_ID_CPU_USE       (GUI_ID_USER + 55)
#define GUI_ID_PROGRAM_STATE       (GUI_ID_USER + 56)

#define GUI_ID_SYN       (GUI_ID_USER + 57)
#define GUI_ID_COUNT       (GUI_ID_USER + 58)
#define GUI_ID_COUNT_NUM       (GUI_ID_USER + 59)
#define GUI_ID_FIRMWARE_TIME       (GUI_ID_USER + 60)

#define DRAW_MCUART_COM {\
GUI_SetFont(&GUI_FontAudiowide32);\
  GUI_SetColor(GUI_WHITE);\
    GUI_DispStringAt("mcu",40, 320-50-10);\
      GUI_SetColor(GUI_BLUE);\
        GUI_DispString("art.com");}



const char *pro_item_0[] = {" "," "," "," "," "," "," "," "," "," "," "," "," "," "," "};
const char *pro_item_1[] = {"1.绑定UID","2.清芯片","3.擦指定页","4.写FLASH","5.校FLASH","6.写EEP","7.校EEP","8.写OTP","9.校OTP","10.ID加密","11.写滚码","12.写选项字","13.智能复位","14.写后跳转","15.跳转到    "};
const char *pro_item_2[] = {"1.","2.清芯片","3.","4.写APROM","5.校APROM","6.写LDROM","7.校LDROM","8.写SPROM","9.校SPROM","10.","11.写滚码","12.写配置字","13.智能复位","14.写后跳转","15.跳转到    "};
const char *pro_item_3[] = {"1.","2.清芯片","3.擦指定页","4.写FLASH","5.校FLASH","6.","7.","8.","9.","10.","11.写滚码","12.","13.","14.","15."};
const char *pro_item_4[] = {"1.","2.清芯片","3.","4.","5.","6.写EEP","7.校EEP","8.","9.","10.","11.写滚码","12.","13.","14.","15."};


#define XSIZE_PHYS  240 // To be adapted to x-screen size
#define YSIZE_PHYS  320 // To be adapted to y-screen size、
const GUI_WIDGET_CREATE_INFO _aFrameWinControl_offline_write[] = {
  { WINDOW_CreateIndirect, "Control", 0,                0,  0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW,          0 },
  { TEXT_CreateIndirect,     "1.绑定UID",        GUI_ID_TEXT0,     5,  108, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "2.清芯片",        GUI_ID_TEXT1,     5,  123, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "3.擦指定页",        GUI_ID_TEXT2,     5,  138, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,     "4.写FLASH",        GUI_ID_TEXT3,     5,  153, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "5.校FLASH",        GUI_ID_TEXT4,     5,  168, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "6.写EEP",        GUI_ID_TEXT5,     82,  108, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,     "7.校EEP",        GUI_ID_TEXT6,     82,  123, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "8.写OTP",        GUI_ID_TEXT7,     82,  138, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "9.校OTP",        GUI_ID_TEXT8,     82,  153, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,     "10.ID加密",        GUI_ID_TEXT9,     82,  168, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "11.写滚码",        GUI_ID_TEXT10,     159,  108, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "12.写选项字",        GUI_ID_TEXT11,     159,  123, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,     "13.智能复位",        GUI_ID_TEXT12,     159,  138, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "14.写后跳转",        GUI_ID_TEXT13,     159,  153, 0,    0,    0,          0 },  
  { TEXT_CreateIndirect,     "15.跳转到    ",        GUI_ID_TEXT14,     159,  168, 0,    0,    0,          0 },    
  { TEXT_CreateIndirect,     "0000:00:00:0",        GUI_ID_TEXT15,     165,  183, 0,    0,    0,          0 },  
  //  { TEXT_CreateIndirect,     "芯片:",        GUI_ID_TEXT16,     167,  195, 0,    0,    0,          0 }, 
  //  { TEXT_CreateIndirect,     "电压:",        GUI_ID_TEXT17,     167,  209, 0,    0,    0,          0 },
  //  { TEXT_CreateIndirect,     "接口:",        GUI_ID_TEXT18,     167,  223, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,     "镜像:",        GUI_ID_TEXT19,     6,  6, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "镜像号:",        GUI_ID_TEXT20,     170,  6, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "checksum:",        GUI_ID_TEXT21,     5,  23, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "本次",        GUI_ID_TEXT22,     5,  37, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "烧录",        GUI_ID_TEXT23,     5,  51, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "0000000",     GUI_ID_TEXT24,     32,  39, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "已用次数:0000000000",     GUI_ID_TEXT25,     122,  23, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "剩余次数:0000000000",     GUI_ID_TEXT26,     122,  37, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "下次滚码:0x00000000",     GUI_ID_TEXT27,     122,  51, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "CPU:",     GUI_ID_CPU_USE,     125,  67, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "准备就绪",     GUI_ID_PROGRAM_STATE,     120,  79, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "同步",     GUI_ID_SYN,     5,  67, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "计数",     GUI_ID_COUNT,     5,  81, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "28/28",     GUI_ID_COUNT_NUM,     32,  69, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,   "固件版本",     GUI_ID_FIRMWARE_TIME,     5,  293, 0,    0,    0,          0 },
  { BUTTON_CreateIndirect,   "A通道",    GUI_ID_CHANGE0,   165, 198, 70,  40,  0,          0 },  
  { BUTTON_CreateIndirect,   "B通道",    GUI_ID_CHANGE,   165, 238, 70,  40,  0,          0 },
  { BUTTON_CreateIndirect,   "返回菜单",    GUI_ID_MENU,     165, 278, 70,  27,  0,          0 },

};

#define BUTTON_SIZE_X     150
#define BUTTON_SIZE_Y     45
const GUI_WIDGET_CREATE_INFO _aFrameWinControl_main_menu[] = {
  { WINDOW_CreateIndirect,   "main_menu",0,                0,  0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_HIDE,          0 },
  { BUTTON_CreateIndirect,   "脱机烧写",    GUI_ID_OFFLINE_WRITE,      50, 50, BUTTON_SIZE_X,  BUTTON_SIZE_Y,  0,          0 },
  { BUTTON_CreateIndirect,   "镜像更新",    GUI_ID_IMAGE_UPDATA,     50, 100, BUTTON_SIZE_X,  BUTTON_SIZE_Y,  0,          0 }, 
  { BUTTON_CreateIndirect,   "系统设置",    GUI_ID_SYSTEM_SET,     50, 150, BUTTON_SIZE_X,  BUTTON_SIZE_Y,  0,          0 }, 
  { BUTTON_CreateIndirect,   "优盘模式",    GUI_ID_UDISK_MODE,     50, 200, BUTTON_SIZE_X,  BUTTON_SIZE_Y,  0,          0 }, 
  { TEXT_CreateIndirect,       "固件特征码:",             GUI_ID_TEXT0,     110,  290, 0,    0,    0,          0 },
  { TEXT_CreateIndirect,     "固件版本:",             GUI_ID_TEXT1,     110,  305, 0,    0,    0,          0 },
  //    { TEXT_CreateIndirect,     0,             GUI_ID_TEXT1,     98,  260, 0,    0,    0,          0 }
};
/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END;
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev_winusb __ALIGN_END;
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev_msc __ALIGN_END;

#pragma data_alignment=4
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_Core_dev __ALIGN_END;
#pragma data_alignment=4
__ALIGN_BEGIN USBH_HOST USB_Host __ALIGN_END;

const GUI_WIDGET_CREATE_INFO _aFrameWinControl_listbox[] = {
  { WINDOW_CreateIndirect, "image_listbox",0,                0,  0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_HIDE,          0 },
  { LISTBOX_CreateIndirect,  0,         GUI_ID_LISTBOX,  0, 0, 240, 280, WM_CF_HIDE, 0 },
  { BUTTON_CreateIndirect,   "取消",    GUI_ID_CANCEL,      1, 281, 57,  35,  0,          0 },
  { BUTTON_CreateIndirect,   "上一页",    GUI_ID_PRE,     60, 281, 57,  35,  0,          0 }, 
  { BUTTON_CreateIndirect,   "下一页",    GUI_ID_NEXT,      120, 281, 57,  35,  0,          0 },
  { BUTTON_CreateIndirect,   "确定",    GUI_ID_OK,     180, 281, 57,  35,  0,          0 }, 
};

const GUI_WIDGET_CREATE_INFO _aFrameWinControl_image_updata[] = {
  { WINDOW_CreateIndirect, "image_updata", 0,                0,  0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_HIDE,          0 }, 
  { TEXT_CreateIndirect,     "^*^*^*镜像更新模式*^*^*^",        GUI_ID_TEXT2,     15,  20, 0,    0,    0,          0 },  
  { BUTTON_CreateIndirect,   "返回主菜单",    GUI_ID_BUTTON0,     70, 210, 100,  45,  0,          0 },  
};

const GUI_WIDGET_CREATE_INFO _aFrameWinControl_udisk_mode[] = {
  { WINDOW_CreateIndirect, "udisk_mode", 0,                0,  0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_HIDE,          0 }, 
  { TEXT_CreateIndirect,     "^*^*^*^*优盘模式*^*^*^*^",        GUI_ID_TEXT2,     15,  20, 0,    0,    0,          0 },  
  { BUTTON_CreateIndirect,   "返回主菜单",    GUI_ID_BUTTON0,     70, 210, 100,  45,  0,          0 },  
};

const GUI_WIDGET_CREATE_INFO _aFrameWinControl_system_set[] = {
  { WINDOW_CreateIndirect, "system_set", 0,                0,  0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_HIDE,          0 }, 
  { TEXT_CreateIndirect,     "^*^*^*^*系统设置*^*^*^*^",        GUI_ID_TEXT2,     15,  20, 0,    0,    0,          0 },  
  { CHECKBOX_CreateIndirect,   "使能蜂鸣器提示",    GUI_ID_CHECK0,   20, 50, 150,  16,  0,          0 },
  { CHECKBOX_CreateIndirect,   "开机进入脱机烧写",    GUI_ID_CHECK1,   20, 70, 150,  16,  0,          0 },
  { CHECKBOX_CreateIndirect,   "允许自动连接计算机",    GUI_ID_CHECK2,   20, 90, 150,  16,  0,          0 },
  { CHECKBOX_CreateIndirect,   "使能开机密码",    GUI_ID_CHECK3,   20, 110, 150,  16,  0,          0 },
  { CHECKBOX_CreateIndirect,   "使能A通道",    GUI_ID_CHECK4,   20, 130, 150,  16,  0,          0 },
  { CHECKBOX_CreateIndirect,   "使能B通道",    GUI_ID_CHECK5,   20, 150, 150,  16,  0,          0 },  
  { BUTTON_CreateIndirect,   "返回菜单",    GUI_ID_BUTTON0,     75, 200, 90,  55,  0,          0 },  
};


extern uint8_t EnableUsbCreateFile;
extern uint8_t UsbTransferComplete;
extern uint8_t UsbTransferCompleteCount;
extern u32 transfertimes;
extern FIL f_name;
FRESULT res;         // FatFs function common result code
FATFS fs;            // Work area (file system object) for logical drive
DIR mulu;
char sp1[100];
char sp_a[100];
char sp_b[100];
u16 ImageNumber_store=0;
u16 ImageNumber_store_a=0;
u16 ImageNumber_store_b=0;
#define HEADER_LENGTH 2048
FIL fsrc;
u8 buf[HEADER_LENGTH];//镜像切换列表显示临时性buf1
u8 buf_a[HEADER_LENGTH];//镜像切换列表显示临时性buf1
u8 buf_b[HEADER_LENGTH];//镜像切换列表显示临时性buf1
u32 br;         // File R/W count

u16 listcount=0;
u16 listcount_a=0;
u16 listcount_b=0;

int listselected=0;
int listselected_a=0;
int listselected_b=0;

u8 parameter[100];//100字节


u32 password_store;
u32 language_store;
u8 auto_connect_computer;
u8 password_enable_store;

uint8_t sFLASH_Rx_Buffer[100];
uint8_t sFLASH_Tx_Buffer[100];

cJSON *root;
cJSON *root_a;
cJSON *root_b;



char  sp[100];
char  sp1[100];

u32 totaltimes=0;
u32 totaltimes_used=0;
u32 CurrentTimes=0;
u32 SynCountTimes=0;

float ADCConvertedValueTemp[3];


WM_HWIN h_pro_state;
WM_HWIN h_pro_CurrentTimes;
WM_HWIN h_pro_syn_count;
WM_HWIN h_pro_time;
WM_HWIN h_pro_cpu_use;
WM_HTIMER h_timer;

WM_HWIN h_pro_change0;
WM_HWIN h_pro_change;
WM_HWIN h_pro_menu;



WM_HTIMER h_timer;
u8 has_window=0;
u8 channel=0;
WM_HWIN _hDialogControl_offline_write;
WM_HWIN _hDialogControl_listbox;
WM_HWIN _hDialogControl_voltage_set;
WM_HWIN _hDialogControl_main_menu;
WM_HWIN _hDialogControl_image_updata;
WM_HWIN _hDialogControl_system_set;
WM_HWIN _hDialogControl_udisk_mode;

//extern GUI_CONST_STORAGE GUI_FONT GUI_Fontsongti12;
extern GUI_CONST_STORAGE GUI_FONT GUI_Fontsongti16;
extern GUI_CONST_STORAGE GUI_FONT GUI_Fontsongti17;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontAudiowide32;
extern GUI_CONST_STORAGE GUI_FONT GUI_Fontlizhicaiyun;

extern GUI_CONST_STORAGE GUI_BITMAP bmUSB;
extern GUI_CONST_STORAGE GUI_BITMAP bmupdate;
extern GUI_CONST_STORAGE GUI_BITMAP bmchip;

void _cbFrameWinControl_offline_write(WM_MESSAGE * pMsg);//脱机烧录模式界面回调函数
void _cbFrameWinControl_voltage_set(WM_MESSAGE * pMsg);//电压设置模式界面回调函数
void _cbFrameWinControl_listbox(WM_MESSAGE * pMsg);//镜像切换模式界面回调函数
void _cbFrameWinControl_main_menu(WM_MESSAGE * pMsg);//主菜单界面回调函数
void _cbFrameWinControl_system_set(WM_MESSAGE * pMsg);//系统设置模式界面回调函数
void _cbFrameWinControl_image_updata(WM_MESSAGE * pMsg);//镜像更新模式界面回调函数
void _cbFrameWinControl_udisk_mode(WM_MESSAGE * pMsg);//优盘模式界面回调函数



void tmr1_callback(void *p_tmr, void *p_arg); 	//定时器1回调函数

void tf_card_warning(void);

void aes_encode(u8 * p_Buffer,u32 length_size);//aes加密
void aes_decode(u8 * p_Buffer,u32 length_size);//aes解密
void voltage_caculate(void);//电压计算
void out_voltage_select(void);//选择输出电压

RCC_ClocksTypeDef RCC_Clocks;


__IO uint16_t  uhADCVal = 0;
uint8_t        abuffer[40];


extern __IO uint8_t         ValueMax;


u16 displayposition=DISPLAY_POSITION_Y;


void display_detect_chip_id_code_ok(u32 idcode)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  //  GUI_SetTextMode(GUI_TM_TRANS);
  
  //displayposition=185;
  GUI_DispStringAt ("IDCODE=0x",7, displayposition); 
  //16进制显示
  GUI_DispHex(idcode,8);//允许自动连接计算机
  displayposition+=15; 
  
  CPU_CRITICAL_EXIT();	//退出临界区
}

void display_error_code(u32 error_code)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  
  GUI_DispStringAt ("ERROR=0x",7, displayposition); 
  //16进制显示
  GUI_DispHex(error_code,8);//允许自动连接计算机
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
}


u8 progress_bar_tmp=200;
u8 mh=0;
#define ENABLE_PROGRESS_REFRESH (progress_bar_tmp=200)
void draw_progress_bar(u8 i);


void display_clear_chip_ok(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  //if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("清芯片成功√",7, displayposition);
  }
  //  else
  {
    // GUI_DispStringAt ("clear chip ok√",7, displayposition); 
  }
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
  
}


void display_erase_sector_ok(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  //if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("擦指定页成功√",7, displayposition);
  }
  //  else
  {
    // GUI_DispStringAt ("erase sector ok√",7, displayposition); 
  }
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
}

void display_write_page_flash_ok(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  //if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("写指定页成功√",7, displayposition);
  }
  //  else
  {
    // GUI_DispStringAt ("erase sector ok√",7, displayposition); 
  }
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
}


void display_write_flash_ok(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  //  if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("写FLASH成功√",7, displayposition);
  }
  //  else
  {
    //  GUI_DispStringAt ("program flash ok√",7, displayposition); 
  }
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
}

void display_verify_flash_ok(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);      
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  //if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("校验FLASH成功√",7, displayposition);
  }
  //else
  {
    //  GUI_DispStringAt ("verify flash ok√",7, displayposition); 
  }
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
}


u32 verify_failed_address;
void display_verify_flash_different(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  // if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("校验失败",7, displayposition);
  }
  // else
  {
    //  GUI_DispStringAt ("Verify failed",7, displayposition); 
  }
  char tmpbuf[20];
  sprintf(tmpbuf," @0x%08X",verify_failed_address);
  
  GUI_DispString(tmpbuf);
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
}

void display_write_option_byte_ok(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区
  WM_SelectWindow(_hDialogControl_offline_write);
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  //if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("写选项字成功√",7, displayposition);
  }
  // else
  {
    //  GUI_DispStringAt ("program option byte ok√",7, displayposition); 
  }
  displayposition+=15; 
  CPU_CRITICAL_EXIT();	//退出临界区
}


u8 unlock_chip_ok_flag=0;
void display_unlock_chip_ok(void)
{
  //  GUI_SetFont(&GUI_Fontsongti12);
  //  GUI_SetColor(GUI_WHITE);
  //  if(language_store==0x00000005)      
  //  {
  //      GUI_DispStringAt ("解锁芯片成功√",7, displayposition);
  //  }
  //  else
  //  {
  //    GUI_DispStringAt ("unlock chip ok√",7, displayposition); 
  //  }
  //  displayposition+=15;
  //  unlock_chip_ok_flag=1;
  //  WM_Exec();
}






void display_write_config_byte_ok(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("写配置字成功√",7, displayposition);
  }
  else
  {
    GUI_DispStringAt ("program config byte ok√",7, displayposition); 
  }
  displayposition+=15; 
  WM_Exec();
}

void display_write_aprom_ok(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("写APROM成功√",7, displayposition);
  }
  else
  {
    GUI_DispStringAt ("program aprom ok√",7, displayposition); 
  }
  displayposition+=15; 
  WM_Exec();
}

void display_write_ldrom_ok(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("写LDROM成功√",7, displayposition);
  }
  else
  {
    GUI_DispStringAt ("program ldrom ok√",7, displayposition); 
  }
  displayposition+=15; 
  WM_Exec();
}

void display_verify_ldrom_ok(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("校验LDROM成功√",7, displayposition);
  }
  else
  {
    GUI_DispStringAt ("verify ldrom ok√",7, displayposition); 
  }
  displayposition+=15; 
  WM_Exec();
}

void display_verify_aprom_ok(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  if(language_store==0x00000005)      
  {
    GUI_DispStringAt ("校验APROM成功√",7, displayposition);
  }
  else
  {
    GUI_DispStringAt ("verify ldrom ok√",7, displayposition); 
  }
  displayposition+=15; 
  WM_Exec();
}


void display_WriteRollingCode_ok(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  
  GUI_DispStringAt ("写滚码成功√",7, displayposition);
  
  
  
  
  
  displayposition+=15; 
  WM_Exec();
}

void display_WriteCodeBar_ok(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_WHITE);
  
  GUI_DispStringAt ("扫码SN写入成功√",7, displayposition);
  
  displayposition+=15; 
  WM_Exec();
}

void display_WriteCodeBar_ignore(void)
{
  GUI_SetFont(&GUI_Fontsongti12);
  GUI_SetColor(GUI_YELLOW);
  
  GUI_DispStringAt ("码枪未连接,SN写入已取消!",7, displayposition);
  
  displayposition+=15; 
  WM_Exec();
}

uint8_t GUI_Initialized   = 0;

//uint8_t  Year, Month, Day,hh,mm,ss;
typedef struct FIRMWARE_TIME
{
  uint8_t Year;
  uint8_t Month;
  uint8_t Day;
  uint8_t hh;
  uint8_t mm;
  uint8_t ss;
} FIRMWARE_TIME_HANDLE;


FIRMWARE_TIME_HANDLE firmware_time;


/* Private function prototypes -----------------------------------------------*/
static void ADC_Config(void);
void Get_Compile_Time(uint8_t *Year, uint8_t *Month, uint8_t *Day,uint8_t *hh,uint8_t *mm,uint8_t *ss);


/******************************************************************************************************/
/*uC/OS-III配置*/

/* START_TASK 任务 配置
* 包括: 任务优先级 任务栈大小 任务控制块 任务栈 任务函数
*/
#define START_TASK_PRIO 8                               /* 任务优先级 */
#define START_STK_SIZE  128                             /* 任务栈大小 */
OS_TCB                  StartTask_TCB;                  /* 任务控制块 */
//#pragma data_alignment=8
CPU_STK                 StartTask_STK[START_STK_SIZE];  /* 任务栈 */
void start_task(void *p_arg);                           /* 任务函数 */


//任务优先级
#define KEY_TASK_PRIO		4
//任务堆栈大小	
#define KEY_STK_SIZE 		128
//任务控制块
OS_TCB Key_TaskTCB;
//任务堆栈	
CPU_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *p_arg);


//任务优先级
#define EMWIN_TASK_PRIO			5
//任务堆栈大小	
#define EMWIN_STK_SIZE			512
//任务控制块
OS_TCB Emwin_TaskTCB;
//任务堆栈	
//#pragma data_alignment=8
CPU_STK EMWIN_TASK_STK[EMWIN_STK_SIZE];
//任务函数
void emwin_task(void *p_arg);


//任务优先级
#define PROGRAM_A_TASK_PRIO		3
//任务堆栈大小	
#define PROGRAM_A_STK_SIZE 		512
//任务控制块
OS_TCB Program_a_TaskTCB;
//任务堆栈
//#pragma data_alignment=8
CPU_STK PROGRAM_A_TASK_STK[PROGRAM_A_STK_SIZE];
//任务函数
void program_a_task(void *p_arg);


//任务优先级
#define PROGRAM_B_TASK_PRIO		3
//任务堆栈大小	
#define PROGRAM_B_STK_SIZE 		512
//任务控制块
OS_TCB Program_b_TaskTCB;
//任务堆栈
//#pragma data_alignment=8
CPU_STK PROGRAM_B_TASK_STK[PROGRAM_B_STK_SIZE];
//任务函数
void program_b_task(void *p_arg);


//任务优先级
#define BARGUN_PROCESS_TASK_PRIO		2
//任务堆栈大小	
#define BARGUN_PROCESS_STK_SIZE			256
//任务控制块
OS_TCB Bargun_process_TaskTCB;
//任务堆栈	
//#pragma data_alignment=8
CPU_STK BARGUN_PROCESS_TASK_STK[BARGUN_PROCESS_STK_SIZE];
//任务函数
void bargun_process_task(void *p_arg);

//任务优先级
#define BARGUN_START_SOURCE_TASK_PRIO		6
//任务堆栈大小	
#define BARGUN_START_SOURCE_STK_SIZE			256
//任务控制块
OS_TCB Bargun_start_source_TaskTCB;
//任务堆栈	
//#pragma data_alignment=8
CPU_STK BARGUN_START_SOURCE_TASK_STK[BARGUN_START_SOURCE_STK_SIZE];
//任务函数
void bargun_start_source_task(void *p_arg);



u8 flash_control_reg_address;
OS_TMR 	tmr1;		//定时器1
u8 kaiguan=0;
u16  ss=0;
/* Private functions ---------------------------------------------------------*/

/**
* @brief  Main program
* @param  None
* @retval None
*/
int main(void)
{
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000); 
  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  files (startup_stm32f40_41xxx.s/startup_stm32f427_437xx.s/startup_stm32f429_439xx.s)
  before to branch to application main.
  */
  OS_ERR err;
  //BSP_Tick_Init();
  delay_init();  
  
  //  ImageNumber_store=2;
  //  
  //  sprintf(sp1,"0:/system/dat/HEAD%03u.BIN",ImageNumber_store);
  //  if(!f_open(&fsrc, sp1,  FA_READ))
  //  {
  //    f_read (&fsrc,buf,HEADER_LENGTH,&br);
  //    
  //    f_close (&fsrc);
  //    
  //    aes_decode(buf,HEADER_LENGTH);
  //    
  //    if(root)
  //      cJSON_Delete(root);
  //    root = cJSON_Parse((char const *)&buf[0]);
  //  }
  //  
  //  SWIM_GPIO_Init();
  //  
  //  
  //  extern u8 flash_control_reg_address;
  //  flash_control_reg_address=0;
  //  while(1)
  //  {
  //    if(kaiguan==0)
  //    {
  //      
  //      flash_control_reg_address=(u8)cJSON_GetObjectItem(root, "flash_reg_address")->valueint;
  //      ss=Pro_Init();
  //      
  //      ss=Write_OpitonByte_STM8_pre();
  //      
  //      ss=ClearChip();
  //      ss=ProgramFlash();
  //      ss=Write_OpitonByte_STM8();
  //      ss=End_Pro();           
  //      kaiguan=1;
  //    }
  //  }    
  
  
  
  
  
  /* 初始化uC/OS-III */
  OSInit(&err);
  
  /* 创建Start Task */
  OSTaskCreate(   (OS_TCB        *)&StartTask_TCB,
               (CPU_CHAR      *)"start_task",
               (OS_TASK_PTR    )start_task,
               (void          *)0,
               (OS_PRIO        )START_TASK_PRIO,
               (CPU_STK       *)StartTask_STK,
               (CPU_STK_SIZE   )START_STK_SIZE / 10,
               (CPU_STK_SIZE   )START_STK_SIZE,
               (OS_MSG_QTY     )0,
               (OS_TICK        )0,
               (void          *)0,
               (OS_OPT         )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (OS_ERR        *)&err);
  
  /* 开始任务调度 */
  OSStart(&err);
  
  while(1)
  {
    
  }
  
  
}


__IO uint32_t FlashID = 0;
/**
* @brief       start_task
* @param       p_arg : 传入参数(未用到)
* @retval      无
*/
void start_task(void *p_arg)
{
  CPU_SR_ALLOC();
  OS_ERR err;
  CPU_INT32U cnts;
  
  /* 初始化CPU库 */
  CPU_Init();
  
  
#if OS_CFG_STAT_TASK_EN > 0u
  OSStatTaskCPUUsageInit(&err);  	//í3??è???                
#endif  
  
  
  
  //  //  /* 根据配置的节拍频率配置SysTick */
  //  cnts = (CPU_INT32U)(SystemCoreClock / OSCfg_TickRate_Hz);
  //  OS_CPU_SysTickInit(cnts);
  
  /* 开启时间片调度，时间片设为默认值 */
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //
  OSSchedRoundRobinCfg(DEF_ENABLED,0,&err);  
#endif	
  //NVIC_Configuration();
  
  /* USART configuration */
  //USART_Config();
  //device
  USBD_Init(&USB_OTG_dev_winusb,USB_OTG_FS_CORE_ID,&USR_desc, &USBD_HID_CDC_cb, &USR_cb);
  USB_OTG_dev=USB_OTG_dev_winusb;
  USBD_Init(&USB_OTG_dev_msc,USB_OTG_FS_CORE_ID,&USR_desc_msc, &USBD_MSC_cb, &USR_cb);
  DCD_DevDisconnect(&USB_OTG_dev_winusb);
  DCD_DevDisconnect(&USB_OTG_dev_msc);
  //host
  USBH_Init(&USB_OTG_Core_dev,USB_OTG_HS_CORE_ID,&USB_Host, &HID_cb, &USR_Callbacks);
  //TIM6_Config();
  KEY_Init();
  ADC_Config();
  GPIO_Config();
  TIM1_Config();
  /* Initialize the LCD */
  LCD_Init();
  
  /* Enable the CRC Module */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);  
  Touch_Init();
  
  
  
  /* Activate the use of memory device feature */
  // WM_SetCreateFlags(WM_CF_MEMDEV);
  
  /* Init the STemWin GUI Library */
  GUI_Init();
  
  FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
  SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
  BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
  BUTTON_SetDefaultTextColor(GUI_GRAY_D0,BUTTON_CI_DISABLED);
  PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
  RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
  CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
  CHECKBOX_SetDefaultBkColor(0x787878);
  LISTBOX_SetDefaultBkColor(LISTBOX_CI_UNSEL,0x1AA4F2);
  LISTBOX_SetDefaultBkColor(LISTBOX_CI_SELFOCUS,0x41170B);
  LISTBOX_SetDefaultTextColor(LISTBOX_CI_UNSEL,GUI_WHITE);
  
  
  
  GUI_UC_SetEncodeUTF8(); // Enable UTF8 decoding
  BUTTON_SetDefaultFont(&GUI_Fontsongti16);
  RADIO_SetDefaultFont(&GUI_Fontsongti12);
  CHECKBOX_SetDefaultFont(&GUI_Fontsongti12);
  LISTBOX_SetDefaultFont(&GUI_Fontsongti12);
  TEXT_SetDefaultFont(&GUI_Fontsongti12);
  RADIO_SetDefaultTextColor (GUI_WHITE);
  FRAMEWIN_SetDefaultFont(&GUI_Fontsongti12);
  BUTTON_SetDefaultFocusColor(GUI_YELLOW);       
  
  /* Initialize the SPI FLASH driver */
  sFLASH_Init();
  
  /* Get SPI Flash ID */
  FlashID = sFLASH_ReadID();  
  
  sFLASH_ReadBuffer(sFLASH_Rx_Buffer, FLASH_READ_ADDRESS, 100);//读参数信息
  
  
  CreateXBF_Font();  //创建XBF字体
  GUI_UC_SetEncodeUTF8(); //这个必须要有,SPIFLASH里保存的是UTF-8 xbf字库
  
  
  res=f_mount(0, &fs);
  res=f_opendir (&mulu,"0:");/* Open an existing directory */
  
  if(FR_OK!=res)//存储器异常，即将重启动
  {
    tf_card_warning();
  }
  else//验证合法通过
  {
    res=f_opendir (&mulu,"0:/system");/* Open an existing directory */
    if(FR_OK==res)
    {
      sprintf(sp1,"0:/system/dat/HEAD%03d.BIN",ImageNumber_store);
      
      if(!f_open(&fsrc, sp1,  FA_READ))
      {
        f_read (&fsrc,buf,HEADER_LENGTH,&br);//关键性的修改buf中的数据
        f_close (&fsrc);
      }
    }
    else//创建system/dat目录
    {
      f_mkdir("system");
      f_mkdir("system/dat");
      //memset(buf,0,HEADER_LENGTH);
    }
  } 
  
  
  Get_Compile_Time(&firmware_time.Year, &firmware_time.Month, &firmware_time.Day,&firmware_time.hh,&firmware_time.mm,&firmware_time.ss);//取编译时间
  
  
  
  
  
  //创建定时器1
  OSTmrCreate((OS_TMR		*)&tmr1,		//定时器1
              (CPU_CHAR	*)"tmr1",		//定时器名字
              (OS_TICK	 )20,			//20*10=200ms
              (OS_TICK	 )2,          //2*10=20ms
              (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
              (OS_TMR_CALLBACK_PTR)tmr1_callback,//定时器1回调函数
              (void	    *)0,			//参数为0
              (OS_ERR	    *)&err);		//返回的错误码
  
  
  
  CPU_CRITICAL_ENTER();//进入临界区
  
  
  //      /* 创建事件标志 */
  //    OSFlagCreate(   (OS_FLAG_GRP   *)&flag,
  //                    (CPU_CHAR      *)"flag",
  //                    (OS_FLAGS       )0,
  //                    (OS_ERR        *)&err);
  
  
  OSTaskCreate((OS_TCB*     )&Emwin_TaskTCB,		
               (CPU_CHAR*   )"Emwin task", 		
               (OS_TASK_PTR )emwin_task, 			
               (void*       )0,					
               (OS_PRIO	  )EMWIN_TASK_PRIO,     
               (CPU_STK*    )&EMWIN_TASK_STK[0],	
               (CPU_STK_SIZE)EMWIN_STK_SIZE/10,	
               (CPU_STK_SIZE)EMWIN_STK_SIZE,		
               (OS_MSG_QTY  )0,					
               (OS_TICK	  )0,  					
               (void*       )0,					
               (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
               (OS_ERR*     )&err);
  
  
  OSTaskCreate((OS_TCB*     )&Bargun_process_TaskTCB,		
               (CPU_CHAR*   )"Bargun proceess task", 		
               (OS_TASK_PTR )bargun_process_task, 			
               (void*       )0,					
               (OS_PRIO	  )BARGUN_PROCESS_TASK_PRIO,     
               (CPU_STK*    )&BARGUN_PROCESS_TASK_STK[0],	
               (CPU_STK_SIZE)BARGUN_PROCESS_STK_SIZE/10,	
               (CPU_STK_SIZE)BARGUN_PROCESS_STK_SIZE,		
               (OS_MSG_QTY  )0,					
               (OS_TICK	  )0,  					
               (void*       )0,					
               (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
               (OS_ERR*     )&err);
  OSTaskSuspend((OS_TCB*)&Bargun_process_TaskTCB,&err);//挂起码枪任务，不再检测  
  
  OSTaskCreate((OS_TCB*     )&Bargun_start_source_TaskTCB,		
               (CPU_CHAR*   )"Bargun_start_source task", 		
               (OS_TASK_PTR )bargun_start_source_task, 			
               (void*       )0,					
               (OS_PRIO	  )BARGUN_START_SOURCE_TASK_PRIO,     
               (CPU_STK*    )&BARGUN_START_SOURCE_TASK_STK[0],	
               (CPU_STK_SIZE)BARGUN_START_SOURCE_STK_SIZE/10,	
               (CPU_STK_SIZE)BARGUN_START_SOURCE_STK_SIZE,		
               (OS_MSG_QTY  )0,					
               (OS_TICK	  )0,  					
               (void*       )0,					
               (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
               (OS_ERR*     )&err);  
  
  //创建按键处理任务
  OSTaskCreate((OS_TCB 	* )&Key_TaskTCB,		
               (CPU_CHAR	* )"Key task", 		
               (OS_TASK_PTR )key_task, 			
               (void		* )0,					
               (OS_PRIO	  )KEY_TASK_PRIO,     
               (CPU_STK   * )&KEY_TASK_STK[0],	
               (CPU_STK_SIZE)KEY_STK_SIZE/10,	
               (CPU_STK_SIZE)KEY_STK_SIZE,		
               (OS_MSG_QTY  )0,					
               (OS_TICK	  )0,  					
               (void   	* )0,					
               (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
               (OS_ERR 	* )&err);	  
  
  
  //创建PROGRAM_A任务
  OSTaskCreate((OS_TCB 	* )&Program_a_TaskTCB,		
               (CPU_CHAR	* )"Program a task", 		
               (OS_TASK_PTR )program_a_task, 			
               (void		* )0,					
               (OS_PRIO	  )PROGRAM_A_TASK_PRIO,     
               (CPU_STK   * )&PROGRAM_A_TASK_STK[0],	
               (CPU_STK_SIZE)PROGRAM_A_STK_SIZE/10,	
               (CPU_STK_SIZE)PROGRAM_A_STK_SIZE,		
               (OS_MSG_QTY  )0,					
               (OS_TICK	  )0,  					
               (void   	* )0,					
               (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
               (OS_ERR 	* )&err);	   
  OSTaskSuspend((OS_TCB*)&Program_a_TaskTCB,&err);//挂起烧写任务 
  
  //创建PROGRAM_B任务
  OSTaskCreate((OS_TCB 	* )&Program_b_TaskTCB,		
               (CPU_CHAR	* )"Program b task", 		
               (OS_TASK_PTR )program_b_task, 			
               (void		* )0,					
               (OS_PRIO	  )PROGRAM_B_TASK_PRIO,     
               (CPU_STK   * )&PROGRAM_B_TASK_STK[0],	
               (CPU_STK_SIZE)PROGRAM_B_STK_SIZE/10,	
               (CPU_STK_SIZE)PROGRAM_B_STK_SIZE,		
               (OS_MSG_QTY  )0,					
               (OS_TICK	  )0,  					
               (void   	* )0,					
               (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
               (OS_ERR 	* )&err);	   
  OSTaskSuspend((OS_TCB*)&Program_b_TaskTCB,&err);//挂起烧写任务 
  
  
  
  CPU_CRITICAL_EXIT();//退出临界区
  OSTaskDel((OS_TCB*)&StartTask_TCB,&err);//删除开始任务	(先退出临界区再删除开始任务，否则会导致无法退出临界区)
}


void key_task(void *p_arg)
{
  u8 key;
  u8 start_source_a;
  u8 start_source_b;
  OS_ERR err;
  while(1)
  {
    if(has_window==1)
    {
      key=0;
      while(KEY_Scan(1)==KEY1_PRES)
      {
        key++;
        OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms
      }
      
      if(key>=100)//1s
      {
        
      }
      else if(key>=2)//20ms
      {
        //OSTaskSemPost(&Program_TaskTCB,OS_OPT_POST_NONE,&err);//使用系统内建信号量向任务program_task发送信号量
        OSTaskResume((OS_TCB*)&Program_a_TaskTCB,&err);//恢复烧写任务A
        OSTaskResume((OS_TCB*)&Program_b_TaskTCB,&err);//恢复烧写任务B
        start_source_a=0;
        OSTaskQPost((OS_TCB    *)&Program_a_TaskTCB,
                    (void      *)&start_source_a,
                    (OS_MSG_SIZE)sizeof(start_source_a),
                    (OS_OPT     )OS_OPT_POST_FIFO,
                    (OS_ERR    *)&err);
        start_source_b=0;
        OSTaskQPost((OS_TCB    *)&Program_b_TaskTCB,
                    (void      *)&start_source_b,
                    (OS_MSG_SIZE)sizeof(start_source_b),
                    (OS_OPT     )OS_OPT_POST_FIFO,
                    (OS_ERR    *)&err);
        
        
      }
    }
    OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);//延时10ms
  }
}


//emwin界面任务
void emwin_task(void *pdata)
{
  OS_ERR err;
  WM_SetDesktopColor(0x787878);
  WINDOW_SetDefaultBkColor(0x787878);
  system_set_data_r.system_set_data_all=*(u32*)SYSTEM_SET_DATA_ADDRESS_R;
  
  if(system_set_data_r.system_set_data_bit.bit_4==0)
  {
    OSTaskDel((OS_TCB * )&Program_a_TaskTCB,(OS_ERR * )&err);
  }
  if(system_set_data_r.system_set_data_bit.bit_5==0)
  {
    OSTaskDel((OS_TCB * )&Program_b_TaskTCB,(OS_ERR * )&err);
  }  
  
  
  if(system_set_data_r.system_set_data_bit.bit_1==1)//设置开机进入脱机烧写
  {
    _hDialogControl_offline_write=GUI_CreateDialogBox(_aFrameWinControl_offline_write, GUI_COUNTOF(_aFrameWinControl_offline_write), &_cbFrameWinControl_offline_write, WM_HBKWIN, 0, 0);  
    h_timer=WM_CreateTimer(_hDialogControl_offline_write, 0, 100, 0);
  }
  else
  {
    _hDialogControl_main_menu=GUI_CreateDialogBox(_aFrameWinControl_main_menu, GUI_COUNTOF(_aFrameWinControl_main_menu), &_cbFrameWinControl_main_menu, WM_HBKWIN, 0, 0);
  }
  
  GUI_Exec();
  OSTaskResume((OS_TCB*)&Bargun_process_TaskTCB,&err);//恢复码枪任务允许码枪检测
  OSTmrStart(&tmr1,&err);	//开启定时器1
  
  // OSTmrStart(&tmr2,&err);	//开启定时器2
  
  while(1)
  {

    GUI_Exec();
    GUI_TOUCH_Exec();
    OSTimeDlyHMSM(0,0,0,20,OS_OPT_TIME_PERIODIC,&err);//20ms
  }
}

/*****************************************************
bargun扫码枪任务
1、USBH_Process函数调用时间不能抖动，如果抖动可能会引起丢包
2、在UCOS-III中以请求信号量的方式执行
3、在OS_TickTask任务中用OSTaskSemPost发送信号量
4、该任务不能以OSTimeDlyHMSM(0,0,0,1,OS_OPT_TIME_PERIODIC,&err);函数延时1ms的方式调度执行，这样调用时间会抖动？由于时间抖动导致丢包？
*****************************************************/
void bargun_process_task(void *pdata)
{
  OS_ERR err;
  //  OS_MSG_SIZE size;
  while(1)
  {
    
    //    OSTaskQPend(   (OS_TICK        )0,
    //                (OS_OPT         )OS_OPT_PEND_BLOCKING,
    //                (OS_MSG_SIZE   *)&size,
    //                (CPU_TS        *)0,
    //                (OS_ERR        *)&err);
    
    
    GPIO_ToggleBits(GPIOB, GPIO_Pin_8);
    USBH_Process(&USB_OTG_Core_dev, &USB_Host);
    OSTimeDlyHMSM(0,0,0,1,OS_OPT_TIME_PERIODIC,&err);//1ms
  }
}

//20250515扫码枪触发烧录启动任务
void bargun_start_source_task(void *pdata)
{
  u8 start_source_a;
  u8 start_source_b;
  OS_ERR err;
  OS_MSG_SIZE size;
  while(1)
  {
    OSTaskQPend(   (OS_TICK        )0,
                (OS_OPT         )OS_OPT_PEND_BLOCKING,
                (OS_MSG_SIZE   *)&size,
                (CPU_TS        *)0,
                (OS_ERR        *)&err);
    
    GPIO_ToggleBits(GPIOB, GPIO_Pin_11);
    if(has_window==1)
    {
      OSTaskResume((OS_TCB*)&Program_a_TaskTCB,&err);//恢复烧写任务A
      OSTaskResume((OS_TCB*)&Program_b_TaskTCB,&err);//恢复烧写任务B
      start_source_a=1;
      OSTaskQPost((OS_TCB    *)&Program_a_TaskTCB,
                  (void      *)&start_source_a,
                  (OS_MSG_SIZE)sizeof(start_source_a),
                  (OS_OPT     )OS_OPT_POST_FIFO,
                  (OS_ERR    *)&err);
      
      start_source_b=1;
      OSTaskQPost((OS_TCB    *)&Program_b_TaskTCB,
                  (void      *)&start_source_b,
                  (OS_MSG_SIZE)sizeof(start_source_b),
                  (OS_OPT     )OS_OPT_POST_FIFO,
                  (OS_ERR    *)&err);      
      
    }
  }
}

// 对话框控件定义表
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "同步计数确认", 0,        40,  100, 160, 80, FRAMEWIN_CF_DRAGGING },
  { TEXT_CreateIndirect,     "请确认已烧录数量！", GUI_ID_TEXT0,  10, 10, 180, 40, 0, 0x0, 0 },
  { BUTTON_CreateIndirect,   "确定",     GUI_ID_OK,     35, 30,  80, 30 },
};

// 回调处理
static void _cbDialog(WM_MESSAGE * pMsg) {
  OS_ERR err;
  int NCode, Id;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    return;
    
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    if (Id == GUI_ID_OK && NCode == WM_NOTIFICATION_RELEASED) {
      
      GUI_EndDialog(pMsg->hWin, 0);
      OSTaskQFlush((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//刷新清零计数
      OSTaskResume((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//恢复码枪任务
      SynCountTimes=0;//同步计数值清零
      
//      if(system_set_data_r.system_set_data_bit.bit_4==1)
//      {
//        WM_EnableWindow(h_pro_change0);//使能通道A切换
//      }
//      if(system_set_data_r.system_set_data_bit.bit_5==1)
//      {
//        WM_EnableWindow(h_pro_change);//使能通道B切换
//      }    
//      
//      WM_EnableWindow(h_pro_menu);//使能menu切换
    }
    return;
    
  default:
    WM_DefaultProc(pMsg);
  }
}


extern u8 bar_code[64];
//program_a的任务函数
void program_a_task(void *p_arg)
{	
  u8 *start_source;
  OS_MSG_SIZE size;
  OS_ERR err;
  //CPU_SR_ALLOC();
  
  while(1)
  {
    
    start_source = (uint8_t *)OSTaskQPend(   (OS_TICK        )0,
                                          (OS_OPT         )OS_OPT_PEND_BLOCKING,
                                          (OS_MSG_SIZE   *)&size,
                                          (CPU_TS        *)0,
                                          (OS_ERR        *)&err);
    
    //OSTaskSuspend((OS_TCB*)&Key_TaskTCB,&err);//挂起按键扫描任务，不再检测按键 
    OSTaskSuspend((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//挂起码枪任务，不再检测码枪
    
    
    
    
    sprintf(sp_a,"0:/system/dat/HEAD%03u.BIN",ImageNumber_store_a);
    if(!f_open(&fsrc, sp_a,  FA_READ))
    {
      f_read (&fsrc,buf_a,HEADER_LENGTH,&br);
      
      f_close (&fsrc);
      
      aes_decode(buf_a,HEADER_LENGTH);
      
      if(root_a)
        cJSON_Delete(root_a);
      root_a = cJSON_Parse((char const *)&buf_a[0]);
    }
    
    if(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWD") == 0)
    {
      WM_SelectWindow(_hDialogControl_offline_write);
      GUI_SetBkColor(0x787878);
      GUI_SetFont(&GUI_Fontsongti12);
      GUI_SetColor(GUI_WHITE);
      displayposition=DISPLAY_POSITION_Y;
      
      GUI_ClearRect(7, 185, 155, 292);//显示区域清屏幕
      
      
      ENABLE_PROGRESS_REFRESH;
      draw_progress_bar(mh);

      GUI_SetFont(&GUI_Fontlizhicaiyun);
      GUI_SetColor(GUI_BLUE);
      GUI_DispStringAt ("正在烧写",120, 79);      
      
      switch(*start_source)
      {
      case 0:
        SWD_HANDLE swd_a;
        
        swd_a.ImageNumber_store=ImageNumber_store_a;
        swd_a.root=root_a;
        
        swd_a.swdio_port=GPIOB;
        swd_a.swdio_pin=0;      
        
        swd_a.swclk_port=GPIOB;
        swd_a.swclk_pin=1;
        
        swd_a.sw_nreset_port=GPIOB;
        swd_a.sw_nreset_pin=2;      
        
        
        swd_a.code_bar_plug=0;
        
        
        ss=SWD_main(&swd_a);
        
        break;
        
      case 1://扫码枪触发
        GUI_SetColor(GUI_YELLOW);
        GUI_SetFont(&GUI_Fontsongti12);
        sprintf(sp_a,"A->SN:%s",bar_code);
        GUI_DispStringAt (sp_a,7, displayposition);
        displayposition+=15;
        
        
        for(u8 i=0;i<64;i++)
        {
          if(bar_code[i] ==  0)//遇到0
          {
            if(i==16)//符合16位长度
            {
              SWD_HANDLE swd_a;
              
              swd_a.ImageNumber_store=ImageNumber_store_a;
              swd_a.root=root_a;
              
              swd_a.swdio_port=GPIOB;
              swd_a.swdio_pin=0;      
              
              swd_a.swclk_port=GPIOB;
              swd_a.swclk_pin=1;
              
              swd_a.sw_nreset_port=GPIOB;
              swd_a.sw_nreset_pin=2;      
              
              swd_a.code_bar_plug=1;
              
              
              ss=SWD_main(&swd_a);
              break;//退出当前for循环
            }
            else//小于或者大于16位长度
            {
              GUI_DispStringAt ("不是16位码！",7, displayposition);
              displayposition+=15;
              ss=1;
              break;//退出当前for循环
            }
          }
        }
        
        
        
        break;
        
      }
      
      

      
      
      
      
      
      if(ss==0)
      {
        CurrentTimes+=1;
        SynCountTimes+=1;//同步计数值
        if(SynCountTimes==28)
        {
          GUI_CreateDialogBox(_aDialogCreate,
                              GUI_COUNTOF(_aDialogCreate),
                              _cbDialog,
                              WM_HBKWIN,
                              0, 0);
          OSTaskSuspend((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//挂起码枪任务，不再检测码枪
//          WM_DisableWindow(h_pro_change0);//禁用通道A切换
//          WM_DisableWindow(h_pro_change);//禁用通道B切换
//          WM_DisableWindow(h_pro_menu);//禁用menu切换
        }
  
        if(system_set_data_r.system_set_data_bit.bit_0==1)
        {      
          BUZZER_ON;
          
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);//?óê±500ms
          BUZZER_OFF;
        }
        
            
        GUI_SetFont(&GUI_Fontlizhicaiyun);
        GUI_SetColor(GUI_YELLOW);
        GUI_DispStringAt ("烧写成功",120, 79);
      }
      else
      {
        WM_SelectWindow(_hDialogControl_offline_write);
        GUI_SetBkColor(0x787878);
        GUI_SetFont(&GUI_Fontsongti12);
        GUI_SetColor(GUI_WHITE);
        GUI_DispStringAt ("A->烧写失败×",7, displayposition);
        if(system_set_data_r.system_set_data_bit.bit_0==1)
        {      
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
        }
        
        GUI_SetFont(&GUI_Fontlizhicaiyun);
        GUI_SetColor(GUI_RED);
        GUI_DispStringAt ("烧写失败",120, 79);
      }
    }
    else if((u8)strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWIM") == 0)
    {
      WM_SelectWindow(_hDialogControl_offline_write);
      GUI_SetBkColor(0x787878);
      GUI_SetFont(&GUI_Fontsongti12);
      GUI_SetColor(GUI_WHITE);
      displayposition=DISPLAY_POSITION_Y;
      
      GUI_ClearRect(7, 185, 155, 300);//显示区域清屏幕
      
      switch(*start_source)
      {
      case 0:
        break;
        
      case 1:
        sprintf(sp_a,"A->SN:%s",bar_code);
        displayposition=DISPLAY_POSITION_Y;
        GUI_DispStringAt (sp_a,7, displayposition);
        displayposition+=15;
        break;
      }
      
      
      SWIM_HANDLE swim_a;
      
      swim_a.ImageNumber_store=ImageNumber_store_a;
      swim_a.root=root_a;
      
      swim_a.syncswpwm_in_timer_rise_dma_init=SYNCSWPWM_IN_TIMER_RISE_DMA_INIT_A;
      swim_a.syncswpwm_out_timer_dma_init=SYNCSWPWM_OUT_TIMER_DMA_INIT_A;
      swim_a.syncswpwm_in_timer_rise_dma_wait=SYNCSWPWM_IN_TIMER_RISE_DMA_WAIT_A;
      swim_a.syncswpwm_out_timer_dma_wait=SYNCSWPWM_OUT_TIMER_DMA_WAIT_A;
      swim_a.syncswpwm_out_timer_setcycle=SYNCSWPWM_OUT_TIMER_SetCycle_A;
      swim_a.swim_gpio_init=SWIM_GPIO_Init_SWIM_A;
      swim_a.swim_init=SWIM_Init_A;
      swim_a.swim_deinit=SWIM_DeInit_A;
      swim_a.swim_enable_clock_input=SWIM_EnableClockInput_A;
      
      swim_a.swim_port=GPIOB;
      swim_a.swim_pin=GPIO_Pin_0;
      
      swim_a.swim_rst_port=GPIOB;
      swim_a.swim_rst_pin=GPIO_Pin_1;
      
      
      swim_a.swim_gpio_init(&swim_a);
      
      
      flash_control_reg_address=0;
      u8  ss=0;
      flash_control_reg_address=(u8)cJSON_GetObjectItem(root_a, "flash_reg_address")->valueint;
      ss=Pro_Init_SWIM(&swim_a);//          
      if((u8)cJSON_GetObjectItem(root_a, "mass_erase")->valueint == 1)
      {
        ss=Write_OpitonByte_STM8_pre_SWIM(&swim_a);
        if(flash_control_reg_address==1)//如果是STM8L系列就要清空一次芯片。
        {
          ss=ClearChip_SWIM(&swim_a);
        }
        
        if(ss==0)
        {
          
          GUI_DispStringAt ("A->清芯片成功√",7, displayposition);
          
          WM_Exec();
          displayposition+=15;
          
          
        }
      }
      
      if(ss==0)//上一步必须为0才说明有芯片才能写flash
      {
        if((u8)cJSON_GetObjectItem(root_a, "write_flash")->valueint == 1)
        {
          ss=ProgramFlash_SWIM(&swim_a);//
          if(ss==0)
          {
            // if(language_store==0x00000005)                   
            GUI_DispStringAt ("A->写FLASH成功√",7, displayposition);
            // else
            //  GUI_DispStringAt ("program flash ok√",7, displayposition);                
            WM_Exec();
            displayposition+=15;
          }
          else
          {
            GUI_DispStringAt ("A->写失败",7, displayposition);
            WM_Exec();
            displayposition+=15;
          }
        }
      }
      
      
      
      if((u8)cJSON_GetObjectItem(root_a, "write_eeprom")->valueint == 1)
      {
        ss=ProgramEEPROM_SWIM(&swim_a);//
        if(ss==0)
        {
          //  if(language_store==0x00000005)                 
          GUI_DispStringAt ("A->写EEPROM成功√",7, displayposition);
          //   else
          //   GUI_DispStringAt ("program eeprom ok√",7, displayposition);               
          displayposition+=15;
          WM_Exec();
        }
      }
      
      
      
      if((u8)cJSON_GetObjectItem(root_a, "write_option_byte")->valueint == 1)
      {
        ss=Write_OpitonByte_STM8_SWIM(&swim_a);
        if(ss==0)
        {
          //            if(language_store==0x00000005)     
          GUI_DispStringAt ("A->写选项字成功√",7, displayposition);
          //   else
          //      GUI_DispStringAt ("program option byte ok√",7, displayposition);
          
          WM_Exec();
          displayposition+=15;
        }
      }
      
      
      ss=End_Pro_SWIM(&swim_a);            
      
      // OSIntExit(); 
      
      if(ss==0)
      {
        BUZZER_ON;
        
        OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);//?óê±500ms
        BUZZER_OFF;
      }
      else
      {
        if(system_set_data_r.system_set_data_bit.bit_0==1)
        {      
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
        }
      }
    }
    else if((u8)strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "ICP") == 0)
    {
#include "icp.h"
      N76E003_TEST();
    }
    
   
    
    
    
    
    
    OSTaskQFlush((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//刷新清零计数
    OSTaskResume((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//恢复码枪任务
    //OSTaskResume((OS_TCB*)&Key_TaskTCB,&err);//恢复按键扫描任务允许按键检测
    OSTaskSuspend((OS_TCB*)&Program_a_TaskTCB,&err);//挂起烧写任务 
    
  }
}

//program_b的任务函数
void program_b_task(void *p_arg)
{	
  u8 *start_source;
  OS_MSG_SIZE size;
  OS_ERR err;
  //CPU_SR_ALLOC();
  
  while(1)
  {
    
    start_source = (uint8_t *)OSTaskQPend(   (OS_TICK        )0,
                                          (OS_OPT         )OS_OPT_PEND_BLOCKING,
                                          (OS_MSG_SIZE   *)&size,
                                          (CPU_TS        *)0,
                                          (OS_ERR        *)&err);
    
    //OSTaskSuspend((OS_TCB*)&Key_TaskTCB,&err);//挂起按键扫描任务，不再检测按键 
    OSTaskSuspend((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//挂起码枪任务，不再检测码枪
    
    
    
    
    sprintf(sp_b,"0:/system/dat/HEAD%03u.BIN",ImageNumber_store_b);
    if(!f_open(&fsrc, sp_b,  FA_READ))
    {
      f_read (&fsrc,buf_b,HEADER_LENGTH,&br);
      
      f_close (&fsrc);
      
      aes_decode(buf_b,HEADER_LENGTH);
      
      if(root_b)
        cJSON_Delete(root_b);
      root_b = cJSON_Parse((char const *)&buf_b[0]);
    }
    
    if(strcmp(cJSON_GetObjectItem(root_b, "port_type")->valuestring, "SWD") == 0)
    {
      WM_SelectWindow(_hDialogControl_offline_write);
      GUI_SetBkColor(0x787878);
      GUI_SetFont(&GUI_Fontsongti12);
      GUI_SetColor(GUI_WHITE);
      displayposition=DISPLAY_POSITION_Y;
      
      GUI_ClearRect(7, 185, 155, 300);//显示区域清屏幕
      
      switch(*start_source)
      {
      case 0:
        SWD_HANDLE swd_b;
        
        
        
        swd_b.ImageNumber_store=ImageNumber_store_b;
        swd_b.root=root_b;
        
        swd_b.swdio_port=GPIOA;
        swd_b.swdio_pin=2;      
        
        swd_b.swclk_port=GPIOA;
        swd_b.swclk_pin=3;
        
        swd_b.sw_nreset_port=GPIOC;
        swd_b.sw_nreset_pin=5;      
        
        ss=SWD_main(&swd_b); 
        
        break;
        
      case 1:
        sprintf(sp_b,"B->SN:%s",bar_code);
        GUI_DispStringAt (sp_b,7, displayposition);
        displayposition+=15;
        
        
        for(u8 i=0;i<64;i++)
        {
          if(bar_code[i] ==  0)//遇到0
          {
            if(i==16)//符合16位长度
            {
              SWD_HANDLE swd_b;
              
              
              
              swd_b.ImageNumber_store=ImageNumber_store_b;
              swd_b.root=root_b;
              
              swd_b.swdio_port=GPIOA;
              swd_b.swdio_pin=2;      
              
              swd_b.swclk_port=GPIOA;
              swd_b.swclk_pin=3;
              
              swd_b.sw_nreset_port=GPIOC;
              swd_b.sw_nreset_pin=5;      
              
              ss=SWD_main(&swd_b);
              break;//退出当前for循环             
            }
            else//小于或者大于16位长度
            {
              GUI_DispStringAt ("不是16位码！",7, displayposition);
              displayposition+=15;
              ss=1;
              break;//退出当前for循环              
            }
          }
        }        
        
        
        
        break;
        
      }
      
      
      
      
      
      if(ss==0)
      {
        CurrentTimes+=1;
        if(system_set_data_r.system_set_data_bit.bit_0==1)
        {      
          BUZZER_ON;
          
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);//?óê±500ms
          BUZZER_OFF;
        }
      }
      else
      {
        WM_SelectWindow(_hDialogControl_offline_write);
        GUI_SetBkColor(0x787878);
        GUI_SetFont(&GUI_Fontsongti12);
        GUI_SetColor(GUI_WHITE);
        GUI_DispStringAt ("B->烧写失败×",7, displayposition);
        if(system_set_data_r.system_set_data_bit.bit_0==1)
        {      
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
        }
      }
    }
    else if((u8)strcmp(cJSON_GetObjectItem(root_b, "port_type")->valuestring, "SWIM") == 0)
    {
      WM_SelectWindow(_hDialogControl_offline_write);
      GUI_SetBkColor(0x787878);
      GUI_SetFont(&GUI_Fontsongti12);
      GUI_SetColor(GUI_WHITE);
      displayposition=DISPLAY_POSITION_Y;
      
      GUI_ClearRect(7, 185, 155, 300);//显示区域清屏幕
      
      switch(*start_source)
      {
      case 0:
        break;
        
      case 1:
        sprintf(sp_b,"B->SN:%s",bar_code);
        displayposition=DISPLAY_POSITION_Y;
        GUI_DispStringAt (sp_b,7, displayposition);
        displayposition+=15;
        break;
        
      }
      
      
      SWIM_HANDLE swim_b;
      
      swim_b.ImageNumber_store=ImageNumber_store_b;
      swim_b.root=root_b;
      
      swim_b.syncswpwm_in_timer_rise_dma_init=SYNCSWPWM_IN_TIMER_RISE_DMA_INIT_B;
      swim_b.syncswpwm_out_timer_dma_init=SYNCSWPWM_OUT_TIMER_DMA_INIT_B;
      swim_b.syncswpwm_in_timer_rise_dma_wait=SYNCSWPWM_IN_TIMER_RISE_DMA_WAIT_B;
      swim_b.syncswpwm_out_timer_dma_wait=SYNCSWPWM_OUT_TIMER_DMA_WAIT_B;
      swim_b.syncswpwm_out_timer_setcycle=SYNCSWPWM_OUT_TIMER_SetCycle_B;
      swim_b.swim_gpio_init=SWIM_GPIO_Init_SWIM_B;
      swim_b.swim_init=SWIM_Init_B;
      swim_b.swim_deinit=SWIM_DeInit_B;
      swim_b.swim_enable_clock_input=SWIM_EnableClockInput_B;      
      
      swim_b.swim_port=GPIOA;
      swim_b.swim_pin=GPIO_Pin_2;
      
      swim_b.swim_rst_port=GPIOA;
      swim_b.swim_rst_pin=GPIO_Pin_3;
      
      
      swim_b.swim_gpio_init(&swim_b);
      flash_control_reg_address=0;
      u8 ss=0;
      flash_control_reg_address=(u8)cJSON_GetObjectItem(root_b, "flash_reg_address")->valueint;
      ss=Pro_Init_SWIM(&swim_b);//
      if((u8)cJSON_GetObjectItem(root_b, "mass_erase")->valueint == 1)
      {
        ss=Write_OpitonByte_STM8_pre_SWIM(&swim_b);;
        if(flash_control_reg_address==1)//如果是STM8L系列就要清空一次芯片。
        {
          ss=ClearChip_SWIM(&swim_b);
        }
        
        if(ss==0)
        {
          GUI_DispStringAt ("B->清芯片成功√",7, displayposition);
          
          WM_Exec();
          displayposition+=15;
        }
      }
      
      if(ss==0)//上一步必须为0才说明有芯片才能写flash
      {
        if((u8)cJSON_GetObjectItem(root_b, "write_flash")->valueint == 1)
        {
          ss=ProgramFlash_SWIM(&swim_b);//
          if(ss==0)
          {
            // if(language_store==0x00000005)                   
            GUI_DispStringAt ("B->写FLASH成功√",7, displayposition);
            // else
            //  GUI_DispStringAt ("program flash ok√",7, displayposition);                
            WM_Exec();
            displayposition+=15;
          }
          else
          {
            GUI_DispStringAt ("B->写失败",7, displayposition);
            WM_Exec();
            displayposition+=15;
          }
        }
      }
      
      
      
      if((u8)cJSON_GetObjectItem(root_b, "write_eeprom")->valueint == 1)
      {
        ss=ProgramEEPROM_SWIM(&swim_b);;//
        if(ss==0)
        {
          //  if(language_store==0x00000005)                 
          GUI_DispStringAt ("B->写EEPROM成功√",7, displayposition);
          //   else
          //   GUI_DispStringAt ("program eeprom ok√",7, displayposition);               
          displayposition+=15;
          WM_Exec();
        }
      }
      
      
      
      if((u8)cJSON_GetObjectItem(root_b, "write_option_byte")->valueint == 1)
      {
        ss=Write_OpitonByte_STM8_SWIM(&swim_b);;
        if(ss==0)
        {
          //            if(language_store==0x00000005)     
          GUI_DispStringAt ("B->写选项字成功√",7, displayposition);
          //   else
          //      GUI_DispStringAt ("program option byte ok√",7, displayposition);
          
          WM_Exec();
          displayposition+=15;
        }
      }
      
      
      ss=End_Pro_SWIM(&swim_b);;            
      
      if(ss==0)
      {
        BUZZER_ON;
        
        OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);//?óê±500ms
        BUZZER_OFF;
      }
      else
      {
        if(system_set_data_r.system_set_data_bit.bit_0==1)
        {      
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_ON;
          OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_PERIODIC,&err);   //延时50ms
          BUZZER_OFF;
        }
      }
    }
    OSTaskQFlush((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//刷新清零计数
    OSTaskResume((OS_TCB*)&Bargun_start_source_TaskTCB,&err);//恢复码枪任务
    //OSTaskResume((OS_TCB*)&Key_TaskTCB,&err);//恢复按键扫描任务允许按键检测
    OSTaskSuspend((OS_TCB*)&Program_b_TaskTCB,&err);//挂起烧写任务 
    
  }
}


//定时器1的回调函数
void tmr1_callback(void *p_tmr, void *p_arg)
{
  time_data_r.time_data_bit.milli++;
  
  if(time_data_r.time_data_bit.milli==10)
  {
    time_data_r.time_data_bit.milli=0;
    time_data_r.time_data_bit.seconds++;
    if(time_data_r.time_data_bit.seconds==60)
    {
      time_data_r.time_data_bit.seconds=0;
      time_data_r.time_data_bit.minutes++;
      if(time_data_r.time_data_bit.minutes==60)
      {
        time_data_r.time_data_bit.minutes=0;
        time_data_r.time_data_bit.hours++;
        if(time_data_r.time_data_bit.hours==9999)
        {
          time_data_r.time_data_bit.hours=0;
        }
      }
    }
  }
  
  if(EnableUsbCreateFile==0)//不允许创建文件的情况下，说明数据已经到来
  {
    transfertimes+=1;
    
    if(transfertimes>=5)//5*10=50毫秒无数据认为数据完成
    {
      transfertimes=0;//清零
      
      
      f_close(&f_name);//结束本次文件写
      
      
      
      
      EnableUsbCreateFile=1;//使能允许下次创建文件
      UsbTransferComplete=1;//置位发送完成标志，蜂鸣器提示音
    }
  }
  
  if(UsbTransferComplete==1)
  {
    if(system_set_data_r.system_set_data_bit.bit_0==1)//使能蜂鸣器提示音？
      BUZZER_ON;
    
    
    UsbTransferCompleteCount++;
    if(UsbTransferCompleteCount>=5)
    {
      UsbTransferCompleteCount=0;
      UsbTransferComplete=0;
      BUZZER_OFF;
    }
  }
  voltage_caculate();
}


void Get_Compile_Time(uint8_t *Year, uint8_t *Month, uint8_t *Day,uint8_t *hh,uint8_t *mm,uint8_t *ss)
{
  
  const char *pMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  
  const char Date[12] = __DATE__;//取编译日期
  const char Time[16] = __TIME__;//取编译时间
  uint8_t i;
  
  
  
  for(i = 0; i < 12; i++)if(memcmp(Date, pMonth[i], 3) == 0)*Month = i + 1, i = 12;
  
  *Year = (uint8_t)atoi(Date + 9); //Date[9]为２位年份，Date[7]为完整年份
  
  *Day = (uint8_t)atoi(Date + 4);
  
  *hh = (uint8_t)atoi(Time + 0);
  *mm = (uint8_t)atoi(Time + 3);
  *ss = (uint8_t)atoi(Time + 6);
}



void drawline(void)
{
  GUI_SetColor(GUI_YELLOW);
  GUI_DrawHLine(20,3,240-3);
  
  GUI_DrawHLine(36,3,119);
  
  GUI_DrawHLine(65,3,240-3);
  
  GUI_DrawHLine(106,3,237);//???闂囧懘妲??閹??????鐫????缁????閹叉鏆?閻熺瓔鎼稿?閿?閻犲﹥鏋?閻︽粠鐓?妫版彃鍙?閹??閾︻亣鍦虹潰?
  
  GUI_DrawHLine(121,3,237);//閾?閾?椤?閻???缁??濠??閳???
  
  GUI_DrawHLine(136,3,237);//閾?閾?椤?閻?閻ュ秶妲???濠??閳???
  
  
  GUI_DrawHLine(151,3,237);//閾?閾?椤?閻㈠牜鎮樼划璇″祤閻???椤?閻??
  
  GUI_DrawHLine(166,3,237);//閾?閾?椤?閻???妞??濠??閳???
  
  GUI_DrawHLine(181,3,237);//閾?閾?椤?閻㈠牜娈介幓鎰暟???濠??閳???
  
  
  GUI_DrawVLine(80,106,181);//閻???閹??娴??閻?閾?鐫㈠繒鐎?閸??椤?閻?閻繘顦??椤嫮妫?椤???鐫㈡垶鎷?閹?
  
  GUI_DrawVLine(157,106,181);//閻???閹??娴??閻?閾?鐫㈠繒鐎?閸??椤???閺佽鐓?閾???椤ㄑ嗕絾??閹绢厽鍠涢　???
  
  GUI_DrawVLine(119,20,65);//閻???鐠??閹锋瑦鎷?椤?閾?鐫㈠繒鐎?閸?????椤?閸忓憡鍩夐惉鍫氬煐?閻︽粠鐓涚挧??閹绢厽鍠涢　???
  
  GUI_DrawVLine(164,181,305);//????娴??閹海妫?????鐫??????????閸??妞???椤?
  
}


void draw_select(void)
{
  if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWD") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWIM") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "UART") == 0))
  {
    
    if(cJSON_GetObjectItem(root_a, "mass_erase")->valueint)
    {
      GUI_DispStringAt("√",67,124);
    }
    else
    {
      GUI_DispStringAt("×",67,124);
    }
    
    
    if(cJSON_GetObjectItem(root_a, "page_erase")->valueint)
    {
      GUI_DispStringAt("√",67,140);
    }
    else
    {
      GUI_DispStringAt("×",67,140);
    }
    
    
    if(cJSON_GetObjectItem(root_a, "write_flash")->valueint)
    {
      GUI_DispStringAt("√",67,154);
    }
    else
    {
      GUI_DispStringAt("×",67,154);
    }
    
    
    if(cJSON_GetObjectItem(root_a, "verify_flash")->valueint)
    {
      GUI_DispStringAt("√",67,170);
    }
    else
    {
      GUI_DispStringAt("×",67,170);
    }
    
    
    switch(cJSON_GetObjectItem(root_a, "write_eeprom")->valueint)
    {
    case 0:
      GUI_DispStringAt("×",145,108);
      break;
      
    case 1:
      GUI_DispStringAt("√",145,108);
      break;
      
    default:
      GUI_DispStringAt(" ",145,108);
      break;
    }
    
    
    switch(cJSON_GetObjectItem(root_a, "verify_eeprom")->valueint)
    {
    case 0:
      GUI_DispStringAt("×",145,124);
      break;
      
    case 1:
      GUI_DispStringAt("√",145,124);
      break;
      
    default:
      GUI_DispStringAt(" ",145,124);
      break;
    }
    
    
    
    switch(cJSON_GetObjectItem(root_a, "write_otp")->valueint)
    {
    case 0:
      GUI_DispStringAt("×",145,140);
      break;
      
    case 1:
      GUI_DispStringAt("√",145,140);
      break;
      
    default:
      GUI_DispStringAt(" ",145,140);
      break;
    }
    
    
    switch(cJSON_GetObjectItem(root_a, "verify_otp")->valueint)
    {
    case 0:
      GUI_DispStringAt("×",145,156);
      break;
      
    case 1:
      GUI_DispStringAt("√",145,156);
      break;
      
    default:
      GUI_DispStringAt(" ",145,156);
      break;
    }
    
    
    
    
    if(cJSON_GetObjectItem(root_a, "write_rolling_code")->valueint)//
    {
      GUI_DispStringAt("√",225,108);
    }
    else
    {
      GUI_DispStringAt("×",225,108);
    }
    
    
    switch(cJSON_GetObjectItem(root_a, "custom_secret")->valueint)
    {
    case 0:
      GUI_DispStringAt("×",145,170);
      break;
      
    case 1:
      GUI_DispStringAt("√",145,170);
      break;
      
    default:
      GUI_DispStringAt(" ",145,170);
      break;
    }
    
    
    
    switch(cJSON_GetObjectItem(root_a, "write_option_byte")->valueint)
    {
    case 0:
      GUI_DispStringAt("×",225,124);
      break;
      
    case 1:
      GUI_DispStringAt("√",225,124);
      break;
      
    default:
      GUI_DispStringAt(" ",225,124);
      break;
    }
    
    
    
    if(cJSON_GetObjectItem(root_a, "reset_type")->valueint==2)//
    {
      GUI_DispStringAt("×",225,140);
    }
    else
    {
      GUI_DispStringAt("√",225,140);
    }
    
    
    switch(cJSON_GetObjectItem(root_a, "jump")->valueint)
    {
    case 0:
      GUI_DispStringAt("×",225,154);
      GUI_DispStringAt("--", 217, 168);//
      break;
      
    case 1:
      GUI_DispStringAt("√",225,154);
      
      GUI_DispDecAt(cJSON_GetObjectItem(root_a, "jump_number")->valueint, 215, 168, 3);//
      break;
      
    default:
      GUI_DispStringAt(" ",225,154);
      GUI_DispStringAt("  ", 217, 168);//
      break;
    }
  }
  else if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "ICP") == 0))
  {
    
    if(cJSON_GetObjectItem(root, "mass_erase")->valueint)
    {
      GUI_DispStringAt("√",67,124);
    }
    else
    {
      GUI_DispStringAt("×",67,124);
    }
    
    
    
    
    if(cJSON_GetObjectItem(root_a, "write_aprom")->valueint)
    {
      GUI_DispStringAt("√",67,154);
    }
    else
    {
      GUI_DispStringAt("×",67,154);
    }
    
    
    if(cJSON_GetObjectItem(root_a, "verify_aprom")->valueint)
    {
      GUI_DispStringAt("√",67,170);
    }
    else
    {
      GUI_DispStringAt("×",67,170);
    }
    
    if(cJSON_GetObjectItem(root_a, "write_ldrom")->valueint)//?????閿涘啳婢?PROM
    {
      GUI_DispStringAt("√",145,108);
    }
    else
    {
      GUI_DispStringAt("×",145,108);
    }
    
    if(cJSON_GetObjectItem(root_a, "verify_ldrom")->valueint)//???閾?闁瑧鐎??閹?椤湧ROM
    {
      GUI_DispStringAt("√",145,124);
    }
    else
    {
      GUI_DispStringAt("×",145,124);
    }
    
    if(cJSON_GetObjectItem(root_a, "write_sprom")->valueint)// 
    {
      GUI_DispStringAt("√",145,140);
    }
    else
    {
      GUI_DispStringAt("×",145,140);
    }
    
    if(cJSON_GetObjectItem(root_a, "verify_sprom")->valueint)//
    {
      GUI_DispStringAt("√",145,156);
    }
    else
    {
      GUI_DispStringAt("×",145,156);
    }
    
    if(cJSON_GetObjectItem(root_a, "write_rolling_code")->valueint)//
    {
      GUI_DispStringAt("√",225,108);
    }
    else
    {
      GUI_DispStringAt("×",225,108);
    }
    
    
    
    if(cJSON_GetObjectItem(root_a, "write_config")->valueint)//
    {
      GUI_DispStringAt("√",225,124);
    }
    else
    {
      GUI_DispStringAt("×",225,124);
    }
    
    if(cJSON_GetObjectItem(root_a, "reset_type")->valueint==2)//
    {
      GUI_DispStringAt("×",225,140);
    }
    else
    {
      GUI_DispStringAt("√",225,140);
    }
    
    if(cJSON_GetObjectItem(root_a, "jump")->valueint)//
    {
      GUI_DispStringAt("√",225,154);
      
      GUI_DispDecAt(cJSON_GetObjectItem(root_a, "jump_number")->valueint, 215, 168, 3);//
    }
    else
    {
      GUI_DispStringAt("×",225,154);
      GUI_DispStringAt("--", 217, 168);//
    }
    
  }
  else if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SPI1") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SPI2") == 0))
  {
    if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "FLASH") == 0)
    {
      if(cJSON_GetObjectItem(root_a, "mass_erase")->valueint==1)
      {
        GUI_DispStringAt("√",67,124);
      }
      else
      {
        GUI_DispStringAt("×",67,124);
      }
      
      
      //    if(cJSON_GetObjectItem(root, "page_erase")->valueint)
      //    {
      //      GUI_DispStringAt("√",67,140);
      //    }
      //    else
      //    {
      //      GUI_DispStringAt("×",67,140);
      //    }
      
      if(cJSON_GetObjectItem(root_a, "write_flash")->valueint==1)
      {
        GUI_DispStringAt("√",67,154);
      }
      else
      {
        GUI_DispStringAt("×",67,154);
      }
      
      
      if(cJSON_GetObjectItem(root_a, "verify_flash")->valueint==1)
      {
        GUI_DispStringAt("√",67,170);
      }
      else
      {
        GUI_DispStringAt("×",67,170);
      }
      
      
      if(cJSON_GetObjectItem(root_a, "write_rolling_code")->valueint==1)//
      {
        GUI_DispStringAt("√",225,108);
      }
      else
      {
        GUI_DispStringAt("×",225,108);
      }      
    }
    else if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
    {
      if(cJSON_GetObjectItem(root_a, "mass_erase")->valueint==1)
      {
        GUI_DispStringAt("√",67,124);
      }
      else
      {
        GUI_DispStringAt("×",67,124);
      }
      switch(cJSON_GetObjectItem(root_a, "write_eeprom")->valueint)
      {
      case 0:
        GUI_DispStringAt("×",145,108);
        break;
        
      case 1:
        GUI_DispStringAt("√",145,108);
        break;
        
      default:
        GUI_DispStringAt(" ",145,108);
        break;
      }
      
      
      switch(cJSON_GetObjectItem(root_a, "verify_eeprom")->valueint)
      {
      case 0:
        GUI_DispStringAt("×",145,124);
        break;
        
      case 1:
        GUI_DispStringAt("√",145,124);
        break;
        
      default:
        GUI_DispStringAt(" ",145,124);
        break;
      }
      
      if(cJSON_GetObjectItem(root_a, "write_rolling_code")->valueint==1)//
      {
        GUI_DispStringAt("√",225,108);
      }
      else
      {
        GUI_DispStringAt("×",225,108);
      }      
    }
    
    
  }
}




#define WM_UPDATE      (WM_USER + 0x00) /* 自定义消息 */  // --------------（2）
void _cbFrameWinControl_offline_write(WM_MESSAGE * pMsg) {
  OS_ERR err;
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  static u8 state_t=WM_INIT_DIALOG;
  CPU_SR_ALLOC();
  
  switch (pMsg->MsgId) {
    
  case WM_UPDATE:
    state_t=WM_INIT_DIALOG;
    WM_InvalidateWindow(pMsg->hWin);
    break;
    
  case WM_CREATE:
    
    break;
  case WM_KEY:
    WM_SendMessage(WM_HBKWIN, pMsg);
    break;
  case WM_INIT_DIALOG:
    
    CPU_CRITICAL_ENTER();//进入临界区
    has_window=1;
    
    system_set_data_r.system_set_data_all=*(u32*)SYSTEM_SET_DATA_ADDRESS_R;
    ImageNumber_store_a=*(u16*)(PARAMETER_ADDRESS_R + OFFSET_IMAGE_NUMBER_A);
    ImageNumber_store_a=(ImageNumber_store_a==0xFFFF)?0:ImageNumber_store_a;
    
    ImageNumber_store_b=*(u16*)(PARAMETER_ADDRESS_R + OFFSET_IMAGE_NUMBER_B);
    ImageNumber_store_b=(ImageNumber_store_b==0xFFFF)?0:ImageNumber_store_b;
    
    h_pro_change0=hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHANGE0);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    
    if(system_set_data_r.system_set_data_bit.bit_4==0)
    {
      WM_DisableWindow(hItem);
    }
    else
    {
      WM_EnableWindow(hItem);
    }
    
    
    h_pro_change=hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHANGE);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    if(system_set_data_r.system_set_data_bit.bit_5==0)
    {
      WM_DisableWindow(hItem);
    }
    else
    {
      WM_EnableWindow(hItem);
    }    
    
    
    h_pro_menu=hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_MENU);
    
    
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_MENU);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    
    
    
    for(u8 i=0;i<16;i++)
    {
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
      
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      TEXT_SetText(hItem,   pro_item_0[i]);
    }
    
    
    for(u8 i=0;i<9;i++)
    {
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT15+i);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      
    }
    
    for(u8 i=0;i<3;i++)
    {
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT25+i);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      
    }
    
    
    h_pro_time = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT15);
    
    
    
    h_pro_cpu_use = WM_GetDialogItem(pMsg->hWin, GUI_ID_CPU_USE);
    TEXT_SetFont(h_pro_cpu_use, &GUI_Fontsongti12);
    TEXT_SetTextColor(h_pro_cpu_use, GUI_WHITE);
    
    
    
    
    h_pro_CurrentTimes=hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT24);
    TEXT_SetFont(hItem, GUI_FONT_24B_1);
    TEXT_SetTextColor(hItem, GUI_WHITE);
    sprintf(sp_a,"%07u",CurrentTimes);//本次烧录
    TEXT_SetText(hItem, sp_a);
    
    //同步
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_SYN);
    TEXT_SetFont(hItem, &GUI_Fontsongti12);
    TEXT_SetTextColor(hItem, GUI_WHITE);
    //计数
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_COUNT);
    TEXT_SetFont(hItem, &GUI_Fontsongti12);
    TEXT_SetTextColor(hItem, GUI_WHITE);
    
    
    //同步计数值
    h_pro_syn_count=hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_COUNT_NUM);
    TEXT_SetFont(hItem, GUI_FONT_32B_1);   
    TEXT_SetTextColor(hItem, GUI_YELLOW);
    sprintf(sp_a,"%u/28",SynCountTimes);
    TEXT_SetText(hItem, sp_a);
    
    
    
    h_pro_state=hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_PROGRAM_STATE);
    TEXT_SetFont(hItem, &GUI_Fontlizhicaiyun);
    TEXT_SetTextColor(hItem, GUI_WHITE);
    
    
    sprintf(sp_a,"0:/system/dat/HEAD%03u.BIN",ImageNumber_store_a);
    
    if(!f_open(&fsrc, sp_a,  FA_READ))
    {
      f_read (&fsrc,buf_a,HEADER_LENGTH,&br);
      
      f_close (&fsrc);
      
      aes_decode(buf_a,HEADER_LENGTH);
      
      if(root_a)
        cJSON_Delete(root_a);
      
      root_a = cJSON_Parse((char const *)&buf_a[0]);
      
      
      //      sprintf(sp1,"芯片:%s",cJSON_GetObjectItem(root, "chip_type")->valuestring);
      //      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT16);
      //      
      //      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      //      TEXT_SetTextColor(hItem, GUI_WHITE);
      //      TEXT_SetText(hItem, sp1);
      
      
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT18);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      sprintf(sp_a,"接口:%s",cJSON_GetObjectItem(root_a, "port_type")->valuestring);
      TEXT_SetText(hItem, sp_a);    
      
      
      
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT19);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      sprintf(sp_a,"镜像:%s",cJSON_GetObjectItem(root_a, "note")->valuestring);
      TEXT_SetText(hItem, sp_a);    
      
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT20);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      sprintf(sp_a,"镜像号:%03u",ImageNumber_store_a);
      TEXT_SetText(hItem, sp_a);          
      
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT21);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
      {
        sprintf(sp_a,"checksum:0x%08X",(u32)cJSON_GetObjectItem(root_a, "eeprom_checksum")->valuedouble);
      }
      else
      {
        sprintf(sp_a,"checksum:0x%08X",(u32)cJSON_GetObjectItem(root_a, "flash_checksum")->valuedouble);
      }
      
      TEXT_SetText(hItem, sp_a);
      
      
      
      
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT25);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      sprintf(sp_a,"已用次数:%010u",(u32)cJSON_GetObjectItem(root_a, "times_used")->valuedouble);
      TEXT_SetText(hItem, sp_a);          
      
      
      totaltimes=(u32)(cJSON_GetObjectItem(root_a, "times")->valuedouble);//88888888为不限制次数
      
      hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT26);
      TEXT_SetFont(hItem, &GUI_Fontsongti12);
      TEXT_SetTextColor(hItem, GUI_WHITE);
      sprintf(sp_a,"剩余次数:%010u",totaltimes);
      TEXT_SetText(hItem, sp_a);   
      
      
      
      
      if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWD") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWIM") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "UART") == 0))
      {
        for(u8 i=0;i<15;i++)
        {
          hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
          
          TEXT_SetFont(hItem, &GUI_Fontsongti12);
          TEXT_SetTextColor(hItem, GUI_WHITE);
          TEXT_SetText(hItem,   pro_item_1[i]);
        }
      }
      else if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "ICP") == 0))
      {
        for(u8 i=0;i<15;i++)
        {
          hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
          
          TEXT_SetFont(hItem, &GUI_Fontsongti12);
          TEXT_SetTextColor(hItem, GUI_WHITE);
          TEXT_SetText(hItem,   pro_item_2[i]);
        }
      }      
      else if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SPI1") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SPI2") == 0))
      {
        if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "FLASH") == 0)
        {      
          for(u8 i=0;i<15;i++)
          {
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
            
            TEXT_SetFont(hItem, &GUI_Fontsongti12);
            TEXT_SetTextColor(hItem, GUI_WHITE);
            TEXT_SetText(hItem,   pro_item_3[i]);
          }
        }
        else if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
        {      
          for(u8 i=0;i<15;i++)
          {
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
            
            TEXT_SetFont(hItem, &GUI_Fontsongti12);
            TEXT_SetTextColor(hItem, GUI_WHITE);
            TEXT_SetText(hItem,   pro_item_4[i]);
          }
        } 
      }
      
      //      out_voltage_select();
      //      
      //      
      //      
      //      
      //      
      //      
      //      if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "SWIM") == 0)
      //      {
      //        SWIM_GPIO_Init();
      //      }
      //      else if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "SWD") == 0)
      //      {
      //        //SWIM_GPIO_Init();
      //      }
      //      else if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "UART") == 0)
      //      {
      //        USART3_Config();
      //      }
      //      else if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "IIC") == 0)
      //      {
      //        systick_init(72);	
      //        stm32f1xx_i2c_init();	/*i2c init*/
      //      }
      
      
    }


    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_FIRMWARE_TIME);
    
    TEXT_SetFont(hItem, &GUI_Fontsongti12);
    TEXT_SetTextColor(hItem, GUI_LIGHTRED);
    sprintf(sp1, "固件版本:%02u%02u%02u%02u%02u%02u", firmware_time.Year, firmware_time.Month, firmware_time.Day, firmware_time.hh, firmware_time.mm, firmware_time.ss);//任意格式化
    TEXT_SetText(hItem,   sp1);
    
    
    
    
    
    
    
    state_t=WM_INIT_DIALOG;
    
    // OS_CRITICAL_EXIT();
    CPU_CRITICAL_EXIT();
    
    break;
  case WM_PAINT:
    
    //OS_CRITICAL_ENTER();	//进入临界区
    CPU_CRITICAL_ENTER();
    if(state_t==WM_INIT_DIALOG)
    {
      sprintf(sp_a,"0:/system/dat/HEAD%03u.BIN",ImageNumber_store_a);
      if(!f_open(&fsrc, sp_a,  FA_READ))
      {
        f_read (&fsrc,buf_a,HEADER_LENGTH,&br);
        
        f_close (&fsrc);
        
        aes_decode(buf_a,HEADER_LENGTH);
        
        if(root_a)
          cJSON_Delete(root_a);
        
        root_a = cJSON_Parse((char const *)&buf_a[0]);
        //        sprintf(sp1,"芯片:%s",cJSON_GetObjectItem(root, "chip_type")->valuestring);
        //        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT16);
        //        
        //        TEXT_SetFont(hItem, &GUI_Fontsongti12);
        //        TEXT_SetTextColor(hItem, GUI_WHITE);
        //        TEXT_SetText(hItem, sp1);
        
        
        //        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT18);
        //        TEXT_SetFont(hItem, &GUI_Fontsongti12);
        //        TEXT_SetTextColor(hItem, GUI_WHITE);
        //        sprintf(sp1,"接口:%s",cJSON_GetObjectItem(root, "port_type")->valuestring);
        //        TEXT_SetText(hItem, sp1);    
        
        
        
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT19);
        TEXT_SetFont(hItem, &GUI_Fontsongti12);
        TEXT_SetTextColor(hItem, GUI_WHITE);
        sprintf(sp_a,"镜像:%s",cJSON_GetObjectItem(root_a, "note")->valuestring);
        TEXT_SetText(hItem, sp_a);    
        
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT20);
        TEXT_SetFont(hItem, &GUI_Fontsongti12);
        TEXT_SetTextColor(hItem, GUI_WHITE);
        
        if(USB_Host.gState==HOST_CLASS)//扫码枪已连接
        {
          sprintf(sp_a,"码枪:已连接");
        }
        else
        {
          sprintf(sp_a,"码枪:已断开");
        }
        TEXT_SetText(hItem, sp_a);          
        
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT21);
        TEXT_SetFont(hItem, &GUI_Fontsongti12);
        TEXT_SetTextColor(hItem, GUI_WHITE);
        if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
        {
          sprintf(sp_a,"checksum:0x%08X",(u32)cJSON_GetObjectItem(root_a, "eeprom_checksum")->valuedouble);
        }
        else
        {
          sprintf(sp_a,"checksum:0x%08X",(u32)cJSON_GetObjectItem(root_a, "flash_checksum")->valuedouble);
        }
        
        TEXT_SetText(hItem, sp_a);
        
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT25);
        TEXT_SetFont(hItem, &GUI_Fontsongti12);
        TEXT_SetTextColor(hItem, GUI_WHITE);
        sprintf(sp_a,"已用次数:%010u",(u32)cJSON_GetObjectItem(root_a, "times_used")->valuedouble);
        TEXT_SetText(hItem, sp_a);       
        
        
        totaltimes=(u32)(cJSON_GetObjectItem(root_a, "times")->valuedouble);//88888888为不限制次数
        
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT26);
        TEXT_SetFont(hItem, &GUI_Fontsongti12);
        TEXT_SetTextColor(hItem, GUI_WHITE);
        sprintf(sp_a,"剩余次数:%010u",totaltimes);
        TEXT_SetText(hItem, sp_a);   
        
        if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWD") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SWIM") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "UART") == 0))
        {
          for(u8 i=0;i<15;i++)
          {
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
            
            TEXT_SetFont(hItem, &GUI_Fontsongti12);
            TEXT_SetTextColor(hItem, GUI_WHITE);
            TEXT_SetText(hItem,   pro_item_1[i]);
          }
        }
        else if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "ICP") == 0))
        {
          for(u8 i=0;i<15;i++)
          {
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
            
            TEXT_SetFont(hItem, &GUI_Fontsongti12);
            TEXT_SetTextColor(hItem, GUI_WHITE);
            TEXT_SetText(hItem,   pro_item_2[i]);
          }
        }      
        else if((strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SPI1") == 0)||(strcmp(cJSON_GetObjectItem(root_a, "port_type")->valuestring, "SPI2") == 0))
        {
          if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "FLASH") == 0)
          {      
            for(u8 i=0;i<15;i++)
            {
              hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
              
              TEXT_SetFont(hItem, &GUI_Fontsongti12);
              TEXT_SetTextColor(hItem, GUI_WHITE);
              TEXT_SetText(hItem,   pro_item_3[i]);
            }
          }
          else if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
          {      
            for(u8 i=0;i<15;i++)
            {
              hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0+i);
              
              TEXT_SetFont(hItem, &GUI_Fontsongti12);
              TEXT_SetTextColor(hItem, GUI_WHITE);
              TEXT_SetText(hItem,   pro_item_4[i]);
            }
          } 
        }
        
        //        out_voltage_select();
        //        
        //        
        //        if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "SWIM") == 0)
        //        {
        //          SWIM_GPIO_Init();
        //        }
        //        else if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "SWD") == 0)
        //        {
        //          //SWIM_GPIO_Init();
        //        }
        //        else if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "UART") == 0)
        //        {
        //          USART3_Config();
        //        }
        //        else if(strcmp(cJSON_GetObjectItem(root, "port_type")->valuestring, "IIC") == 0)
        //        {
        //          systick_init(72);	
        //          stm32f1xx_i2c_init();	/*i2c init*/
        //        }
        
        
      }
      
      
      
      
      
      
      GUI_SetFont(&GUI_Fontsongti12);//
      GUI_SetColor(GUI_WHITE);//
      draw_select();    
      drawline();
      
//      if(system_set_data_r.system_set_data_bit.bit_4==0)
//      {
//        GUI_SetColor(GUI_YELLOW);
//        GUI_DrawHLine(220,150,235);
//      }
      
      
      
      GUI_SetColor(GUI_YELLOW);
      GUI_SetPenSize(1);
      GUI_AA_DrawRoundedRect(3, 3, 237, 312, 5);
      // GUI_DrawBitmap(&bmchip, 5, 62);
      ENABLE_PROGRESS_REFRESH;
      draw_progress_bar(mh);
    }
   // state_t=WM_PAINT;
    //  OS_CRITICAL_EXIT();
    CPU_CRITICAL_EXIT();
    break;
    
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_RELEASED:
      switch (Id) {
        
      case GUI_ID_CHANGE0:
        has_window=0;
        WM_DeleteTimer(h_timer);
        GUI_EndDialog(_hDialogControl_offline_write, 0);
        channel=0;
        _hDialogControl_listbox=GUI_CreateDialogBox(_aFrameWinControl_listbox, GUI_COUNTOF(_aFrameWinControl_listbox), &_cbFrameWinControl_listbox, WM_HBKWIN, 0, 0);
        GUI_Exec();            
        break;
      case GUI_ID_CHANGE:
        has_window=0;
        WM_DeleteTimer(h_timer);
        GUI_EndDialog(_hDialogControl_offline_write, 0);   
        channel=1;
        _hDialogControl_listbox=GUI_CreateDialogBox(_aFrameWinControl_listbox, GUI_COUNTOF(_aFrameWinControl_listbox), &_cbFrameWinControl_listbox, WM_HBKWIN, 0, 0);
        GUI_Exec();       
        break;
      case GUI_ID_MENU:
        has_window=0;
        WM_DeleteTimer(h_timer);
        GUI_EndDialog(_hDialogControl_offline_write, 0);        
        _hDialogControl_main_menu=GUI_CreateDialogBox(_aFrameWinControl_main_menu, GUI_COUNTOF(_aFrameWinControl_main_menu), &_cbFrameWinControl_main_menu, WM_HBKWIN, 0, 0);
        GUI_Exec();
        break;
      }
      break;
    }
    break;
    
    
  case WM_TIMER:
    char sss[100];
    
    CPU_CRITICAL_ENTER();//进入临界区
    sprintf(sss,"CPU：%u.%02u%%",OSStatTaskCPUUsage / 100, OSStatTaskCPUUsage % 100 );  
    // sprintf(sss,"CPU使用率：%u%%",OSStatTaskCPUUsage / 100);  
    
    
    
    //   TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_CPU_USE),   sss); 
    
    //     
    //GUI_Exec();
    
    WM_SelectWindow(_hDialogControl_offline_write);
    GUI_SetBkColor(0x787878);
    GUI_ClearRect(120, 67, 235, 78);
    GUI_SetFont(&GUI_Fontsongti12);
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TEXTMODE_REV);
    GUI_DispStringAt (sss,120, 67);
    
    
    
    GUI_SetFont(&GUI_Fontsongti12);
    GUI_SetColor(GUI_WHITE);
    
    
    
    
    
    sprintf(sp_a,"%04u:%02u:%02u:%u",time_data_r.time_data_bit.hours,time_data_r.time_data_bit.minutes,time_data_r.time_data_bit.seconds,time_data_r.time_data_bit.milli);
    GUI_DispStringAt (sp_a,165,183);//
    GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
    //    GUI_SetFont(&GUI_Fontsongti12);
    //    GUI_SetColor(GUI_WHITE);
    //    sprintf(sp1,"电压:%4.2fv",*(ADCConvertedValueTemp+0));
    //    GUI_DispStringAt (sp1,167, 209);
    
    totaltimes=(u32)(cJSON_GetObjectItem(root_a, "times")->valuedouble);//88888888为不限制次数
    
    if(totaltimes==88888888)
    {
      GUI_ClearRect(122, 37, 235, 50);
      GUI_SetBkColor(0x787878);
      GUI_SetColor(GUI_YELLOW);
      
      GUI_SetTextMode(GUI_TEXTMODE_REV);
      GUI_DispStringAt("剩余次数:不限次数",122, 37);
    }
    
    
    
    GUI_SetTextMode(GUI_TEXTMODE_NORMAL);
    GUI_SetBkColor(0x787878);
    GUI_SetColor(GUI_WHITE);
    
    if(USB_Host.gState==HOST_CLASS)//扫码枪已连接
    {
      GUI_DispStringAt ("码枪:已连接",170, 6);
    }
    else
    {
      GUI_DispStringAt ("码枪:已断开",170, 6);
    }
    
    
    //本次烧录
    GUI_SetFont(GUI_FONT_24B_1);
    GUI_SetColor(GUI_WHITE);
    sprintf(sp_a,"%07u",CurrentTimes);
    GUI_DispStringAt (sp_a,32, 39);
    
    

    
    //同步计数值
    GUI_SetFont(GUI_FONT_32B_1);   
    GUI_SetColor(GUI_YELLOW);
    sprintf(sp_a,"%02u/28",SynCountTimes);
    GUI_DispStringAt (sp_a,32, 69);
    
    
    
    
    
    GUI_SetFont(&GUI_Fontsongti12);
    GUI_SetColor(GUI_WHITE);
    

    
    
    WM_RestartTimer(h_timer, 500);//500ms
    //OS_CRITICAL_EXIT();
    CPU_CRITICAL_EXIT();
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

void _cbFrameWinControl_image_updata(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  static u8 state_t=WM_INIT_DIALOG;
  switch (pMsg->MsgId) {
  case WM_KEY:
    WM_SendMessage(WM_HBKWIN, pMsg);
    break;
  case WM_INIT_DIALOG:
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON0);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT2);
    TEXT_SetFont(hItem, &GUI_Fontsongti17);
    TEXT_SetTextColor(hItem, GUI_BLUE);
    
    USB_DP_ENABLE;
    
    state_t=WM_INIT_DIALOG;
    break;
  case WM_PAINT:
    
    if(state_t==WM_INIT_DIALOG)
    {
      GUI_SetColor(GUI_YELLOW);
      GUI_SetPenSize(1);
      GUI_AA_DrawRoundedRect(3, 3, 237, 320-50, 5);
      DRAW_MCUART_COM;
      
      GUI_DrawBitmap(&bmupdate, 58, 58);
    }
    state_t=WM_PAINT;
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_RELEASED:
      switch (Id) {
      case GUI_ID_BUTTON0:
        
        GUI_EndDialog(_hDialogControl_image_updata, 0);        
        _hDialogControl_main_menu=GUI_CreateDialogBox(_aFrameWinControl_main_menu, GUI_COUNTOF(_aFrameWinControl_main_menu), &_cbFrameWinControl_main_menu, WM_HBKWIN, 0, 0);           
        USB_DP_DISABLE;        
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }  
}

void _cbFrameWinControl_system_set(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  OS_ERR err;
  int     NCode;
  int     Id;
  static u8 state_t=WM_INIT_DIALOG;
  switch (pMsg->MsgId) {
  case WM_KEY:
    WM_SendMessage(WM_HBKWIN, pMsg);
    break;
  case WM_INIT_DIALOG:
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT2);
    TEXT_SetFont(hItem, &GUI_Fontsongti17);
    TEXT_SetTextColor(hItem, GUI_BLUE);
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON0);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    
    
    system_set_data_r.system_set_data_all=*(u32 *)SYSTEM_SET_DATA_ADDRESS_R;
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK0);
    CHECKBOX_SetFont(hItem,&GUI_Fontsongti12);
    CHECKBOX_SetTextColor (hItem, GUI_WHITE);
    CHECKBOX_SetText(hItem, "使能蜂鸣器提示");
    CHECKBOX_SetState(hItem,system_set_data_r.system_set_data_bit.bit_0);//蜂鸣器提示音使能 YES/NO
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK1);
    CHECKBOX_SetFont(hItem,&GUI_Fontsongti12);
    CHECKBOX_SetTextColor (hItem, GUI_WHITE);
    CHECKBOX_SetText(hItem, "开机进入脱机烧写");
    CHECKBOX_SetState(hItem,system_set_data_r.system_set_data_bit.bit_1);//设置开机进入脱机烧写 YES/NO
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK2);
    CHECKBOX_SetFont(hItem,&GUI_Fontsongti12);
    CHECKBOX_SetTextColor (hItem, GUI_WHITE);
    CHECKBOX_SetText(hItem, "允许自动连接计算机");
    CHECKBOX_SetState(hItem,system_set_data_r.system_set_data_bit.bit_2);//允许自动连接计算机 YES/NO
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK3);
    CHECKBOX_SetFont(hItem,&GUI_Fontsongti12);
    CHECKBOX_SetTextColor (hItem, GUI_WHITE);
    CHECKBOX_SetText(hItem, "使能开机密码");
    CHECKBOX_SetState(hItem,system_set_data_r.system_set_data_bit.bit_3);//使能开机密码
    
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK4);
    CHECKBOX_SetFont(hItem,&GUI_Fontsongti12);
    CHECKBOX_SetTextColor (hItem, GUI_WHITE);
    CHECKBOX_SetText(hItem, "使能通道A");
    CHECKBOX_SetState(hItem,system_set_data_r.system_set_data_bit.bit_4);//使能通道A    
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK5);
    CHECKBOX_SetFont(hItem,&GUI_Fontsongti12);
    CHECKBOX_SetTextColor (hItem, GUI_WHITE);
    CHECKBOX_SetText(hItem, "使能通道B");
    CHECKBOX_SetState(hItem,system_set_data_r.system_set_data_bit.bit_5);//使能通道B        
    
    state_t=WM_INIT_DIALOG;
    break;
  case WM_PAINT:
    
    if(state_t==WM_INIT_DIALOG)
    {
      
      GUI_SetColor(GUI_YELLOW);
      GUI_SetPenSize(1);
      GUI_AA_DrawRoundedRect(3, 3, 237, 320-50, 5);
      DRAW_MCUART_COM;
    }
    state_t=WM_PAINT;
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_RELEASED:
      switch (Id) {
      case GUI_ID_BUTTON0:
        
        GUI_EndDialog(_hDialogControl_system_set, 0);        
        _hDialogControl_main_menu=GUI_CreateDialogBox(_aFrameWinControl_main_menu, GUI_COUNTOF(_aFrameWinControl_main_menu), &_cbFrameWinControl_main_menu, WM_HBKWIN, 0, 0);           
        
        break;
        
      case GUI_ID_CHECK0:    
      case GUI_ID_CHECK1:
      case GUI_ID_CHECK2:
      case GUI_ID_CHECK3:
      case GUI_ID_CHECK4:
      case GUI_ID_CHECK5:        
        CPU_SR_ALLOC();
        CPU_CRITICAL_ENTER();
        
        for(u16 i=0;i<100;i++)
        {
          parameter[i]=*(u8 *)(PARAMETER_ADDRESS_R+i);
          sFLASH_Tx_Buffer[i]=parameter[i];
        }
        sFLASH_EraseSector(PARAMETER_ADDRESS_W);//擦除parameter参数保存地址信息
        
        sFLASH_WriteBuffer(sFLASH_Tx_Buffer, PARAMETER_ADDRESS_W, OFFSET_SYSTEM_SET_DATA);//擦除后立即回写parameter参数
        
        
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK0);
        system_set_data_r.system_set_data_bit.bit_0=CHECKBOX_IsChecked(hItem);//蜂鸣器提示音使能 
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK1);
        system_set_data_r.system_set_data_bit.bit_1=CHECKBOX_IsChecked(hItem);//设置开机进入脱机烧写
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK2);
        system_set_data_r.system_set_data_bit.bit_2=CHECKBOX_IsChecked(hItem);//允许自动连接计算机
        auto_connect_computer=system_set_data_r.system_set_data_bit.bit_2;
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK3);
        system_set_data_r.system_set_data_bit.bit_3=CHECKBOX_IsChecked(hItem);//使能开机密码
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK4);
        
        if(system_set_data_r.system_set_data_bit.bit_4!=CHECKBOX_IsChecked(hItem))
        {
          system_set_data_r.system_set_data_bit.bit_4=CHECKBOX_IsChecked(hItem);//使能通道A
          if(system_set_data_r.system_set_data_bit.bit_4==0)
          {
            OSTaskDel((OS_TCB * )&Program_a_TaskTCB,(OS_ERR * )&err);
          }
          else
          {
            //创建PROGRAM_A任务
            OSTaskCreate((OS_TCB 	* )&Program_a_TaskTCB,		
                         (CPU_CHAR	* )"Program a task", 		
                         (OS_TASK_PTR )program_a_task, 			
                         (void		* )0,					
                         (OS_PRIO	  )PROGRAM_A_TASK_PRIO,     
                         (CPU_STK   * )&PROGRAM_A_TASK_STK[0],	
                         (CPU_STK_SIZE)PROGRAM_A_STK_SIZE/10,	
                         (CPU_STK_SIZE)PROGRAM_A_STK_SIZE,		
                         (OS_MSG_QTY  )0,					
                         (OS_TICK	  )0,  					
                         (void   	* )0,					
                         (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                         (OS_ERR 	* )&err);	   
            OSTaskSuspend((OS_TCB*)&Program_a_TaskTCB,&err);//挂起烧写任务A 
          }   
        }
        
        
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CHECK5);
        if(system_set_data_r.system_set_data_bit.bit_5!=CHECKBOX_IsChecked(hItem))
        {
          system_set_data_r.system_set_data_bit.bit_5=CHECKBOX_IsChecked(hItem);//使能通道B
          if(system_set_data_r.system_set_data_bit.bit_5==0)
          {
            OSTaskDel((OS_TCB * )&Program_b_TaskTCB,(OS_ERR * )&err);
          }
          else
          {
            //创建PROGRAM_B任务
            OSTaskCreate((OS_TCB 	* )&Program_b_TaskTCB,		
                         (CPU_CHAR	* )"Program b task", 		
                         (OS_TASK_PTR )program_b_task, 			
                         (void		* )0,					
                         (OS_PRIO	  )PROGRAM_B_TASK_PRIO,     
                         (CPU_STK   * )&PROGRAM_B_TASK_STK[0],	
                         (CPU_STK_SIZE)PROGRAM_B_STK_SIZE/10,	
                         (CPU_STK_SIZE)PROGRAM_B_STK_SIZE,		
                         (OS_MSG_QTY  )0,					
                         (OS_TICK	  )0,  					
                         (void   	* )0,					
                         (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                         (OS_ERR 	* )&err);	   
            OSTaskSuspend((OS_TCB*)&Program_b_TaskTCB,&err);//挂起烧写任务B 
          }             
        }
        
        
        
        
        
        
        
        
        
        sFLASH_WriteBuffer((u8 *)&system_set_data_r.system_set_data_all,PARAMETER_ADDRESS_W+OFFSET_SYSTEM_SET_DATA, 4);//写设置
        
        sFLASH_WriteBuffer((u8 *)&ImageNumber_store,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER, 4);//保存镜像号
        sFLASH_WriteBuffer((u8 *)&ImageNumber_store_a,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER_A, 4);//保存镜像号_A
        sFLASH_WriteBuffer((u8 *)&ImageNumber_store_b,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER_B, 4);//保存镜像号_B
        sFLASH_WriteBuffer((u8 *)&password_store,PARAMETER_ADDRESS_W + OFFSET_PASSWORD_VALUE, 4);//保存密码
        sFLASH_WriteBuffer((u8 *)&language_store,PARAMETER_ADDRESS_W + OFFSET_LANGUAGE, 4);//一个字等于4个字节，写保存的中英文语言
        
        
        sFLASH_ReadBuffer(sFLASH_Rx_Buffer, FLASH_READ_ADDRESS, 100);//读参数信息
        CPU_CRITICAL_EXIT();
        
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

void _cbFrameWinControl_udisk_mode(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  
  int     NCode;
  int     Id;
  
  switch (pMsg->MsgId) {
  case WM_KEY:
    WM_SendMessage(WM_HBKWIN, pMsg);
    break;
  case WM_INIT_DIALOG:
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_BUTTON0);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT2);
    TEXT_SetFont(hItem, &GUI_Fontsongti17);
    TEXT_SetTextColor(hItem, GUI_BLUE);
    
    USB_DP_ENABLE_MASS;    
    
    break;
  case WM_PAINT:
    
    GUI_SetColor(GUI_YELLOW);
    GUI_SetPenSize(1);
    GUI_AA_DrawRoundedRect(3, 3, 237, 320-50, 5);
    DRAW_MCUART_COM;
    
    GUI_DrawBitmap(&bmUSB, 58, 58);
    
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_RELEASED:
      switch (Id) {
      case GUI_ID_BUTTON0:
        
        GUI_EndDialog(_hDialogControl_udisk_mode, 0);        
        _hDialogControl_main_menu=GUI_CreateDialogBox(_aFrameWinControl_main_menu, GUI_COUNTOF(_aFrameWinControl_main_menu), &_cbFrameWinControl_main_menu, WM_HBKWIN, 0, 0);           
        USB_DP_DISABLE_MASS;      
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }  
}




void _cbFrameWinControl_main_menu(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  
  int     NCode;
  int     Id;
  
  switch (pMsg->MsgId) {
  case WM_KEY:
    WM_SendMessage(WM_HBKWIN, pMsg);
    break;
  case WM_INIT_DIALOG:
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_OFFLINE_WRITE);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_IMAGE_UPDATA);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_SYSTEM_SET);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_UDISK_MODE);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0);
    
    TEXT_SetFont(hItem, &GUI_Fontsongti12);
    TEXT_SetTextColor(hItem, GUI_WHITE);
    sprintf(sp1,"固件特征码:0x%08X",*(u32 *)0x08007000);
    // extern uint16_t __checksum;
    // sprintf(sp1,"固件特征码:0x%08X",__checksum);
    
    TEXT_SetText(hItem,   sp1); 
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1);
    TEXT_SetFont(hItem, &GUI_Fontsongti12);
    TEXT_SetTextColor(hItem, GUI_WHITE);
    
    
    
    
    
    
    
    
    sprintf(sp1, "固件版本:%02u%02u%02u%02u%02u%02u", firmware_time.Year, firmware_time.Month, firmware_time.Day, firmware_time.hh, firmware_time.mm, firmware_time.ss);//任意格式化
    
    
    //   sprintf(sp1,"固件版本号:0x%08X", __DATE__ );
    TEXT_SetText(hItem,sp1); 
    
    break;
  case WM_PAINT:
    
    
    GUI_SetColor(GUI_YELLOW);
    GUI_SetPenSize(1);
    GUI_AA_DrawRoundedRect(3, 3, 237, 320-50, 5);
    GUI_SetFont(&GUI_FontAudiowide32);
    GUI_SetColor(GUI_WHITE);
    GUI_DispStringAt("mcu",40, 320-50-10);
    GUI_SetColor(GUI_BLUE);
    GUI_DispString("art.com");
    
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_RELEASED:
      switch (Id) {
      case GUI_ID_OFFLINE_WRITE:
        
        
        
        
        
        GUI_EndDialog(_hDialogControl_main_menu, 0);
        _hDialogControl_offline_write=GUI_CreateDialogBox(_aFrameWinControl_offline_write, GUI_COUNTOF(_aFrameWinControl_offline_write), &_cbFrameWinControl_offline_write, WM_HBKWIN, 0, 0);  
        GUI_Exec();
        h_timer=WM_CreateTimer(_hDialogControl_offline_write, 0, 100, 0);
        
        break;
      case GUI_ID_IMAGE_UPDATA:
        
        GUI_EndDialog(_hDialogControl_main_menu, 0);
        
        _hDialogControl_image_updata=GUI_CreateDialogBox(_aFrameWinControl_image_updata, GUI_COUNTOF(_aFrameWinControl_image_updata), &_cbFrameWinControl_image_updata, WM_HBKWIN, 0, 0);

        GUI_Exec();       
        //        OS_ERR err;
        //        
        //        
        ////        BUZZER_ON;
        ////        
        ////        OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);//?óê±500ms
        ////        BUZZER_OFF;
        //        static u8 J=0;
        //        if(J==0)
        //        {
        //          J=1;
        //          SW_5V_OFF_0;
        //          SW_3V3_OFF_0;
        //        }
        //        else if(J==1)
        //        {
        //          J=2;
        //          SW_5V_OFF_0;
        //          SW_3V3_ON_0;
        //        }
        //        else
        //        {
        //          J=0;
        //          SW_3V3_OFF_0;
        //          SW_5V_ON_0;
        //        }  
        
        break;
      case GUI_ID_SYSTEM_SET:
        GUI_EndDialog(_hDialogControl_main_menu, 0);
        _hDialogControl_system_set=GUI_CreateDialogBox(_aFrameWinControl_system_set, GUI_COUNTOF(_aFrameWinControl_system_set), &_cbFrameWinControl_system_set, WM_HBKWIN, 0, 0); 
        GUI_Exec();
        
        //        static u8 h=0;
        //        if(h==0)
        //        {
        //          h=1;
        //          SW_5V_OFF_1;
        //          SW_3V3_OFF_1;
        //        }
        //        else if(h==1)
        //        {
        //          h=2;
        //          SW_5V_OFF_1;
        //          SW_3V3_ON_1;
        //        }
        //        else
        //        {
        //          h=0;
        //          SW_3V3_OFF_1;
        //          SW_5V_ON_1;
        //        }
        break; 
      case GUI_ID_UDISK_MODE:
        
        GUI_EndDialog(_hDialogControl_main_menu, 0);
        _hDialogControl_udisk_mode=GUI_CreateDialogBox(_aFrameWinControl_udisk_mode, GUI_COUNTOF(_aFrameWinControl_udisk_mode), &_cbFrameWinControl_udisk_mode, WM_HBKWIN, 0, 0);
        
        GUI_Exec();
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

static void delay(void)
{
  for(u32 i=0;i<0x1FFFFFF;i++);
}

void tf_card_warning(void)
{
  GUI_UC_SetEncodeUTF8(); // Enable UTF8 decoding
  GUI_SetBkColor(0x787878);
  GUI_Clear();
  GUI_SetColor(GUI_YELLOW);
  GUI_SetPenSize(1);
  GUI_AA_DrawRoundedRect(5, 115, 235, 200, 5);
  
  GUI_SetColor(GUI_RED);
  
  GUI_SetFont(&GUI_Fontsongti12);
  
  GUI_DispStringHCenterAt("存储器异常!\r\n\r\n即将重启动!",120, 140);
  BUZZER_ON;
  delay();
  NVIC_SystemReset();
  while(1);
}

__IO uint16_t uhADCxConvertedValue[3];
/**
* @brief  ADC3 channel07 with DMA configuration
* @note   This function Configure the ADC peripheral  
1) Enable peripheral clocks
2) DMA2_Stream0 channel2 configuration
3) Configure ADC Channel7 pin as analog input
4) Configure ADC3 Channel7 
* @param  None
* @retval None
*/
static void ADC_Config(void)
{
  ADC_InitTypeDef       ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef       DMA_InitStructure;
  GPIO_InitTypeDef      GPIO_InitStructure;
  
  /* Enable ADCx, DMA and GPIO clocks ****************************************/ 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  
  
  /* DMA2 Stream0 channel2 configuration **************************************/
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)uhADCxConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 3;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
  DMA_Cmd(DMA2_Stream0, ENABLE);
  
  /* Configure ADC3 Channel7 pin as analog input ******************************/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
  
  /* ADC3 Init ****************************************************************/
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 3;
  ADC_Init(ADC1, &ADC_InitStructure);
  
  /* ADC3 regular channel7 configuration **************************************/
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_3Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_3Cycles);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 3, ADC_SampleTime_3Cycles);
  /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  
  /* Enable ADC3 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC3 */
  ADC_Cmd(ADC1, ENABLE);
  
  
  
  /* Start ADC Software Conversion */ 
  ADC_SoftwareStartConv(ADC1);
  
  
  //  ADC_InitTypeDef ADC_InitStructure;
  //  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  //  GPIO_InitTypeDef GPIO_InitStructure;
  //
  //  /* Enable ADC3 clock */
  //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  //
  //  /* GPIOF clock enable */
  //  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); 
  //
  //  /* Configure ADC Channel7 as analog */
  //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; 
  //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  //  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  //  GPIO_Init(GPIOC, &GPIO_InitStructure);
  //
  //  /* ADC Common Init */
  //  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  //  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;
  //  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  //  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; 
  //  ADC_CommonInit(&ADC_CommonInitStructure); 
  //
  //  /* ADC3 Configuration ------------------------------------------------------*/
  //  ADC_StructInit(&ADC_InitStructure);
  //  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  //  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  //  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  //  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; 
  //  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //  ADC_InitStructure.ADC_NbrOfConversion = 1;
  //  ADC_Init(ADC3, &ADC_InitStructure);
  //
  //  /* ADC3 Regular Channel Config */
  //  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_56Cycles);
  //
  //  /* Enable ADC3 */
  //  ADC_Cmd(ADC1, ENABLE);
  //
  //  /* ADC3 regular Software Start Conv */ 
  //  ADC_SoftwareStartConv(ADC1);  
  
  
  
}

u16 ADCConvertedValue[3];
u16 ADCConvertedValueAverage[3];
u32 ADCConvertedValueTotal[3];


void voltage_caculate(void)
{
  
  *(ADCConvertedValueTotal+2)-=*(ADCConvertedValueAverage+2);//总值减去平均值,一开始平均值会很小
  *(ADCConvertedValueTotal+2)+=uhADCxConvertedValue[2];//总值加上本次值 
  *(ADCConvertedValueAverage+2)=*(ADCConvertedValueTotal+2)/8;//8次滤波值
  
  *(ADCConvertedValueTemp+2)=(float)*(ADCConvertedValueAverage+2)/614;
  
  
  *(ADCConvertedValueTotal+1)-=*(ADCConvertedValueAverage+1);//总值减去平均值,一开始平均值会很小
  *(ADCConvertedValueTotal+1)+=uhADCxConvertedValue[1];//总值加上本次值 
  *(ADCConvertedValueAverage+1)=*(ADCConvertedValueTotal+1)/8;//8次滤波值
  
  *(ADCConvertedValueTemp+1)=(float)*(ADCConvertedValueAverage+1)/614;
  
  
  
  
  *(ADCConvertedValueTotal+0)-=*(ADCConvertedValueAverage+0);//总值减去平均值,一开始平均值会很小
  *(ADCConvertedValueTotal+0)+=uhADCxConvertedValue[0];//总值加上本次值 
  *(ADCConvertedValueAverage+0)=*(ADCConvertedValueTotal+0)/8;//8次滤波值
  
  *(ADCConvertedValueTemp+0)=(float)*(ADCConvertedValueAverage+0)/614;
  
}


void _cbFrameWinControl_listbox(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  
  switch (pMsg->MsgId) {
  case WM_KEY:
    WM_SendMessage(WM_HBKWIN, pMsg);
    break;
  case WM_INIT_DIALOG:
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_CANCEL);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_PRE);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_NEXT);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_OK);
    BUTTON_SetFocussable(hItem, 0);
    BUTTON_SetFont(hItem, &GUI_Fontsongti16);
    
    hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX);
    LISTBOX_SetAutoScrollH(hItem, 1);
    LISTBOX_SetAutoScrollV(hItem, 1);
    LISTBOX_SetScrollbarWidth(hItem,30);
    LISTBOX_SetItemSpacing(hItem,20); 
    
    LISTBOX_SetFont(hItem, &GUI_Fontsongti12);
    
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    if(channel==0)
    {
      listcount_a=ImageNumber_store_a/20*20;//listcount被清零
      for(u8 ix=0;ix<20;ix++)
      {
        sprintf(sp_a,"0:/system/dat/HEAD%03u.BIN",ix+listcount_a);
        if(!f_open(&fsrc, sp_a,  FA_READ))
        {
          f_read (&fsrc,buf_a,HEADER_LENGTH,&br);
          
          f_close (&fsrc);
          
          aes_decode(buf_a,HEADER_LENGTH);
          
          cJSON_Delete(root_a);
          
          root_a = cJSON_Parse((char const *)&buf_a[0]);
          
          if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
          {
            sprintf(sp_a,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_a,cJSON_GetObjectItem(root_a, "note")->valuestring,(u32)cJSON_GetObjectItem(root_a, "eeprom_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_a, "times")->valuedouble);
          }
          else
          {
            sprintf(sp_a,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_a,cJSON_GetObjectItem(root_a, "note")->valuestring,(u32)cJSON_GetObjectItem(root_a, "flash_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_a, "times")->valuedouble);                
          }
          LISTBOX_AddString(hItem,sp_a);
        }
        else
        {
          sprintf(sp_a,"镜像号:%u 无",ix+listcount_a);
          LISTBOX_AddString(hItem,sp_a);
        }
        
        
      }
      
      LISTBOX_SetSel(hItem, ImageNumber_store_a%20);//选中当前余数位置镜像      
      
      
      
    }
    else if(channel==1)
    {
      listcount_b=ImageNumber_store_b/20*20;//listcount被清零
      
      for(u8 ix=0;ix<20;ix++)
      {
        sprintf(sp_b,"0:/system/dat/HEAD%03u.BIN",ix+listcount_b);
        if(!f_open(&fsrc, sp_b,  FA_READ))
        {
          f_read (&fsrc,buf_b,HEADER_LENGTH,&br);
          
          f_close (&fsrc);
          
          aes_decode(buf_b,HEADER_LENGTH);
          
          cJSON_Delete(root_b);
          
          root_b = cJSON_Parse((char const *)&buf_b[0]);
          
          if(strcmp(cJSON_GetObjectItem(root_b, "chip_type")->valuestring, "EEPROM") == 0)
          {
            sprintf(sp_b,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_b,cJSON_GetObjectItem(root_b, "note")->valuestring,(u32)cJSON_GetObjectItem(root_b, "eeprom_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_b, "times")->valuedouble);
          }
          else
          {
            sprintf(sp_b,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_b,cJSON_GetObjectItem(root_b, "note")->valuestring,(u32)cJSON_GetObjectItem(root_b, "flash_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_b, "times")->valuedouble);                
          }
          LISTBOX_AddString(hItem,sp_b);
        }
        else
        {
          sprintf(sp_b,"镜像号:%u 无",ix+listcount_b);
          LISTBOX_AddString(hItem,sp_b);
        }
        
        
      }
      
      LISTBOX_SetSel(hItem, ImageNumber_store_b%20);//选中当前余数位置镜像            
    }    
    
    
    
    CPU_CRITICAL_EXIT();
    
    break;
  case WM_PAINT:
    //    xSize = WM_GetWindowSizeX(pMsg->hWin);
    //    ySize = WM_GetWindowSizeY(pMsg->hWin);
    //    //    //  GUI_DrawGradientV(0, 0, xSize - 1, ySize - 1, 0xFFFFFF, 0xDCCEC0);
    //    GUI_DrawGradientV(0, 0, xSize - 1, ySize - 1, 0xFFFFFF, 0x787878);
    //    //     GUI_DrawGradientV(0, 0, xSize - 1, ySize - 1, 0xFFFFFF, GUI_RED);
    
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch (NCode) {
    case WM_NOTIFICATION_RELEASED:
      switch (Id) {
      case GUI_ID_CANCEL:
        //WM_HideWindow(_hDialogControl_listbox);
        //WM_ShowWindow(_hDialogControl_offline_write); 
        GUI_EndDialog(_hDialogControl_listbox, 0);
        _hDialogControl_offline_write=GUI_CreateDialogBox(_aFrameWinControl_offline_write, GUI_COUNTOF(_aFrameWinControl_offline_write), &_cbFrameWinControl_offline_write, WM_HBKWIN, 0, 0);  
        GUI_Exec();
        h_timer=WM_CreateTimer(_hDialogControl_offline_write, 0, 100, 0);
        break;
      case GUI_ID_PRE:
        if(channel==0)
        {
          if(listcount_a>=20)
          {
            listcount_a-=20;
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX);
            for(u8 ix=0;ix<20;ix++)
            {
              sprintf(sp_a,"0:/system/dat/HEAD%03u.BIN",ix+listcount_a);
              if(!f_open(&fsrc, sp_a,  FA_READ))
              {
                f_read (&fsrc,buf_a,HEADER_LENGTH,&br);
                
                f_close (&fsrc);
                
                aes_decode(buf_a,HEADER_LENGTH);
                
                cJSON_Delete(root_a);
                
                root_a = cJSON_Parse((char const *)&buf_a[0]);
                if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
                {
                  sprintf(sp_a,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_a,cJSON_GetObjectItem(root_a, "note")->valuestring,(u32)cJSON_GetObjectItem(root_a, "eeprom_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_a, "times")->valuedouble);
                }
                else
                {
                  sprintf(sp_a,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_a,cJSON_GetObjectItem(root_a, "note")->valuestring,(u32)cJSON_GetObjectItem(root_a, "flash_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_a, "times")->valuedouble);                
                }
                
                LISTBOX_SetString(hItem,sp_a,ix);
              }
              else
              {
                sprintf(sp_a,"镜像号:%u 无",ix+listcount_a);
                LISTBOX_SetString(hItem,sp_a,ix);
              }
              
            }  
          }          
        }
        else if(channel==1)
        {
          if(listcount_b>=20)
          {
            listcount_b-=20;
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX);
            for(u8 ix=0;ix<20;ix++)
            {
              sprintf(sp_b,"0:/system/dat/HEAD%03u.BIN",ix+listcount_b);
              if(!f_open(&fsrc, sp_b,  FA_READ))
              {
                f_read (&fsrc,buf_b,HEADER_LENGTH,&br);
                
                f_close (&fsrc);
                
                aes_decode(buf_b,HEADER_LENGTH);
                
                cJSON_Delete(root_b);
                
                root_b = cJSON_Parse((char const *)&buf_b[0]);
                if(strcmp(cJSON_GetObjectItem(root_b, "chip_type")->valuestring, "EEPROM") == 0)
                {
                  sprintf(sp_b,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_b,cJSON_GetObjectItem(root_b, "note")->valuestring,(u32)cJSON_GetObjectItem(root_b, "eeprom_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_b, "times")->valuedouble);
                }
                else
                {
                  sprintf(sp_b,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_b,cJSON_GetObjectItem(root_b, "note")->valuestring,(u32)cJSON_GetObjectItem(root_b, "flash_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_b, "times")->valuedouble);                
                }
                
                LISTBOX_SetString(hItem,sp_b,ix);
              }
              else
              {
                sprintf(sp_b,"镜像号:%u 无",ix+listcount_b);
                LISTBOX_SetString(hItem,sp_b,ix);
              }
              
            }  
          }            
        }
        break;
      case GUI_ID_NEXT:
        if(channel==0)
        {
          if(listcount_a<=960)
          {
            listcount_a+=20;
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX);
            for(u8 ix=0;ix<20;ix++)
            {
              sprintf(sp_a,"0:/system/dat/HEAD%03u.BIN",ix+listcount_a);
              if(!f_open(&fsrc, sp_a,  FA_READ))
              {
                f_read (&fsrc,buf_a,HEADER_LENGTH,&br);
                
                f_close (&fsrc);
                
                aes_decode(buf_a,HEADER_LENGTH);
                
                cJSON_Delete(root_a);
                
                root_a = cJSON_Parse((char const *)&buf_a[0]);
                
                if(strcmp(cJSON_GetObjectItem(root_a, "chip_type")->valuestring, "EEPROM") == 0)
                {
                  sprintf(sp_a,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_a,cJSON_GetObjectItem(root_a, "note")->valuestring,(u32)cJSON_GetObjectItem(root_a, "eeprom_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_a, "times")->valuedouble);
                }
                else
                {
                  sprintf(sp_a,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_a,cJSON_GetObjectItem(root_a, "note")->valuestring,(u32)cJSON_GetObjectItem(root_a, "flash_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_a, "times")->valuedouble);                
                }
                LISTBOX_SetString(hItem,sp_a,ix);
              }
              else
              {
                sprintf(sp_a,"镜像号:%u 无",ix+listcount_a);
                LISTBOX_SetString(hItem,sp_a,ix);
              }
              
            }  
          }          
        }
        else if(channel==1)
        {
          if(listcount_b<=960)
          {
            listcount_b+=20;
            hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX);
            for(u8 ix=0;ix<20;ix++)
            {
              sprintf(sp_b,"0:/system/dat/HEAD%03u.BIN",ix+listcount_b);
              if(!f_open(&fsrc, sp_b,  FA_READ))
              {
                f_read (&fsrc,buf_b,HEADER_LENGTH,&br);
                
                f_close (&fsrc);
                
                aes_decode(buf_b,HEADER_LENGTH);
                
                cJSON_Delete(root_b);
                
                root_b = cJSON_Parse((char const *)&buf_b[0]);
                
                if(strcmp(cJSON_GetObjectItem(root_b, "chip_type")->valuestring, "EEPROM") == 0)
                {
                  sprintf(sp_b,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_b,cJSON_GetObjectItem(root_b, "note")->valuestring,(u32)cJSON_GetObjectItem(root_b, "eeprom_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_b, "times")->valuedouble);
                }
                else
                {
                  sprintf(sp_b,"镜像号:%u 镜像:%s\nChecksum:0x%08X 剩余次数:%u",ix+listcount_b,cJSON_GetObjectItem(root_b, "note")->valuestring,(u32)cJSON_GetObjectItem(root_b, "flash_checksum")->valuedouble,(u32)cJSON_GetObjectItem(root_b, "times")->valuedouble);                
                }
                LISTBOX_SetString(hItem,sp_b,ix);
              }
              else
              {
                sprintf(sp_b,"镜像号:%u 无",ix+listcount_b);
                LISTBOX_SetString(hItem,sp_b,ix);
              }
              
            }  
          }             
        }
        
        
        break;
        
      case GUI_ID_OK:
        CPU_SR_ALLOC();
        CPU_CRITICAL_ENTER();	//进入临界区
        hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX);
        
        if(channel==0)
        {
          listselected_a=LISTBOX_GetSel(hItem);
          sprintf(sp_a,"0:/system/dat/HEAD%03u.BIN",listselected_a+listcount_a);
          
          if(!f_open(&fsrc, sp_a,  FA_READ))
          {
            f_read (&fsrc,buf_a,HEADER_LENGTH,&br);//关键性的修改buf中的数据
            f_close (&fsrc);
            
            aes_decode(buf_a,HEADER_LENGTH);        
            if(ImageNumber_store_a!=(listselected_a+listcount_a))//镜像切换了，和上次的镜像号不一样了，保存到单片机FLAH
            {
              
              ImageNumber_store_a=listselected_a+listcount_a;
              
              
              for(u16 i=0;i<100;i++)
              {
                parameter[i]=*(u8 *)(PARAMETER_ADDRESS_R+i);
                sFLASH_Tx_Buffer[i]=parameter[i];
              }
              sFLASH_EraseSector(PARAMETER_ADDRESS_W);//擦除parameter参数保存地址信息
              
              sFLASH_WriteBuffer(sFLASH_Tx_Buffer, PARAMETER_ADDRESS_W, 20);//擦除后立即回写parameter参数
              
              
              sFLASH_WriteBuffer((u8 *)&ImageNumber_store,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER, 4);//保存镜像号
              sFLASH_WriteBuffer((u8 *)&ImageNumber_store_a,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER_A, 4);//保存镜像号_A
              sFLASH_WriteBuffer((u8 *)&ImageNumber_store_b,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER_B, 4);//保存镜像号_B
              sFLASH_WriteBuffer((u8 *)&system_set_data_r.system_set_data_all,SYSTEM_SET_DATA_ADDRESS_W, 4);//写设置
              sFLASH_WriteBuffer((u8 *)&password_store,PARAMETER_ADDRESS_W + OFFSET_PASSWORD_VALUE, 4);//保存密码
              sFLASH_WriteBuffer((u8 *)&language_store,PARAMETER_ADDRESS_W + OFFSET_LANGUAGE, 4);//一个字等于4个字节，写保存的中英文语言          
              
              sFLASH_ReadBuffer(sFLASH_Rx_Buffer, FLASH_READ_ADDRESS, 100);//读参数信息
              
            }  
          }
          else
          {
            if(!f_opendir (&mulu,"0:/system"))
            {
              
            }
            else
            {
              tf_card_warning();
            }
          }               
        }
        else if(channel==1)
        {
          listselected_b=LISTBOX_GetSel(hItem);
          sprintf(sp_b,"0:/system/dat/HEAD%03u.BIN",listselected_b+listcount_b);
          
          if(!f_open(&fsrc, sp_b,  FA_READ))
          {
            f_read (&fsrc,buf_b,HEADER_LENGTH,&br);//关键性的修改buf中的数据
            f_close (&fsrc);
            
            aes_decode(buf_b,HEADER_LENGTH);        
            if(ImageNumber_store_b!=(listselected_b+listcount_b))//镜像切换了，和上次的镜像号不一样了，保存到单片机FLAH
            {
              
              ImageNumber_store_b=listselected_b+listcount_b;
              
              
              for(u16 i=0;i<100;i++)
              {
                parameter[i]=*(u8 *)(PARAMETER_ADDRESS_R+i);
                sFLASH_Tx_Buffer[i]=parameter[i];
              }
              sFLASH_EraseSector(PARAMETER_ADDRESS_W);//擦除parameter参数保存地址信息
              
              sFLASH_WriteBuffer(sFLASH_Tx_Buffer, PARAMETER_ADDRESS_W, 20);//擦除后立即回写parameter参数
              
              
              sFLASH_WriteBuffer((u8 *)&ImageNumber_store,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER, 4);//保存镜像号
              sFLASH_WriteBuffer((u8 *)&ImageNumber_store_a,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER_A, 4);//保存镜像号_A
              sFLASH_WriteBuffer((u8 *)&ImageNumber_store_b,PARAMETER_ADDRESS_W + OFFSET_IMAGE_NUMBER_B, 4);//保存镜像号_B
              sFLASH_WriteBuffer((u8 *)&system_set_data_r.system_set_data_all,SYSTEM_SET_DATA_ADDRESS_W, 4);//写设置
              sFLASH_WriteBuffer((u8 *)&password_store,PARAMETER_ADDRESS_W + OFFSET_PASSWORD_VALUE, 4);//保存密码
              sFLASH_WriteBuffer((u8 *)&language_store,PARAMETER_ADDRESS_W + OFFSET_LANGUAGE, 4);//一个字等于4个字节，写保存的中英文语言          
              
              sFLASH_ReadBuffer(sFLASH_Rx_Buffer, FLASH_READ_ADDRESS, 100);//读参数信息
              
            }  
          }
          else
          {
            if(!f_opendir (&mulu,"0:/system"))
            {
              
            }
            else
            {
              tf_card_warning();
            }
          }           
        }
        
        
        GUI_EndDialog(_hDialogControl_listbox, 0);
        _hDialogControl_offline_write=GUI_CreateDialogBox(_aFrameWinControl_offline_write, GUI_COUNTOF(_aFrameWinControl_offline_write), &_cbFrameWinControl_offline_write, WM_HBKWIN, 0, 0);  
        GUI_Exec();
        h_timer=WM_CreateTimer(_hDialogControl_offline_write, 0, 100, 0);
        CPU_CRITICAL_EXIT();	//退出临界区
        break;
        
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}

void draw_progress_bar(u8 i)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();	//进入临界区  
  WM_SelectWindow(_hDialogControl_offline_write);//必须选择窗口句柄，否则不显示
  // GUI_COLOR tmp_color;
  /*三角形 开始图标 三个顶点相对坐标*/
  GUI_POINT aPoints[] = {
    { 0, 0},
    { 0, 7},
    { 8, 3}
  };
  if(progress_bar_tmp!=i)
  {
    //tmp_color=GUI_GetColor();
    if(progress_bar_tmp==200)
    {
      GUI_SetColor(0x00F3F3F3); 
      GUI_ClearRect(10, 306, 30, 318);
      GUI_AA_DrawRoundedRect(10, 306, 30, 318, 6);
      
      GUI_ClearRect(35, 306, 230, 318);
      GUI_AA_DrawRoundedRect(35, 306, 230, 318, 6);
      i=0;
      mh=0;
    }
    
    GUI_ClearRect(12, 308, 28, 316);
    GUI_ClearRect(37, 308, 228, 316);
    
    progress_bar_tmp=i;
    
    
    GUI_SetColor(GUI_RED); 
    for(u8 g=0;g<i;g++)
    {
      GUI_FillRect(42+g*8, 309,46+g*8, 315);
    }
    
    
    if(i==0||i==23)//暂停图标
    {
      GUI_SetColor(GUI_RED); 
      for(u8 g=0;g<2;g++)
      {
        GUI_FillRect(16+g*5, 309,18+g*5, 315);
      }
    }
    else//开始后图标
    {
      GUI_SetColor(GUI_BLUE);
      
      /* 绘制多边形 这里绘制的是三角形开始图标*/
      GUI_FillPolygon(aPoints,              /* 指向要显示和填充的多边形相对坐标点数组 */
                      3, /* 点列表中指定的点数量 */
                      16,                  /* 原点的X位置 */
                      309);                 /* 原点的Y位置 */
    }
    
    // GUI_SetColor(tmp_color);
  }
  CPU_CRITICAL_EXIT();	//退出临界区  
}



void aes_encode(u8 * p_Buffer,u32 length_size)
{
  AES_Encrypt_Config_Init(16, 3000);
  for (u32 j = 0; j < length_size; j += 16)
  {
    //加密数据包
    AES_Encrypt_Calculate(p_Buffer);
    p_Buffer += 16;
  }
}


void aes_decode(u8 * p_Buffer,u32 length_size)
{
  AES_Decrypt_Config_Init(16, 3000);
  for (u32 j = 0; j < length_size; j += 16)
  {
    //解密数据包
    AES_Decrypt_Calculate(p_Buffer);
    p_Buffer += 16;
  }
}

#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
* @}
*/ 

/**
* @}
*/ 


/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "stdio.h"
#include "diskio.h"
#include "stm324xg_eval_sdio_sd.h"

#include <time.h>

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */

#define ATA		0
#define MMC		1
#define USB		2
#define SECTOR_SIZE 		512U /* Block Size in Bytes */



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
                         BYTE drv				/* Physical drive nmuber (0..) */
                           )
{
  if (SD_Init() == SD_OK)
  {
    //	printf("Init success\n");
    return STA_OK;           //初始化成功
  }
  
  return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Menu Disk Status                                                    */

DSTATUS disk_status (
                     BYTE drv		/* Physical drive nmuber (0..) */
                       )
{
  if(drv)
  {
    //	printf("Only support drv 0\n");
    return STA_NOINIT;
  }	
  
  return 0;
  //	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
                   BYTE drv,		/* Physical drive nmuber (0..) */
                   BYTE *buff,		/* Data buffer to store read data */
                   DWORD sector,	/* Sector address (LBA) */
                   BYTE count		/* Number of sectors to read (1..255) */
                     )
{
  
  
  SD_Error res = SD_OK;
  
  if (drv || !count)
  {    
    return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
  }
#if defined (SD_POLLING_MODE)  
  if(count==1)
  {
    __disable_irq();
    res = SD_ReadBlock(buff,sector << 9  ,SECTOR_SIZE);
    
    //res =  SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
    __enable_irq();     /* 开启总中断 */  
  }
  else
  {
    
    __disable_irq();
    res =  SD_ReadMultiBlocks(buff,sector << 9 ,SECTOR_SIZE,count); 
    
    //res =  SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
    __enable_irq();     /* 开启总中断 */
  }
#else
  if(count==1)
  {
    res = SD_ReadBlock(buff,sector << 9  ,SECTOR_SIZE);
    res =  SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }
  else
  {
    res =  SD_ReadMultiBlocks(buff,sector << 9 ,SECTOR_SIZE,count); 
    res =  SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }  
#endif  
  if(res == SD_OK)
  {
    return RES_OK;
  }
  else
  {
    return RES_ERROR;
  } 
  
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
                    BYTE drv,			/* Physical drive nmuber (0..) */
                    const BYTE *buff,	/* Data to be written */
                    DWORD sector,		/* Sector address (LBA) */
                    BYTE count			/* Number of sectors to write (1..255) */
                      )
{
  SD_Error res = SD_OK;
  
  if (drv || !count)
  {    
    return RES_PARERR;  //仅支持单磁盘操作，count不能等于0，否则返回参数错误
  }
  
  //printf("begin write\n");
#if defined (SD_POLLING_MODE)   
  if(count==1)
  {  
    __disable_irq();
    res =SD_WriteBlock((uint8_t *)buff,sector << 9 ,SECTOR_SIZE);
    
    //res =SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
    __enable_irq();     /* 开启总中断 */
  }
  else
  {          
    __disable_irq();
    res = SD_WriteMultiBlocks((uint8_t *)buff,sector <<9 ,SECTOR_SIZE,count);
    
    //res =SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
    __enable_irq();     /* 开启总中断 */
  }
#else 
  if(count==1)
  {  
    res =SD_WriteBlock((uint8_t *)buff,sector << 9 ,SECTOR_SIZE);
    res =SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }
  else
  {          
    res = SD_WriteMultiBlocks((uint8_t *)buff,sector <<9 ,SECTOR_SIZE,count);
    res =SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }  
#endif  
  if(res == SD_OK)
  {
    //	printf("write_ok\n");
    return RES_OK;
  }
  else
  {
    return RES_ERROR;
  }
  
  //return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
                    BYTE drv,		/* Physical drive nmuber (0..) */
                    BYTE ctrl,		/* Control code */
                    void *buff		/* Buffer to send/receive control data */
                      )
{
  return RES_OK;
}

/*DWORD get_fattime (void)
{
struct tm t;
DWORD fattime;
t = Time_GetCalendarTime();
t.tm_year -= 1980;		//年份改为1980年起
t.tm_mon++;         	//0-11月改为1-12月
t.tm_sec /= 2;      	//将秒数改为0-29

fattime = 0;
fattime = (t.tm_year << 25)|(t.tm_mon<<21)|(t.tm_mday<<16)|\
(t.tm_hour<<11)|(t.tm_min<<5)|(t.tm_sec);

return fattime;
}  */

DWORD get_fattime(void)
{
  return 0;
}

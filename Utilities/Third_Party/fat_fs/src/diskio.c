/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "stm324xg_eval_sdio_sd.h"


static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */


/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */

#define ATA		0
#define MMC		1
#define USB		2



/*-----------------------------------------------------------------------*/
/* Inicializes a Drive                                                    */

DSTATUS disk_initialize (BYTE drv)    /* Physical drive nmuber (0..) */
{
  DSTATUS stat = STA_NOINIT;
  
//  if(HCD_IsDeviceConnected(&USB_OTG_Core_dev))
//  {  
//    stat &= ~STA_NOINIT;
//  }
  	if (SD_Init() == SD_OK)
	{

	  	return STA_OK;           //初始化成功
	}
  return stat;
  
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;
	int result;

//	switch (drv) {
//	case ATA :
//		result = ATA_disk_status();
//		// translate the reslut code here
//
//		return stat;
//
//	case MMC :
//		result = MMC_disk_status();
//		// translate the reslut code here
//
//		return stat;
//
//	case USB :
//		result = USB_disk_status();
//		// translate the reslut code here
//
//		return stat;
//	}
//	return STA_NOINIT;
        
        
        
        return STA_OK;
        
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
	DRESULT res;
//	int result;
//
//	switch (drv) {
//	case ATA :
//		result = ATA_disk_read(buff, sector, count);
//		// translate the reslut code here
//
//		return res;
//
//	case MMC :
//		result = MMC_disk_read(buff, sector, count);
//		// translate the reslut code here
//
//		return res;
//
//	case USB :
//		result = USB_disk_read(buff, sector, count);
//		// translate the reslut code here
//
//		return res;
//	}
//	return RES_PARERR;
        
          SD_Error sdstatus = SD_OK;
        
        
          if (drv == 0)
  {
    	if(count==1)
		sdstatus =  SD_ReadBlock(buff,sector << 9  ,512);   
        else
    sdstatus =  SD_ReadMultiBlocks(buff, sector << 9, 512, count);
    
//    /* Check if the Transfer is finished */
//    sdstatus =  SD_WaitReadOperation();
//    while(SD_GetStatus() != SD_TRANSFER_OK);
    
    if (sdstatus == SD_OK)
    {
      return RES_OK;
    }
  }
  return RES_ERROR;
        
        
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
//	DRESULT res;
//	int result;
//
//	switch (drv) {
//	case ATA :
//		result = ATA_disk_write(buff, sector, count);
//		// translate the reslut code here
//
//		return res;
//
//	case MMC :
//		result = MMC_disk_write(buff, sector, count);
//		// translate the reslut code here
//
//		return res;
//
//	case USB :
//		result = USB_disk_write(buff, sector, count);
//		// translate the reslut code here
//
//		return res;
//	}
//	return RES_PARERR;
  
  
    SD_Error sdstatus = SD_OK;
  
  if (drv == 0)
  {
    if(count==1)
      sdstatus = SD_WriteBlock((uint8_t *)buff,sector << 9 ,512);
    else
    sdstatus = SD_WriteMultiBlocks((BYTE *)buff, sector << 9, 512, count);
//    /* Check if the Transfer is finished */
//    sdstatus = SD_WaitWriteOperation();
//    while(SD_GetStatus() != SD_TRANSFER_OK);     
//    
    if (sdstatus == SD_OK)
    {
      return RES_OK;
    }
  }
  return RES_ERROR;
  
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
//	DRESULT res;
//	int result;
//
//	switch (drv) {
//	case ATA :
//		// pre-process here
//
//		result = ATA_disk_ioctl(ctrl, buff);
//		// post-process here
//
//		return res;
//
//	case MMC :
//		// pre-process here
//
//		result = MMC_disk_ioctl(ctrl, buff);
//		// post-process here
//
//		return res;
//
//	case USB :
//		// pre-process here
//
//		result = USB_disk_ioctl(ctrl, buff);
//		// post-process here
//
//		return res;
//	}
//	return RES_PARERR;
  
  
  DRESULT res;
  SD_CardInfo SDCardInfo;    
  
  if (drv) return RES_PARERR;
  
  res = RES_ERROR;
  
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  
  switch (ctrl) {
  case CTRL_SYNC :		/* Make sure that no pending write process */
    
    res = RES_OK;
    break;
    
  case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
    if(drv == 0)
    {
      SD_GetCardInfo(&SDCardInfo);  
      *(DWORD*)buff = SDCardInfo.CardCapacity / 512; 
    }
    
    res = RES_OK;
    break;
    
  case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
    *(WORD*)buff = 512;
    res = RES_OK;
    break;
    
  case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
    if(drv == 0)
      *(DWORD*)buff = 512;
    else
      *(DWORD*)buff = 32;
    
    
    break;
    
    
  default:
    res = RES_PARERR;
  }
  
  
  
  return res;  
  
}

/**
   * @brief  Get Time from RTC
   * @param   None
   * @retval Time in DWORD
  */

DWORD get_fattime (void)
{
  return 0;
}


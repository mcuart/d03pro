
#include"ff.h"
#include"main.h"
#include <cJSON/cJSON.h>
#include "icp.h"
#include "includes.h"

//ICP_CLK    
#define PIN_ICPCLK_SET() GPIOB->BSRR=(((uint16_t)0x01)<<1)
#define PIN_ICPCLK_CLR() GPIOB->BSRR=((((uint16_t)0x01)<<1)<<16)

//ICP_DAT
#define PIN_ICPDAT_SET() GPIOB->BSRR=(((uint16_t)0x01)<<0)
#define PIN_ICPDAT_CLR() GPIOB->BSRR=((((uint16_t)0x01)<<0)<<16)

//ICP_RST
#define PIN_ICPRST_SET() GPIOB->BSRR=((((uint16_t)0x01)<<2)|(((uint16_t)0x01)<<5))
#define PIN_ICPRST_CLR() GPIOB->BSRR=(((((uint16_t)0x01)<<2)|(((uint16_t)0x01)<<5))<<16)



#define ICP_WRITE_BIT(bit) PIN_ICPDAT_OUT(bit);PIN_DELAY_FAST_ICP(2);PIN_ICPCLK_SET();PIN_DELAY_FAST_ICP(4);PIN_ICPCLK_CLR();PIN_DELAY_FAST_ICP(2)
#define ICP_READ_BIT(bit) PIN_ICPCLK_SET();PIN_DELAY_FAST_ICP(2);bit = PIN_ICPDAT_IN();PIN_ICPCLK_CLR();PIN_DELAY_FAST_ICP(2)     



//#define ICP_WRITE_BIT(bit) PIN_ICPDAT_OUT(bit);PIN_DELAY_FAST_ICP(1);PIN_ICPCLK_SET();PIN_DELAY_FAST_ICP(1);PIN_ICPCLK_CLR();PIN_DELAY_FAST_ICP(1)
//#define ICP_READ_BIT(bit) PIN_ICPCLK_SET();PIN_DELAY_FAST_ICP(1);bit = PIN_ICPDAT_IN();PIN_ICPCLK_CLR();PIN_DELAY_FAST_ICP(1)    




//#define ICP_WRITE_BIT(bit) PIN_ICPDAT_OUT(bit);PIN_ICPCLK_SET();PIN_ICPCLK_CLR();
//#define ICP_READ_BIT(bit) PIN_ICPCLK_SET();bit = PIN_ICPDAT_IN();PIN_ICPCLK_CLR();


//#define ICP_WRITE_BIT_1(bit) PIN_ICPDAT_OUT(bit);PIN_DELAY_FAST_ICP(2);PIN_ICPCLK_SET();PIN_DELAY_FAST_ICP(4);PIN_ICPCLK_CLR();PIN_DELAY_FAST_ICP(2)
//#define ICP_READ_BIT_1(bit) PIN_ICPCLK_SET();PIN_DELAY_FAST_ICP(2);bit = PIN_ICPDAT_IN();PIN_ICPCLK_CLR();PIN_DELAY_FAST_ICP(2)   

#define ICP_WRITE_BIT_1(bit) ICP_WRITE_BIT(bit)
#define ICP_READ_BIT_1(bit) ICP_READ_BIT(bit)  



static u8 FileData[4096];
static FIL FilSys;



void icp_io_config(void);
u8 display_verify_ldrom_ok(void);
uint8_t PIN_ICPDAT_IN (void);

void PIN_ICPDAT_OUT(uint8_t bit);
void PIN_ICPDAT_OUT_ENABLE(void);
void PIN_ICPDAT_OUT_DISABLE(void);


void PIN_DELAY_SLOW_ICP (uint32_t delay);
void PIN_DELAY_FAST_ICP (uint32_t delay);
static u8 ReadBuff[1024];

void icp_io_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;  
  /*使能GPIO时钟，使能复用功能IO时钟*/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);
  

  
  PIN_ICPRST_SET();
  PIN_ICPDAT_CLR();
  PIN_ICPCLK_SET();
  /*PB2 N76E003 RST复位脚输出*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//这里必须为开漏输出否则会导致5V供电情况下，下载失败。N76E芯片RST引脚内部自带上拉电阻最小50K
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /*PB1 N76E003 CLK输出*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /*PB0 N76E003 DAT输出*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出，高电平靠外接两个并联的1K电阻（合计为500R）
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;		
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出，高电平靠外接两个并联的1K电阻（合计为500R）
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}


//输入模式+（上拉/下拉）
void PIN_ICPDAT_OUT_DISABLE(void)
{
//  do {
//    GPIOB->CRL = (GPIOB->CRL & ~(((uint32_t)0x0F) << 24)) | (((uint32_t)0x8) << 24);
//    //GPIOB->BSRR = (((uint16_t)0x01) << 6);
//  } while (0);
  
  
  //上拉输入
  PIN_ICP_DAT_PORT->MODER = (PIN_ICP_DAT_PORT->MODER & ~PIN_MODE_MASK(PIN_ICP_DAT_PIN)) | PIN_MODE(0x0, PIN_ICP_DAT_PIN);
  PIN_ICP_DAT_PORT->OTYPER = (PIN_ICP_DAT_PORT->OTYPER & ~PIN_OTYPE_MASK(PIN_ICP_DAT_PIN)) | PIN_OTYPE(0x1, PIN_ICP_DAT_PIN);
  PIN_ICP_DAT_PORT->OSPEEDR = (PIN_ICP_DAT_PORT->OSPEEDR & ~PIN_OSPEED_MASK(PIN_ICP_DAT_PIN)) | PIN_OSPEED(0x3, PIN_ICP_DAT_PIN);
  PIN_ICP_DAT_PORT->PUPDR = (PIN_ICP_DAT_PORT->PUPDR & ~PIN_PUPD_MASK(PIN_ICP_DAT_PIN)) | PIN_PUPD(0x1, PIN_ICP_DAT_PIN);
}




uint8_t PIN_ICPDAT_IN (void)
{
  if (GPIOB->IDR & (((uint16_t)0x01) << 0))
    return 1;
  return 0;
}



//输出模式+开漏
void PIN_ICPDAT_OUT_ENABLE(void)
{
//  do {
//    GPIOB->CRL = (GPIOB->CRL & ~(((uint32_t)0x0F) << ((6) << 2))) | (((uint32_t)0x7) << ((6) << 2));
//    GPIOB->BSRR  = ((((uint16_t)0x01) << 0)<<16);
//  } while (0);
  
  //开漏输出低电平
  PIN_ICP_DAT_PORT->MODER = (PIN_ICP_DAT_PORT->MODER & ~PIN_MODE_MASK(PIN_ICP_DAT_PIN)) | PIN_MODE(0x1, PIN_ICP_DAT_PIN);//输出
  PIN_ICP_DAT_PORT->OTYPER = (PIN_ICP_DAT_PORT->OTYPER & ~PIN_OTYPE_MASK(PIN_ICP_DAT_PIN)) | PIN_OTYPE(0x1, PIN_ICP_DAT_PIN);//开漏
  PIN_ICP_DAT_PORT->OSPEEDR = (PIN_ICP_DAT_PORT->OSPEEDR & ~PIN_OSPEED_MASK(PIN_ICP_DAT_PIN)) | PIN_OSPEED(0x3, PIN_ICP_DAT_PIN);
  PIN_ICP_DAT_PORT->PUPDR = (PIN_ICP_DAT_PORT->PUPDR & ~PIN_PUPD_MASK(PIN_ICP_DAT_PIN)) | PIN_PUPD(0x0, PIN_ICP_DAT_PIN);//没有上下拉电阻  
  GPIOB->BSRR  = ((((uint16_t)0x01) << 0)<<16);
}



void PIN_ICPDAT_OUT(uint8_t bit)
{
  if (bit & 0x80)
    GPIOB->BSRR = (((uint16_t)0x01) << 0);
  else
    GPIOB->BSRR  = ((((uint16_t)0x01) << 0)<<16);
}


void PIN_DELAY_SLOW_ICP (uint32_t delay)
{
  System_Delay_ms(delay);
}


void PIN_DELAY_FAST_ICP (uint32_t delay)
{
  while (--delay)
  { }
}



//IAP 打开FLASH操作时序
void ICPCLK_Transfer(void)
{
  uint32_t val;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPRST_SET();
  PIN_ICPCLK_CLR();
  PIN_ICPDAT_CLR();
  
  
  
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(10);
  PIN_ICPRST_SET();  
  PIN_DELAY_SLOW_ICP(10);
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(10);
  PIN_ICPRST_SET();  
  PIN_DELAY_SLOW_ICP(10); 
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(10);  
  PIN_ICPRST_SET();  
  PIN_DELAY_SLOW_ICP(30);   
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(40);  
  PIN_ICPRST_SET();  
  PIN_DELAY_SLOW_ICP(30);
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(20);  
  PIN_ICPRST_SET();  
  PIN_DELAY_SLOW_ICP(10); 
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(10);  
  PIN_ICPRST_SET();  
  PIN_DELAY_SLOW_ICP(20); 
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(10);  
  PIN_ICPRST_SET();  
  PIN_DELAY_SLOW_ICP(20);
  PIN_ICPRST_CLR();
  PIN_DELAY_SLOW_ICP(10);
  val=0x5a;
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0xa5;
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }
  
  val=0x03;
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }
  CPU_CRITICAL_EXIT();
}




//IAP 关闭FLASH操作 并退出ICP模式时序 
void ICPCLK_Transfer_close_flash(void)
{
  uint32_t val;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  PIN_ICPRST_CLR();
  PIN_ICPCLK_CLR();
  PIN_ICPDAT_CLR();
  
  
  val=0x0F;
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0x78;
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }
  
  val=0xF0;
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }
  
  
  System_Delay_ms(1);
  PIN_ICPRST_SET();
  CPU_CRITICAL_EXIT();
}


u8 N76E003_IAP_CMD_READ_CID(void)
{
  u8 val;
  u8 bit;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0x00;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0x00;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x2C;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  
  val=0;
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  //读CID=0xDA
  for (u8 n = 8; n; n--)
  {
    val <<= 1;
    ICP_READ_BIT(bit);
    
    val  |= bit;
  }
  
  
  ICP_READ_BIT(bit);//dummp ack
  CPU_CRITICAL_EXIT();
  return val;
}



u8 N76E003_IAP_CMD_READ_DID_L(void)
{
  u8 val;
  u8 bit;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0x00;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0x00;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x30;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  
  val=0;
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  //读DID_L=0x50
  for (u8 n = 8; n; n--)
  {
    val <<= 1;
    ICP_READ_BIT(bit);
    
    val  |= bit;
    
  }
  
  
  ICP_READ_BIT(bit);//dummp ack
  CPU_CRITICAL_EXIT();
  return val;
}



u8 N76E003_IAP_CMD_READ_DID_H(void)
{
  u8 val;
  u8 bit;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0x00;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0x01;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x30;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  
  val=0;
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  //读DID_H=0x36
  for (u8 n = 8; n; n--)
  {
    val <<= 1;
    ICP_READ_BIT(bit);
    
    val  |= bit;
    
  }
  
  
  ICP_READ_BIT(bit);//dummp ack
  CPU_CRITICAL_EXIT();
  return val;
}


u8 N76E003_IAP_CMD_READ_DID_0x0002(void)
{
  u8 val;
  u8 bit;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0x00;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0x02;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x30;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  
  val=0;
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  //读DID_H=0x36
  for (u8 n = 8; n; n--)
  {
    val <<= 1;
    ICP_READ_BIT(bit);
    
    val  |= bit;
    
  }
  
  
  ICP_READ_BIT(bit);//dummp ack
  CPU_CRITICAL_EXIT();
  return val;
}

u8 N76E003_IAP_CMD_READ_DID_0x0003(void)
{
  u8 val;
  u8 bit;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0x00;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0x03;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x30;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  
  val=0;
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  //读DID_H=0x36
  for (u8 n = 8; n; n--)
  {
    val <<= 1;
    ICP_READ_BIT(bit);
    
    val  |= bit;
    
  }
  
  
  ICP_READ_BIT(bit);//dummp ack
  CPU_CRITICAL_EXIT();
  return val;
}






u16 N76E003_IAP_CMD_READ_DID_H_L(void)
{
  u16 DID=0;
  DID=N76E003_IAP_CMD_READ_DID_L();
  
  DID=(N76E003_IAP_CMD_READ_DID_H()<<8)|DID;
  
  return DID;
}

u16 N76E003_IAP_CMD_READ_DID_0x0002_0x0003(void)
{
  u16 DID=0;
  DID=N76E003_IAP_CMD_READ_DID_0x0002();
  
  DID=(N76E003_IAP_CMD_READ_DID_0x0003()<<8)|DID;
  
  return DID;
}


u8 N76E003_aprom_read_multi_byte_start(u8 * dst,u16 address_start,u32 length)
{
  u8 val=0;
  u8 bit=0;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=address_start>>8;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=(u8)address_start;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x00;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0;
  PIN_ICPDAT_CLR();
  // System_Delay_us(2);//增加延时20190505否则读到的第一个字节数字错误
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  PIN_ICPDAT_SET();
  //读CID=0xDA
  
  for(u32 i=0;i<length;i++)
  {
    
    for (u8 n = 8; n; n--)
    {
      val <<= 1;
      ICP_READ_BIT_1(bit);
      
      val  |= bit;
      
    }
    *(dst+i)=val;
    val=0;
    //这里强制写0，dummp数据
    PIN_ICPDAT_OUT_ENABLE();
    ICP_WRITE_BIT_1(0);
    
    PIN_ICPDAT_OUT_DISABLE();
  }
  CPU_CRITICAL_EXIT();
  return 0;
}



u8 N76E003_aprom_read_multi_byte_continue(u8 * dst,u32 length)
{ 
  u8 val=0;
  u8 bit=0;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  for(u32 i=0;i<length;i++)
  {
    for (u8 n = 8; n; n--)
    {
      val <<= 1;
      ICP_READ_BIT_1(bit);
      
      val  |= bit;
      
    }
    *(dst+i)=val;
    val=0;
    //这里强制写0，dummp数据
    PIN_ICPDAT_OUT_ENABLE();
    ICP_WRITE_BIT_1(0);
    PIN_ICPDAT_OUT_DISABLE();
  }
  CPU_CRITICAL_EXIT();
  return 0;
}


u8 N76E003_aprom_write_byte_32(u32 address,u8 *src,u32 length)
{
  u8 val;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=address>>8;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=(u8)address;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x84;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  u8 tmp_value=0;
  
  System_Delay_ms(1);//增加延时防止第一个字节变成0xFF
  
  
  //紧接着写第一个字节
  tmp_value=*(src+0);
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(tmp_value);
    tmp_value <<= 1;
  }
  
  
  
  
  //  for(u8 i=1;i<32;i++)
  //  {
  //    System_Delay_us(65);
  //    
  //    ICP_WRITE_BIT(0);
  //    
  //    for (u8 n = 8; n; n--) {
  //      ICP_WRITE_BIT(*(src+i));
  //      *(src+i) <<= 1;
  //    }
  //  }
  
  
  for(u32 i=1;i<length;i++)
  {
    System_Delay_us(65);
    
    ICP_WRITE_BIT(0);
    tmp_value=*(src+i);
    for (u8 n = 8; n; n--) {
      ICP_WRITE_BIT(tmp_value);
      tmp_value <<= 1;
    }
  } 
  
  
  System_Delay_us(65);
  ICP_WRITE_BIT(1);
  System_Delay_ms(5);
  System_Delay_ms(100);
  PIN_ICPDAT_CLR();
  System_Delay_us(10);
  PIN_ICPRST_SET();
  System_Delay_ms(10);
  PIN_ICPRST_CLR();
  System_Delay_ms(20);
  CPU_CRITICAL_EXIT();
  return val;
}



u8 N76E003_aprom_write_byte_32_start(u32 address,u8 *src)
{
  u8 val;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=address>>8;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=(u8)address;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x84;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  u8 tmp_value=0;
  
  System_Delay_ms(1);//增加延时防止第一个字节变成0xFF
  
  
  //紧接着写第一个字节
  tmp_value=*(src+0);
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(tmp_value);
    tmp_value <<= 1;
  }
  CPU_CRITICAL_EXIT();
  return val;
}




void N76E003_aprom_write_byte_32_start_continue(u32 address,u8 *src,u32 length)
{
  // u8 val;
  
  u8 tmp_value=0;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  for(u32 i=0;i<length;i++)
  {
    System_Delay_us(6);
    System_Delay_us(6);//延时从20多us增加到40多us解决MS51FB9AE不能烧录问题
    ICP_WRITE_BIT(0);
    
    tmp_value=*(src+i);
    for (u8 n = 8; n; n--) {
      ICP_WRITE_BIT(tmp_value);
      tmp_value <<= 1;
    }
  } 
  CPU_CRITICAL_EXIT();
  
  //  System_Delay_us(65);
  //  ICP_WRITE_BIT(1);
  //  System_Delay_ms(5);
  //  System_Delay_ms(100);
  //  PIN_ICPDAT_CLR();
  //  System_Delay_us(10);
  //  PIN_ICPRST_SET();
  //  System_Delay_ms(10);
  //  PIN_ICPRST_CLR();
  //  System_Delay_ms(20);
  // return val;
}




u8 N76E003_aprom_write_byte_32_start_continue_end(void)
{
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  System_Delay_us(65);
  ICP_WRITE_BIT(1);
  System_Delay_ms(5);
  System_Delay_ms(100);
  PIN_ICPDAT_CLR();
  System_Delay_us(10);
  PIN_ICPRST_SET();
  System_Delay_ms(10);
  PIN_ICPRST_CLR();
  System_Delay_ms(20);
  CPU_CRITICAL_EXIT();
  return 0;
}




u8 N76E003_aprom_write_multi_byte(u8 * src,u32 address_start,u32 length)
{
  //  for(u32 i=0;i<length;)
  //  {
  //    N76E003_aprom_write_byte_32(address_start+i,src+i);
  //    i+=32;
  //  }
  
  N76E003_aprom_write_byte_32(address_start,src,length);
  
  
  
  
  return 0;
}


u32 ic=0;
//读过配置字节以后发现又写入了未知目的的6个字节
//0xB2 0x3C 0xFC 0x4D 0xC3 0x03

//FOEN=1说面是写操作，FOEN=0说面是读操作
//FCEN统一为0
//擦除整个芯片的IAP命令，表中没有列出20200203
u8 N76E003_ldrom_aprom_mass_erase(void)
{
  u8 val;
  u8 bit;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0xC0;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0xA5;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=0xA5;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x98;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  
  
  val=0xFF;//IAPFD[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  System_Delay_ms(50);//延时50ms否则aprom擦除会失败
  //一个页擦除需要5ms  18K  128字节/页   144页  5msx144=720毫秒，实际不采用全片擦除指令不需要这么久50毫秒即可。
  
  ICP_READ_BIT(bit);
  
  
  CPU_CRITICAL_EXIT();
  
  return bit;
}




u8 N76E003_config_read(u8 *dst)
{
  u8 val;
  u8 bit;
  u16 address=0x0000;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0xC0;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=address>>8;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=(u8)address;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x00;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=0;
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  
  
  for(u8 i=0;i<8;i++)
  {
    for (u8 n = 8; n; n--)
    {
      val <<= 1;
      ICP_READ_BIT(bit);
      
      val  |= bit;
      
    }
    *(dst+i)=val;
    val=0;
    
    //这里强制写0，dummp数据
    PIN_ICPDAT_OUT_ENABLE();
    ICP_WRITE_BIT(0);
    PIN_ICPDAT_OUT_DISABLE();
  }
  
  
  
  CPU_CRITICAL_EXIT();
  
  
  return val;
}




//20200203
u8 N76E003_aprom_sector_erase(u32 address)
{
  u8 val;
  u8 bit;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0x00;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=address>>8;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=(u8)address;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x88;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  u8 tmp_value=0;
  
  System_Delay_ms(1);//增加延时防止第一个字节变成0xFF
  
  
  //紧接着写第一个字节
  //tmp_value=*(src+0);  
  tmp_value=0xFF;
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(tmp_value);
    tmp_value <<= 1;
  }
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  System_Delay_ms(5);//延时50ms否则aprom擦除会失败
  //一个页擦除需要5ms  18K  128字节/页   144页  5msx144=720毫秒，实际不采用全片擦除指令不需要这么久50毫秒即可。
  
  ICP_READ_BIT(bit);
  
  CPU_CRITICAL_EXIT();
  return val;
}

////20200203
//u8 N76E003_ldrom_sector_erase(u32 address)
//{
//  u8 val;
//  u8 bit;
//
//  PIN_ICPDAT_OUT_ENABLE();
//  val=0x40;//IAPB[1:0]
//  
//  for (u8 n = 2; n; n--) {
//    ICP_WRITE_BIT(val);
//    val <<= 1;
//  }	
//  
//  
//  val=address>>8;//IAPA[15:8]
//  
//  for (u8 n = 8; n; n--) {
//    ICP_WRITE_BIT(val);
//    val <<= 1;
//  }	
//  
//  val=(u8)address;//IAPA[7:0]
//  
//  for (u8 n = 8; n; n--) {
//    ICP_WRITE_BIT(val);
//    val <<= 1;
//  }	
//  
//  
//  
//  val=0x88;//FOEN FCEN FCTRL[3:0]
//  for (u8 n = 6; n; n--) {
//    ICP_WRITE_BIT(val);
//    val <<= 1;
//  }	
//  
//  
//  u8 tmp_value=0;
//  
//  System_Delay_ms(1);//增加延时防止第一个字节变成0xFF
//  
//  
//  //紧接着写第一个字节
//  //tmp_value=*(src+0);  
//  tmp_value=0xFF;
//  for (u8 n = 8; n; n--) {
//    ICP_WRITE_BIT(tmp_value);
//    tmp_value <<= 1;
//  }
//  
//    //开始读
//  PIN_ICPDAT_OUT_DISABLE();
//  System_Delay_ms(5);//延时50ms否则aprom擦除会失败
//  //一个页擦除需要5ms  18K  128字节/页   144页  5msx144=720毫秒，实际不采用全片擦除指令不需要这么久50毫秒即可。
//  
//  ICP_READ_BIT(bit);
//  return val;
//}


//20200203
u8 N76E003_config_all_erase(void)
{
  u8 val;
  u8 bit;
  u32 address=0;
  
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0xC0;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=address>>8;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=(u8)address;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x88;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  u8 tmp_value=0;
  
  System_Delay_ms(1);//增加延时防止第一个字节变成0xFF
  
  
  //紧接着写第一个字节
  //tmp_value=*(src+0);  
  tmp_value=0xFF;
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(tmp_value);
    tmp_value <<= 1;
  }
  
  //开始读
  PIN_ICPDAT_OUT_DISABLE();
  System_Delay_ms(5);//延时50ms否则aprom擦除会失败
  //一个页擦除需要5ms  18K  128字节/页   144页  5msx144=720毫秒，实际不采用全片擦除指令不需要这么久50毫秒即可。
  
  ICP_READ_BIT(bit);
  
  CPU_CRITICAL_EXIT();
  return val;
}

extern FIL FilSys;
extern char sp1[100];
extern u16 ImageNumber_store;
extern cJSON *root;

u8 N76E003_erase_aprom(u32 address)
{
  u32 file_size=0;
  u32 i=0;
  sprintf(sp1,"0:/system/dat/APP%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  file_size=FilSys.fsize;
  f_close(&FilSys); 
  while(i<file_size)
  {
    N76E003_aprom_sector_erase(address+i);
    i+=128;
  }
  return 0;
}


u8 N76E003_erase_ldrom(u32 address)
{
  u32 file_size=0;
  u32 i=0;
  sprintf(sp1,"0:/system/dat/LDD%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  file_size=FilSys.fsize;
  f_close(&FilSys); 
  while(i<file_size)
  {
    N76E003_aprom_sector_erase(address+i);
    i+=128;
  }
  return 0;
}



//u8 N76E003_erase_ldrom(u32 address)
//{
//  u32 file_size=0;
//  u32 i=0;
//  sprintf(sp1,"0:/system/dat/LDD%03d.BIN",ImageNumber_store);
//  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
//  file_size=FilSys.fsize;
//  f_close(&FilSys); 
//  while(i<file_size)
//  {
//      N76E003_ldrom_sector_erase(address+i);
//      i+=128;
//  }
//  return 0;
//}



u8 N76E003_config_write(u8 *src)
{
  u8 val;
  u8 bit;
  u16 address=0x0000;
  CPU_SR_ALLOC();
  CPU_CRITICAL_ENTER();
  PIN_ICPDAT_OUT_ENABLE();
  val=0xC0;//IAPB[1:0]
  
  for (u8 n = 2; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  val=address>>8;//IAPA[15:8]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  val=(u8)address;//IAPA[7:0]
  
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  
  val=0x84;//FOEN FCEN FCTRL[3:0]
  for (u8 n = 6; n; n--) {
    ICP_WRITE_BIT(val);
    val <<= 1;
  }	
  
  
  u8 tmp_value=0;
  
  System_Delay_ms(1);//增加延时防止第一个字节变成0xFF
  
  
  //紧接着写第一个字节
  tmp_value=*(src+0);
  for (u8 n = 8; n; n--) {
    ICP_WRITE_BIT(tmp_value);
    tmp_value <<= 1;
  }
  
  
  
  for(u8 i=1;i<8;i++)
  {
    System_Delay_us(65);
    
    ICP_WRITE_BIT(0);
    tmp_value=*(src+i);
    for (u8 n = 8; n; n--) {
      ICP_WRITE_BIT(tmp_value);
      tmp_value <<= 1;
    }
  } 
  
  
  System_Delay_us(65);
  ICP_WRITE_BIT(1);
  
  System_Delay_ms(5);
  
  System_Delay_ms(100);
  
  PIN_ICPDAT_CLR();
  
  PIN_ICPCLK_SET();
  
  System_Delay_ms(10);
  PIN_ICPCLK_CLR();
  System_Delay_ms(20);
  
  
  CPU_CRITICAL_EXIT();
  return val;
}



u8 N76E003_write_rollingcode(void)
{
  u8 stm8_rolling_code_value_array[4]={0,0,0,0};
  u32 rolling_code_address=0;
  u8 rolling_code_width=0;
  
  if((u8)cJSON_GetObjectItem(root, "rolling_code_endian")->valueint==1)//大端模式
  {
    switch(cJSON_GetObjectItem(root, "rolling_code_byte_width")->valueint)
    {
    case 1:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      break;
    case 2:
      stm8_rolling_code_value_array[1]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[0]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      break;
    case 3:
      stm8_rolling_code_value_array[2]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[0]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      break;
    case 4:
      stm8_rolling_code_value_array[3]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[2]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      stm8_rolling_code_value_array[0]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>24; 
      break;
    }
  }
  else//小端模式
  {
    switch(cJSON_GetObjectItem(root, "rolling_code_byte_width")->valueint)
    {
    case 1:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      break;
    case 2:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      break;
    case 3:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[2]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      break;
    case 4:
      stm8_rolling_code_value_array[0]= (u8)cJSON_GetObjectItem(root, "rolling_code")->valuedouble;
      stm8_rolling_code_value_array[1]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>8;
      stm8_rolling_code_value_array[2]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>16;
      stm8_rolling_code_value_array[3]= ((u32)cJSON_GetObjectItem(root, "rolling_code")->valuedouble)>>24; 
      break;
    }
  }
  
  rolling_code_address=(u32)cJSON_GetObjectItem(root, "rolling_code_address")->valuedouble;
  rolling_code_width=(u8)cJSON_GetObjectItem(root, "rolling_code_byte_width")->valueint; 
  
  
  
  N76E003_aprom_write_byte_32_start(rolling_code_address,&stm8_rolling_code_value_array[0]);//写一个头字节先
  
  
  
  
  N76E003_aprom_write_byte_32_start_continue(rolling_code_address,&stm8_rolling_code_value_array[1],rolling_code_width-1);
  
  
  
  
  N76E003_aprom_write_byte_32_start_continue_end();
  
  
  
  return 0;
}



u8 N76E003_write_aprom(void)
{
  u32 blk=0;
  u32 aes_key=3000;
  u32 ReadCount=0;
  u32 app_base=0;
  
  
  sprintf(sp1,"0:/system/dat/APP%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  
  
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)//先读取第一个字节
  {
    return 1;
  }
  
  
  
  u8 *Point=FileData;
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),5)->valuedouble;
    AES_Decrypt_Config_Init(16,aes_key);
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
    
    
    Point = FileData;
  }
  
  
  
  
  app_base=(u32)cJSON_GetObjectItem(root, "app_base")->valuedouble;
  N76E003_aprom_write_byte_32_start(app_base,Point);//写一个头字节先
  blk++;
  Point++;
  N76E003_aprom_write_byte_32_start_continue(app_base+blk,Point,ReadCount-1);
  
  
  if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
  {
    return 9;
  }
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 10;
  }
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
  }
  
  
  while(ReadCount)
  {
    Point=FileData;
    
    N76E003_aprom_write_byte_32_start_continue(app_base+blk,Point,ReadCount);
    
    //    ///////////////////////////////////////////////开始校验FLASH     
    //    if((u8)cJSON_GetObjectItem(root, "verify_aprom")->valueint == 1)
    //    {
    //      //      N76E003_aprom_read_multi_byte(ReadBuff,app_base+blk,ReadCount);
    //      //      for(u16 i=0;i<ReadCount;i++)
    //      //      {
    //      //        if(*(Point+i)!=*(ReadBuff+i))
    //      //        {
    //      //          return 2;
    //      //        }
    //      //      }
    //    }
    //    ///////////////////////////////////////////校验结束    
    
    blk+=ReadCount;
    
    
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
      
      
      if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
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
  
  N76E003_aprom_write_byte_32_start_continue_end();
  f_close(&FilSys);
  
  
  return 0;
}



u8 N76E003_write_ldrom(u16 address_start)
{
  u32 blk=0;
  u32 aes_key=3000;
  u32 ReadCount=0;
  u32 app_base=0;
  
  
  sprintf(sp1,"0:/system/dat/LDD%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  
  
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 1;
  }
  
  
  
  u8 *Point=FileData;
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),5)->valuedouble;
    AES_Decrypt_Config_Init(16,aes_key);
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
    
    Point=FileData;
  }
  
  app_base=address_start;
  N76E003_aprom_write_byte_32_start(app_base,Point);//写一个头字节先
  blk++;
  Point++;
  N76E003_aprom_write_byte_32_start_continue(app_base+blk,Point,ReadCount-1);
  
  
  if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)//移动指针到上次读的末尾
  {
    return 9;
  }
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 10;
  }
  
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
  }
  
  
  
  
  while(ReadCount)
  {
    Point=FileData;
    
    N76E003_aprom_write_byte_32_start_continue(app_base+blk,Point,ReadCount);
    
    ///////////////////////////////////////////////开始校验FLASH     
    //    if((u8)cJSON_GetObjectItem(root, "verify_aprom")->valueint == 1)
    //    {
    //      //        cmdrtv=SWIM_ROTF((u32)cJSON_GetObjectItem(root, "aprom_base")->valuedouble+(blk*(u8)cJSON_GetObjectItem(root, "page_size")->valueint),(u8)cJSON_GetObjectItem(root, "page_size")->valueint,ReadBuff);
    //      //        
    //      //        
    //      //        if (cmdrtv>0)
    //      //        {
    //      //          SWIM_DeInit();
    //      //          return 2;
    //      //        }
    //      //        
    //      //        
    //      //        for(u8 i=0;i<(u8)cJSON_GetObjectItem(root, "page_size")->valueint;i++)
    //      //        {
    //      //          if(*(Point+i)!=*(ReadBuff+i))
    //      //          {
    //      //            SWIM_DeInit();
    //      //            return 2;
    //      //          }
    //      //        }
    //    }
    ///////////////////////////////////////////校验结束    
    
    blk+=ReadCount;
    
    
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
      
      
      if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
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
  
  N76E003_aprom_write_byte_32_start_continue_end();
  f_close(&FilSys);
  
  
  return 0;
}



u8 N76E003_verify_ldrom(u16 address_start)
{
  u32 blk=0;
  u32 aes_key=3000;
  u32 ReadCount=0;
  u8 *Point=FileData;
  
  
  sprintf(sp1,"0:/system/dat/LDD%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  
  
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 1;
  }
  
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),5)->valuedouble;
    
    AES_Decrypt_Config_Init(16,aes_key);
    
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
    
    Point=FileData;
  }
  
  
  N76E003_aprom_read_multi_byte_start(ReadBuff,address_start,ReadCount);
  for(u16 i=0;i<ReadCount;i++)
  {
    if((*(Point+i)&0x7F)!=(*(ReadBuff+i)&0x7F))
    {
      return 2;
    }
  }
  
  
  
  if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
  {
    return 9;
  }
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 10;
  }
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    
    
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      Point += 16;
    }
    
    Point = FileData;
  }
  
  
  
  
  while(ReadCount)
  {
    Point=FileData;
    N76E003_aprom_read_multi_byte_continue(ReadBuff,ReadCount); 
    ///////////////////////////////////////////////开始校验FLASH     
    
    for(u16 i=0;i<ReadCount;i++)
    {
      if((*(Point+i)&0x7F)!=(*(ReadBuff+i)&0x7F))
      {
        return 2;
      }
    }
    ///////////////////////////////////////////校验结束    
    
    blk+=ReadCount;
    
    
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
      
      
      if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
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
  f_close(&FilSys);
  
  
  return 0;
}




u8 N76E003_verify_aprom(u16 address_start)
{
  u32 blk=0;
  u32 aes_key=3000;
  u32 ReadCount=0;
  u32 app_base=0;
  //u8 ReadBuff[1024];
  
  sprintf(sp1,"0:/system/dat/APP%03d.BIN",ImageNumber_store);
  f_open(&FilSys, sp1,  FA_READ); //根据buf[1]中的镜像号来进行读文件操作
  
  
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 1;
  }
  
  
  
  u8 *Point=FileData;
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    aes_key=(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),4)->valuedouble*60+(u32)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "aes_key"),5)->valuedouble;
    AES_Decrypt_Config_Init(16,aes_key);
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      
      Point += 16;
    }
    
    Point=FileData;
  }
  
  N76E003_aprom_read_multi_byte_start(ReadBuff,address_start,ReadCount);
  
  for(u16 i=1;i<ReadCount;i++)
  {
    if((*(Point+i)&0x7F)!=(*(ReadBuff+i)&0x7F))
    {
      return 2;
    }
  }
  
  
  
  if(f_lseek(&FilSys,FilSys.fptr)!=FR_OK)
  {
    return 9;
  }
  if(f_read(&FilSys,FileData,1024,&ReadCount)!=FR_OK)
  {
    return 10;
  }
  
  
  if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
  {
    Point = FileData;
    
    
    for (u32 j = 0; j < ReadCount; j += 16)
    {
      //解密数据包
      AES_Decrypt_Calculate(Point);
      Point += 16;
    }
    
    Point = FileData;
  }
  
  
  
  
  while(ReadCount)
  {
    Point=FileData;
    N76E003_aprom_read_multi_byte_continue(ReadBuff,ReadCount); 
    ///////////////////////////////////////////////开始校验FLASH     
    
    for(u16 i=0;i<ReadCount;i++)
    {
      if((*(Point+i)&0x7F)!=(*(ReadBuff+i)&0x7F))
      {
        return 2;
      }
    }
    ///////////////////////////////////////////校验结束    
    
    blk+=ReadCount;
    
    
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
      
      
      if( (u8)cJSON_GetObjectItem(root, "data_encrypt")->valueint==1)//AES加密
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
  f_close(&FilSys);
  
  
  return 0;
}

u8 conf[8];
u8 conf_w_0[8]={0xCB,0xFF,0x53,0xFF,0xFF,0xFF,0xFF,0xFF};

u8 cid=0;
u16 did=0;


u8 N76E003_verify_config(void)
{
  u32 blk=0;
  u32 aes_key=3000;
  u32 ReadCount=0;
  u32 app_base=0;
  //u8 ReadBuff[1024];
  
  icp_io_config();
  ICPCLK_Transfer();
  cid=N76E003_IAP_CMD_READ_CID();
  did=N76E003_IAP_CMD_READ_DID_H_L();
  
  N76E003_config_read(conf);
  
  
  ICPCLK_Transfer_close_flash();
  
  
  for(u8 i=0;i<(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),0)->valueint;i++)
  {
    conf_w_0[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),i+1)->valueint;
  }
  
  for(u8 i=0;i<(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),0)->valueint;i++)
  {
    if(conf_w_0[i]!=conf[i])
    {
      return 1;
    }
  }
  
  
  
  return 0;
}



u8 N76E003_Auto_detect(void)
{
  icp_io_config();
  ICPCLK_Transfer();
  cid=N76E003_IAP_CMD_READ_CID();
  did=N76E003_IAP_CMD_READ_DID_H_L();
  
  ICPCLK_Transfer_close_flash();
  //  if((cid==0xDA)||(did==0x3650)||(did==0x4B21))
  //  {
  //    return 0;
  //  }
  if((cid==0xDA)||(did!=0xFFFF))
  {
    return 0;
  }
  System_Delay_ms(100);
  return 1;
}



u8 N76E003_Auto_detect_end(void)
{
  icp_io_config();
  ICPCLK_Transfer();
  cid=N76E003_IAP_CMD_READ_CID();
  did=N76E003_IAP_CMD_READ_DID_H_L();
  
  ICPCLK_Transfer_close_flash();
  // while(cid==0xDA||did==0x3650||(did==0x4B21))
  while(cid==0xDA||did!=0xFFFF)
  {
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    
    ICPCLK_Transfer_close_flash();
  }
  return 0;
}

//




u8 N76E003_TEST(void)
{
  u8 ret=0;
  icp_io_config();
  ICPCLK_Transfer();
  cid=N76E003_IAP_CMD_READ_CID();
  did=N76E003_IAP_CMD_READ_DID_H_L();
  
  
  
  ICPCLK_Transfer_close_flash();
  
  
  icp_io_config();
  ICPCLK_Transfer();
  cid=N76E003_IAP_CMD_READ_CID();
  did=N76E003_IAP_CMD_READ_DID_H_L();
  
  //  if((cid!=0xDA)&&(did!=0x3650))
  //  {
  //    if(did!=0x4B21)
  //      return 1;
  //  }
  if((cid==0xFF)&&(did==0xFFFF))
  {
    // if(did!=0x4B21)
    return 1;
  }
  display_detect_chip_id_code_ok(cid<<16|did);
  
  
  
  /****  
  did=N76E003_IAP_CMD_READ_DID_0x0002_0x0003();
  did=N76E003_IAP_CMD_READ_DID_0x0002_0x0003();
  ********/  
  
  
  
  
  //  N76E003_aprom_sector_erase(0);
  //  
  //  N76E003_aprom_sector_erase(5*128);
  //  
  //  N76E003_aprom_sector_erase(8*128);
  
  //擦除芯片
  if((u8)cJSON_GetObjectItem(root, "mass_erase")->valueint==1)
  {
    N76E003_ldrom_aprom_mass_erase();
    display_clear_chip_ok();
    
  }
  ICPCLK_Transfer_close_flash();
  
  
  
  
  //写LDROM
  if((u8)cJSON_GetObjectItem(root, "write_ldrom")->valueint==1)
  {
    if((u8)cJSON_GetObjectItem(root, "mass_erase")->valueint==0)
    {
      icp_io_config();
      ICPCLK_Transfer();
      cid=N76E003_IAP_CMD_READ_CID();
      did=N76E003_IAP_CMD_READ_DID_H_L();
      
      
      conf_w_0[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),2)->valueint;
      
      conf_w_0[1]=conf_w_0[1]&0x07;
      switch(did)
      {
      case 0x3650://N76E003
        switch(conf_w_0[1])
        {
        case 0x07://APROM 18K
          
          break;
        case 0x06://APROM 17K
          N76E003_erase_ldrom(17*1024);
          break;
          
        case 0x05://APROM 16K
          N76E003_erase_ldrom(16*1024);
          break;
        case 0x04://APROM 15K
          N76E003_erase_ldrom(15*1024);
          break;
        case 0x03://APROM 14K
          N76E003_erase_ldrom(14*1024);
          break;
        }   
        break;
      case 0x4B21://MS51FB9AE
      case 0x4721://ML51FB9AE
        switch(conf_w_0[1])
        {
        case 0x07://APROM 16K
          
          break;
        case 0x06://APROM 15K
          N76E003_erase_ldrom(15*1024);
          break;
          
        case 0x05://APROM 14K
          N76E003_erase_ldrom(14*1024);
          break;
        case 0x04://APROM 13K
          N76E003_erase_ldrom(13*1024);
          break;
        case 0x03://APROM 12K
          N76E003_erase_ldrom(12*1024);
          break;
        } 
        break;
        
      default:
        switch(conf_w_0[1])
        {
        case 0x07://APROM 16K
          
          break;
        case 0x06://APROM 15K
          N76E003_erase_ldrom(15*1024);
          break;
          
        case 0x05://APROM 14K
          N76E003_erase_ldrom(14*1024);
          break;
        case 0x04://APROM 13K
          N76E003_erase_ldrom(13*1024);
          break;
        case 0x03://APROM 12K
          N76E003_erase_ldrom(12*1024);
          break;
        }       
        break;
        
      }
      
      //N76E003_erase_ldrom(0);
      
      //IAP 关闭FLASH操作时序
      ICPCLK_Transfer_close_flash();
      // display_write_aprom_ok();      
    }
    
    
    
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    
    conf_w_0[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),2)->valueint;
    
    conf_w_0[1]=conf_w_0[1]&0x07;
    switch(did)
    {
    case 0x3650://N76E003
      switch(conf_w_0[1])
      {
      case 0x07://APROM 18K
        
        break;
      case 0x06://APROM 17K
        N76E003_write_ldrom(17*1024);
        break;
        
      case 0x05://APROM 16K
        N76E003_write_ldrom(16*1024);
        break;
      case 0x04://APROM 15K
        N76E003_write_ldrom(15*1024);
        break;
      case 0x03://APROM 14K
        N76E003_write_ldrom(14*1024);
        break;
      }              
      break;
    case 0x4B21://MS51FB9AE
    case 0x4721://ML51FB9AE
      switch(conf_w_0[1])
      {
      case 0x07://APROM 16K
        
        break;
      case 0x06://APROM 15K
        N76E003_write_ldrom(15*1024);
        break;
        
      case 0x05://APROM 14K
        N76E003_write_ldrom(14*1024);
        break;
      case 0x04://APROM 13K
        N76E003_write_ldrom(13*1024);
        break;
      case 0x03://APROM 12K
        N76E003_write_ldrom(12*1024);
        break;
      }     
      break;
      
    default:
      switch(conf_w_0[1])
      {
      case 0x07://APROM 16K
        
        break;
      case 0x06://APROM 15K
        N76E003_write_ldrom(15*1024);
        break;
        
      case 0x05://APROM 14K
        N76E003_write_ldrom(14*1024);
        break;
      case 0x04://APROM 13K
        N76E003_write_ldrom(13*1024);
        break;
      case 0x03://APROM 12K
        N76E003_write_ldrom(12*1024);
        break;
      }  
      break;
    }
    
    
    //    //IAP 关闭FLASH操作时序
    ICPCLK_Transfer_close_flash();
    display_write_ldrom_ok();
  }
  
  
  //校验LDROM
  if((u8)cJSON_GetObjectItem(root, "verify_ldrom")->valueint==1)
  {
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    
    conf_w_0[1]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),2)->valueint;
    
    conf_w_0[1]=conf_w_0[1]&0x07;
    
    switch(did)
    {
      
    case 0x3650://N76E003
      switch(conf_w_0[1])
      {
      case 0x07://APROM 18K
        
        break;
      case 0x06://APROM 17K
        ret=N76E003_verify_ldrom(17*1024);
        break;
        
      case 0x05://APROM 16K
        ret=N76E003_verify_ldrom(16*1024);
        break;
      case 0x04://APROM 15K
        ret=N76E003_verify_ldrom(15*1024);
        break;
      case 0x03://APROM 14K
        ret=N76E003_verify_ldrom(14*1024);
        break;
      }            
      break;
    case 0x4B21://MS51FB9AE
    case 0x4721://ML51FB9AE
      switch(conf_w_0[1])
      {
      case 0x07://APROM 16K
        
        break;
      case 0x06://APROM 15K
        ret=N76E003_verify_ldrom(15*1024);
        break;
        
      case 0x05://APROM 14K
        ret=N76E003_verify_ldrom(14*1024);
        break;
      case 0x04://APROM 13K
        ret=N76E003_verify_ldrom(13*1024);
        break;
      case 0x03://APROM 12K
        ret=N76E003_verify_ldrom(12*1024);
        break;
      }
      break;
      
    default:
      switch(conf_w_0[1])
      {
      case 0x07://APROM 16K
        
        break;
      case 0x06://APROM 15K
        ret=N76E003_verify_ldrom(15*1024);
        break;
        
      case 0x05://APROM 14K
        ret=N76E003_verify_ldrom(14*1024);
        break;
      case 0x04://APROM 13K
        ret=N76E003_verify_ldrom(13*1024);
        break;
      case 0x03://APROM 12K
        ret=N76E003_verify_ldrom(12*1024);
        break;
      }
      break;
    }
    
    
    //    //IAP 关闭FLASH操作时序
    ICPCLK_Transfer_close_flash();
    if(ret==0)
      display_verify_ldrom_ok();
    else
      return 2;
  }
  
  
  
  //写APROM
  if((u8)cJSON_GetObjectItem(root, "write_aprom")->valueint==1)
  {
    if((u8)cJSON_GetObjectItem(root, "mass_erase")->valueint==0)
    {
      icp_io_config();
      ICPCLK_Transfer();
      cid=N76E003_IAP_CMD_READ_CID();
      did=N76E003_IAP_CMD_READ_DID_H_L();
      
      if(N76E003_erase_aprom(0)==0)
      {
        //IAP 关闭FLASH操作时序
        ICPCLK_Transfer_close_flash();
        // display_write_aprom_ok();
      }
      else
      {
        return 2;
      }        
    }
    
    
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    
    if(N76E003_write_aprom()==0)
    {
      //IAP 关闭FLASH操作时序
      ICPCLK_Transfer_close_flash();
      display_write_aprom_ok();
    }
    else
    {
      return 2;
    }
    
    
  }
  
  
  
  //校验APROM
  if((u8)cJSON_GetObjectItem(root, "verify_aprom")->valueint==1)
  {
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    
    
    
    ret=N76E003_verify_aprom(0);
    
    
    //IAP 关闭FLASH操作时序
    ICPCLK_Transfer_close_flash();
    if(ret==0)
      display_verify_aprom_ok();
    else
      return ret;
  }
  
  
  
  
  //写滚码
  if((u8)cJSON_GetObjectItem(root, "write_rolling_code")->valueint==1)
  {
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    
    if(N76E003_write_rollingcode()==0)
    {
      //IAP 关闭FLASH操作时序
      ICPCLK_Transfer_close_flash();
      
      display_WriteRollingCode_ok();
    }
    else
    {
      return 2;
    }
  }
  
  
  
  //  //校验滚码
  //  if((u8)cJSON_GetObjectItem(root, "verify_aprom")->valueint==1)
  //  {
  //    icp_io_config();
  //    ICPCLK_Transfer();
  //    cid=N76E003_IAP_CMD_READ_CID();
  //    did=N76E003_IAP_CMD_READ_DID_H_L();
  //    
  //    
  //    
  //    ret=N76E003_verify_aprom(0);
  //    
  //    
  //    //IAP 关闭FLASH操作时序
  //    ICPCLK_Transfer_close_flash();
  //    if(ret==0)
  //      display_verify_aprom_ok();
  //    else
  //      return ret;
  //  }
  
  
  //写配置字
  if((u8)cJSON_GetObjectItem(root, "write_config")->valueint==1)
  {
    if((u8)cJSON_GetObjectItem(root, "mass_erase")->valueint==0)
    {
      icp_io_config();
      ICPCLK_Transfer();
      cid=N76E003_IAP_CMD_READ_CID();
      did=N76E003_IAP_CMD_READ_DID_H_L();
      
      if(N76E003_config_all_erase()==0)
      {
        //IAP 关闭FLASH操作时序
        ICPCLK_Transfer_close_flash();
        // display_write_aprom_ok();
      }
      else
      {
        return 2;
      }        
    }
    
    
    
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    
    for(u8 i=0;i<(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),0)->valueint;i++)
    {
      conf_w_0[i]=(u8)cJSON_GetArrayItem(cJSON_GetObjectItem(root, "config_byte"),i+1)->valueint;
    }
    
    N76E003_config_write(conf_w_0);
    
    ICPCLK_Transfer_close_flash();
    
    
    
    ret=N76E003_verify_config();
    
    if(ret==0)
    {
      display_write_config_byte_ok();
    }
    else
    {
      return 2;
    }
    
    
  }
  
  
  
  switch(cJSON_GetObjectItem(root, "reset_type")->valueint==1)
  {
  case 0:
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    N76E003_config_read(conf);
    N76E003_aprom_write_byte_32_start_continue_end();//这一句执行复位运行程序操作
    ICPCLK_Transfer_close_flash();
    break;
    
  case 1:
    icp_io_config();
    ICPCLK_Transfer();
    cid=N76E003_IAP_CMD_READ_CID();
    did=N76E003_IAP_CMD_READ_DID_H_L();
    N76E003_config_read(conf);
    N76E003_aprom_write_byte_32_start_continue_end();//这一句执行复位运行程序操作
    ICPCLK_Transfer_close_flash();
    break;
    
  case 2:
    
    break;
    
  default:
    
    break;
  }
  
  
  
  return 0;
}




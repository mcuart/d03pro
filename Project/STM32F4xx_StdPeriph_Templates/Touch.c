#include "touch.h"
//uint8_t XPT2046_SendByte(uint8_t byte);
//uint8_t XPT2046_ReadByte(void);
uint16_t ADS_Read_AD(uint8_t CMD);
uint8_t Read_ADS(uint16_t *x,uint16_t *y);
static void delay_us(u32 t);
void ADS_Write_Byte(uint8_t num);  

//uint8_t XPT2046_SendByte(uint8_t byte)
//{
////  /* Loop while DR register in not emplty */
////  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
////  
////  /* Send Half Word through the SPI3 peripheral */
////  SPI_I2S_SendData(SPI1, byte);
////  
////  /* Wait to receive a Half Word */
////  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
////  
////  /* Menu the Half Word read from the SPI bus */
////  return SPI_I2S_ReceiveData(SPI1);
//  uint8_t RxData=0;
//  
//  HAL_SPI_TransmitReceive(&SpiHandle,&byte, &RxData, 1,100);
//  
//  return RxData;
//  
//}
//
//
//uint8_t XPT2046_ReadByte(void)
//{
//  return (XPT2046_SendByte(0x00));
//}



//uint16_t ADS_Read_AD(uint8_t CMD)
//{
//  uint16_t temp,Addata;
//  
//  Reset_TOUCH_TCS;//使能
//  XPT2046_SendByte(CMD) ;
//  temp=0;//相当于简短的延时啦
//  temp= XPT2046_ReadByte() ;
//  Addata =temp<<8;
//  temp= XPT2046_ReadByte() ;
//  Addata |=temp;
//  Addata >>=3;//SPI读数由于第8位为0
//  Addata &=0XFFF;
//  Set_TOUCH_TCS;
//  
//  return Addata;  
//}


static void delay_us(u32 t)
{
  for(u32 i=0;i<t;i++);
}

void ADS_Write_Byte(uint8_t num)    
{  
  uint8_t count=0;   
  for(count=0;count<8;count++)  
  { 	  
    if(num&0x80)Set_TOUCH_TDIN; 
    else Reset_TOUCH_TDIN;   
    num<<=1;    
    Reset_TOUCH_TCLK;//上升沿有效
    delay_us(1);
    Set_TOUCH_TCLK;      
  } 			    
} 		 






//SPI读数据 
//从7846/7843/XPT2046/UH7843/UH7846读取adc值	   
uint16_t ADS_Read_AD(uint8_t CMD)	  
{ 	 
  uint8_t count=0; 	  
  uint16_t Num=0; 
  Reset_TOUCH_TCLK;//先拉低时钟 
  Reset_TOUCH_TDIN;//拉低数据线
  Reset_TOUCH_TCS; //选中ADS7843	 
  ADS_Write_Byte(CMD);//发送命令字
  delay_us(6);//ADS7846的转换时间最长为6us
  Reset_TOUCH_TCLK; 
  delay_us(1);
  Set_TOUCH_TCLK;//给1个时钟，清除BUSY   
  delay_us(1);
  Reset_TOUCH_TCLK; 	 
  for(count=0;count<16;count++)  
  { 				  
    Num<<=1; 	 
    Reset_TOUCH_TCLK;//下降沿有效  	
    delay_us(1);
    Set_TOUCH_TCLK;
    if(TOUCH_DOUT)Num++; 		 
  }  	
  Num>>=4;   //只有高12位有效.
  Set_TOUCH_TCS;//释放ADS7843	 
  return(Num);   
}



//读取一个坐标值
//连续读取READ_TIMES次数据,对这些数据升序排列,
//然后去掉最低和最高LOST_VAL个数,取平均值 
#define READ_TIMES 5 //读取次数
#define LOST_VAL 1	  //丢弃值
uint16_t ADS_Read_XY(uint8_t xy)
{
  uint16_t i, j;
  uint16_t buf[READ_TIMES];
  uint16_t sum=0;
  uint16_t temp;
  for(i=0;i<READ_TIMES;i++)
  {				 
    buf[i]=ADS_Read_AD(xy);	    
  }				    
  for(i=0;i<READ_TIMES-1; i++)//排序
  {
    for(j=i+1;j<READ_TIMES;j++)
    {
      if(buf[i]>buf[j])//升序排列
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }	  
  sum=0;
  for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
  temp=sum/(READ_TIMES-2*LOST_VAL);
  return temp;   
} 




void Touch_Init(void)
{	
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE , ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  
}


//带滤波的坐标读取
//最小值不能少于100.
uint8_t Read_ADS(uint16_t *x,uint16_t *y)
{
  uint16_t xtemp,ytemp;			 	 		  
  xtemp=ADS_Read_XY(CMD_RDX);
  ytemp=ADS_Read_XY(CMD_RDY);	  												   
  if(xtemp<100||ytemp<100)
    return 0;//读数失败
  *x=xtemp;
  *y=ytemp;
  return 1;//读数成功
}
//2次读取ADS7846,连续读取2次有效的AD值,且这两次的偏差不能超过
//50,满足条件,则认为读数正确,否则读数错误.	   
//该函数能大大提高准确度

//uint8_t Read_ADS2(uint16_t *x,uint16_t *y) 
//{
//    uint16_t x1,y1;
//    uint16_t x2,y2;
//    uint8_t flag;    
//    flag=Read_ADS(&x1,&y1);   
//    if(flag==0)return(0);
//    flag=Read_ADS(&x2,&y2);	   
//    if(flag==0)return(0);   
//    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
//    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
//    {
//        *x=(x1+x2)/2;
//        *y=(y1+y2)/2;
//        return 1;
//    }else return 0;	  
//} 


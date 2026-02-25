/**
 ****************************************************************************************************
 * @file        delay.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-14
 * @brief       使用SysTick的普通计数模式对延迟进行管理(支持ucosii)
 *              提供delay_init初始化函数， delay_us和delay_ms等延时函数
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 F407电机开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211014
 * 第一次发布
 *
 ****************************************************************************************************
 */


#include "delay.h"

#define SYS_SUPPORT_OS		1		//?¨ò??μí3???t?Dê?·??§3?UCOS
static uint32_t g_fac_us = 0;       /* us延时倍乘数 */

/* 如果SYS_SUPPORT_OS定义了,说明要支持OS了(不限于UCOS) */
#if SYS_SUPPORT_OS

#include "os.h"

/**
 * @brief     systick中断服务函数,使用OS时用到
 * @param     ticks : 延时的节拍数  
 * @retval    无
 */  
void SysTick_Handler(void)
{
    if (OSRunning == OS_STATE_OS_RUNNING)   /* OS开始跑了,才执行正常的调度处理 */
    {
        OS_CPU_SysTickHandler();            /* 调用uC/OS-III的SysTick中断服务程序 */
    }
}
#endif

/**
 * @brief     初始化延迟函数
 * @param     sysclk: 系统时钟频率, 即CPU频率(rcc_c_ck), 168MHz
 * @retval    无
 */  
void delay_init(void)
{
#if SYS_SUPPORT_OS                                          /* 如果需要支持OS. */
    uint32_t reload;
#endif
    
    SysTick->CTRL = 0;                                      /* 清Systick状态，以便下一步重设，如果这里开了中断会关闭其中断 */
    //HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);    /* SYSTICK使用外部时钟源，频率为HCLK */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    g_fac_us = SystemCoreClock/1000000;                                      /* 不论是否使用OS,g_fac_us都需要使用,作为1us的基础时基 */
    
#if SYS_SUPPORT_OS                                          /* 如果需要支持OS. */
    reload = SystemCoreClock/1000000;                                         /* 每秒钟的计数次数 单位为M */
    reload *= 1000000 / OSCfg_TickRate_Hz;                  /* 根据OSCfg_TickRate_Hz设定溢出时间
                                                             * reload为24位寄存器,最大值:16777216,在168M下,约合0.7989s左右
                                                             */
    SysTick->CTRL |= 1 << 1;                                /* 开启SYSTICK中断 */
    SysTick->LOAD = reload;                                 /* 每1/delay_ostickspersec秒中断一次 */
    SysTick->CTRL |= 1 << 0;                                /* 开启SYSTICK */
#endif
}

#if SYS_SUPPORT_OS                                      /* 如果需要支持OS, 用以下代码 */

/**
 * @brief     延时nus
 * @param     nus: 要延时的us数
 * @note      nus取值范围 : 0 ~ 190887435us(最大值即 2^32 / fac_us @fac_us = 21)
 * @retval    无
 */ 
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload;
    OS_ERR err;
    
    reload = SysTick->LOAD;     /* LOAD的值 */
    ticks = nus * g_fac_us;     /* 需要的节拍数 */
  //  OSSchedLock(&err);          /* 阻止OS调度，防止打断us延时 */
    OSIntEnter(); 
    told = SysTick->VAL;        /* 刚进入时的计数器值 */

    while (1)
    {
        tnow = SysTick->VAL;

        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;    /* 这里注意一下SYSTICK是一个递减的计数器就可以了. */
            }
            else
            {
                tcnt += reload - tnow + told;
            }

            told = tnow;

            if (tcnt >= ticks) break;   /* 时间超过/等于要延迟的时间,则退出. */
        }
    }
OSIntExit(); 
 //   OSSchedUnlock(&err);                /* 恢复OS调度 */
} 

/**
 * @brief     延时nms
 * @param     nms: 要延时的ms数 (0< nms <= 65535) 
 * @retval    无
 */
void delay_ms(uint16_t nms)
{
  uint32_t i;
  u32 nus=0;
  uint32_t ticks;
  uint32_t told, tnow, tcnt = 0;
  uint32_t reload;
  OS_ERR err;
  
  
  OSIntEnter(); 
  // OSSchedLock(&err);          /* 阻止OS调度，防止打断us延时 */
  for (i=0; i<nms; i++)
  {
    
    nus=1000;
    
    
    tcnt = 0;
    reload = SysTick->LOAD;     /* LOAD的值 */
    ticks = nus * g_fac_us;     /* 需要的节拍数 */
    told = SysTick->VAL;        /* 刚进入时的计数器值 */
    
    while (1)
    {
      tnow = SysTick->VAL;
      
      if (tnow != told)
      {
        if (tnow < told)
        {
          tcnt += told - tnow;    /* 这里注意一下SYSTICK是一个递减的计数器就可以了. */
        }
        else
        {
          tcnt += reload - tnow + told;
        }
        
        told = tnow;
        
        if (tcnt >= ticks) break;   /* 时间超过/等于要延迟的时间,则退出. */
      }
    }
  }       
 OSIntExit(); 
   //  OSSchedUnlock(&err);                /* 恢复OS调度 */
}

#else  /* 不使用OS时, 用以下代码 */

/**
 * @brief       延时nus
 * @param       nus: 要延时的us数.
 * @note        nus取值范围 : 0~190887435(最大值即 2^32 / fac_us @fac_us = 21)
 * @retval      无
 */
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*g_fac_us; 					//时间加载	  		 
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;      					 //清空计数器	 
}

/**
 * @brief       延时nms
 * @param       nms: 要延时的ms数 (0< nms <= 65535)
 * @retval      无
 */
void delay_ms(uint16_t nms)
{
    uint32_t repeat = nms / 30;     /*  这里用30,是考虑到可能有超频应用 */
    uint32_t remain = nms % 30;

    while (repeat)
    {
        delay_us(30 * 1000);        /* 利用delay_us 实现 1000ms 延时 */
        repeat--;
    }

    if (remain)
    {
        delay_us(remain * 1000);    /* 利用delay_us, 把尾数延时(remain ms)给做了 */
    }
}

/**
 * @brief       HAL库内部函数用到的延时
 * @note        HAL库的延时默认用Systick，如果我们没有开Systick的中断会导致调用这个延时后无法退出
 * @param       Delay : 要延时的毫秒数
 * @retval      None
 */
void HAL_Delay(uint32_t Delay)
{
     delay_ms(Delay);
}
#endif










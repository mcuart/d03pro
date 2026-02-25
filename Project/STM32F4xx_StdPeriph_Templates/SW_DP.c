#include "DAP_config.h"
#include "DAP.h"



GPIO_InitTypeDef INIT_SWD_PINS = {
  PIN_SWCLK_TCK | PIN_SWDIO_TMS,
  GPIO_Mode_OUT,
  GPIO_Speed_100MHz,
  GPIO_OType_PP,
  GPIO_PuPd_NOPULL
};

/** Setup SWD I/O pins: SWCLK, SWDIO, and nRESET.
Configures the DAP Hardware I/O pins for Serial Wire Debug (SWD) mode:
- SWCLK, SWDIO, nRESET to output mode and set to default high level.
- TDI, TDO, nTRST to HighZ mode (pins are unused in SWD mode).
*/ 
void PORT_SWD_SETUP(SWD_HANDLE *pswd)
{
//  PIN_SWCLK_TCK_PORT->BSRR = (PIN_SWCLK_TCK | PIN_SWDIO_TMS);
// // PIN_nRESET_PORT->BRR = PIN_nRESET;
//  
//  
//  GPIO_INIT(PIN_SWCLK_TCK_PORT, INIT_SWD_PINS);
//  //PIN_nRESET_HIGH();
  
  pswd->swclk_port->BSRR = PIN_MASK(pswd->swclk_pin)|PIN_MASK(pswd->swdio_pin);
  INIT_SWD_PINS.GPIO_Pin=PIN_MASK(pswd->swclk_pin)|PIN_MASK(pswd->swdio_pin);
  GPIO_INIT(pswd->swclk_port, INIT_SWD_PINS);
  
}


//const GPIO_InitTypeDef INIT_OFF_1 = {
//  (PIN_SWCLK_TCK | PIN_SWDIO_TMS),
//  GPIO_Mode_IN,
//  GPIO_Speed_100MHz,
//  GPIO_OType_PP,
//  GPIO_PuPd_UP
//};
//const GPIO_InitTypeDef INIT_OFF_2 = {
//  (PIN_nRESET),
//  GPIO_Mode_IN,
//  GPIO_Speed_100MHz,
//  GPIO_OType_PP,
//  GPIO_PuPd_UP
//};
GPIO_InitTypeDef INIT_OFF_1 = {
  (PIN_SWCLK_TCK | PIN_SWDIO_TMS),
  GPIO_Mode_IN,
  GPIO_Speed_100MHz,
  GPIO_OType_PP,
  GPIO_PuPd_UP
};
GPIO_InitTypeDef INIT_OFF_2 = {
  (PIN_nRESET),
  GPIO_Mode_IN,
  GPIO_Speed_100MHz,
  GPIO_OType_PP,
  GPIO_PuPd_UP
};




//void PIN_nRESET_OUT(SWD_HANDLE * pswd,uint8_t bit)
//{
//  if (bit & 1)
//  {
//    PIN_nRESET_HIGH();
//  }
//  else
//  {
//    PIN_nRESET_LOW();
//  }
//}


void PORT_OFF(SWD_HANDLE *pswd)
{
//  GPIO_INIT(PIN_SWCLK_TCK_PORT, INIT_OFF_1);
//  GPIO_INIT(PIN_nRESET_PORT,    INIT_OFF_2);
  INIT_OFF_1.GPIO_Pin=PIN_MASK(pswd->swclk_pin)|PIN_MASK(pswd->swdio_pin);
  GPIO_INIT(pswd->swdio_port, INIT_OFF_1);
  
  INIT_OFF_2.GPIO_Pin=PIN_MASK(pswd->sw_nreset_pin);
  GPIO_INIT(pswd->sw_nreset_port, INIT_OFF_2);
}





// SW Macros
#define PIN_SWCLK_SET(pswd)		PIN_SWCLK_TCK_SET(pswd)
#define PIN_SWCLK_CLR(pswd)		PIN_SWCLK_TCK_CLR(pswd)

////STM32F407方案必须加延时，STM32F103由于主频低，这里可以为空，即不延时
//#define PIN_DELAY() __NOP()
//
//#pragma inline = forced
//void SW_CLOCK_CYCLE(void)
//{
//  PIN_SWCLK_CLR();		
//  PIN_DELAY();			
//  PIN_SWCLK_SET();		
//  PIN_DELAY();
//}


#define SW_CLOCK_CYCLE(pswd)		\
PIN_SWCLK_CLR(pswd);		\
  PIN_DELAY();			\
    PIN_SWCLK_SET(pswd);		\
      PIN_DELAY()
        
#define SW_WRITE_BIT(pswd,bit)		\
        PIN_SWDIO_OUT(pswd,bit);		\
          PIN_SWCLK_CLR(pswd);		\
            PIN_DELAY();			\
              PIN_SWCLK_SET(pswd);		\
		PIN_DELAY()
    

//#define SW_CLOCK_CYCLE()		\
//PIN_SWCLK_CLR();		\
//    PIN_SWCLK_SET();		\
//        
//#define SW_WRITE_BIT(bit)		\
//        PIN_SWDIO_OUT(bit);		\
//          PIN_SWCLK_CLR();		\
//              PIN_SWCLK_SET();		\

#define SW_READ_BIT(pswd,bit)		\
                  PIN_SWCLK_CLR(pswd);		\
                    PIN_DELAY();			\
                      bit = PIN_SWDIO_IN(pswd);	\
                        PIN_SWCLK_SET(pswd);		\
                          PIN_DELAY()
                            
//#define PIN_DELAY()		PIN_DELAY_SLOW(DAP_Data.clock_delay)
//#define PIN_DELAY() PIN_DELAY_FAST()
#define PIN_DELAY()  
//Generate SWJ Sequence
//count:  sequence bit count
//data:	pointer to sequence bit data
//return: none
void SWJ_Sequence (SWD_HANDLE *pswd, uint32_t count, uint8_t *data)
{
    uint8_t val;
    uint8_t n = 0;

    DEBUG("函数名字SWJ_Sequence DATA:");
    while (count != 0)
    {
        count--;
        if (n == 0)
        {
            val = *data++;
            DEBUG(" %02X", val);
            n = 8;
        }
        if (val & 1)
        {
            PIN_SWDIO_TMS_SET(pswd);
        }
        else
        {
            PIN_SWDIO_TMS_CLR(pswd);
        }
        SW_CLOCK_CYCLE(pswd);
        val >>= 1;
        n--;
    }
    DEBUG("\n");
}
 
                                  
//SWD Transfer I/O
//request: A[3:2] RnW APnDP
//data:	DATA[31:0]
//return:  ACK[2:0]
#define SWD_TransferFunction(speed)	/**/						\
__ramfunc uint8_t SWD_Transfer##speed (SWD_HANDLE *pswd,uint8_t request, uint32_t *data)	\
{																\
    uint8_t ack;												\
    uint8_t bit;												\
    uint32_t val;												\
    uint8_t parity;												\
    uint8_t n;													\
    /* Packet Request */										\
    parity = 0;													\
    SW_WRITE_BIT(pswd, 1);		/* Start Bit */						\
    \
    bit = request >> 0;											\
    SW_WRITE_BIT(pswd, bit);		/* APnDP Bit */						\
    parity += bit;												\
    \
    bit = request >> 1;											\
    SW_WRITE_BIT(pswd, bit);		/* RnW Bit */						\
    parity += bit;												\
    \
    bit = request >> 2;											\
    SW_WRITE_BIT(pswd, bit);		/* A2 Bit */						\
    parity += bit;												\
    \
    bit = request >> 3;											\
    SW_WRITE_BIT(pswd, bit);		/* A3 Bit */						\
    parity += bit;												\
    \
    SW_WRITE_BIT(pswd, parity);	/* Parity Bit */					\
    SW_WRITE_BIT(pswd, 0);		/* Stop Bit */						\
    SW_WRITE_BIT(pswd, 1);		/* Park Bit */						\
    \
    /* Turnaround */											\
    PIN_SWDIO_OUT_DISABLE(pswd);									\
    for (n = DAP_Data.swd_conf.turnaround; n != 0; n--)			\
    {															\
        SW_CLOCK_CYCLE(pswd);										\
    }															\
    \
    /* Acknowledge response */									\
    SW_READ_BIT(pswd, bit);											\
    ack  = bit << 0;											\
    \
    SW_READ_BIT(pswd, bit);											\
    ack |= bit << 1;											\
    \
    SW_READ_BIT(pswd, bit);											\
    ack |= bit << 2;											\
    \
    if (ack == DAP_TRANSFER_OK)									\
    {	/* OK response */										\
        /* Data transfer */										\
        if (request & DAP_TRANSFER_RnW)							\
        {	/* Read data */										\
            val = 0;											\
            parity = 0;											\
            for (n = 32; n; n--)								\
            {													\
                SW_READ_BIT(pswd, bit);	/* Read RDATA[0:31] */		\
                parity += bit;									\
                val >>= 1;										\
                val  |= bit << 31;								\
            }													\
            SW_READ_BIT(pswd, bit);		/* Read Parity */			\
            if ((parity ^ bit) & 1)								\
            {													\
                ack = DAP_TRANSFER_ERROR;						\
            }													\
            if (data) *data = val;								\
            /* Turnaround */									\
            for (n = DAP_Data.swd_conf.turnaround; n != 0; n--)	\
            {													\
                SW_CLOCK_CYCLE(pswd);								\
            }													\
            \
            PIN_SWDIO_OUT_ENABLE(pswd);								\
        }														\
        else													\
        {														\
            /* Turnaround */									\
            for (n = DAP_Data.swd_conf.turnaround; n != 0; n--)	\
            {													\
                SW_CLOCK_CYCLE(pswd);								\
            }													\
            \
            PIN_SWDIO_OUT_ENABLE(pswd);								\
            /* Write data */									\
            val = *data;										\
            parity = 0;											\
            for (n = 32; n; n--) {								\
                SW_WRITE_BIT(pswd, val);	/* Write WDATA[0:31] */		\
                parity += val;									\
                val >>= 1;										\
            }													\
            SW_WRITE_BIT(pswd, parity);	/* Write Parity Bit */		\
        }														\
        /* Idle cycles */										\
        n = DAP_Data.transfer.idle_cycles;						\
        if (n != 0)												\
        {														\
            PIN_SWDIO_OUT(pswd, 0);									\
            for (; n != 0; n--)									\
            {													\
                SW_CLOCK_CYCLE(pswd);								\
            }													\
        }														\
        PIN_SWDIO_OUT(pswd, 1);										\
        return (ack);											\
    }															\
    \
    if (ack == DAP_TRANSFER_WAIT || ack == DAP_TRANSFER_FAULT)	\
    {																			\
        /* WAIT or FAULT response */											\
        if (DAP_Data.swd_conf.data_phase && (request & DAP_TRANSFER_RnW) != 0)	\
        {																		\
            for (n = 32+1; n; n--)												\
            {																	\
                SW_CLOCK_CYCLE(pswd);	/* Dummy Read RDATA[0:31] + Parity */		\
            }																	\
        }																		\
        /* Turnaround */														\
        for (n = DAP_Data.swd_conf.turnaround; n != 0; n--)						\
        {																		\
            SW_CLOCK_CYCLE(pswd);													\
        }																		\
        \
        PIN_SWDIO_OUT_ENABLE(pswd);													\
        if (DAP_Data.swd_conf.data_phase && (request & DAP_TRANSFER_RnW) == 0)	\
        {																		\
            PIN_SWDIO_OUT(pswd, 0);													\
            for (n = 32 + 1; n != 0; n--)										\
            {																	\
                SW_CLOCK_CYCLE(pswd);	/* Dummy Write WDATA[0:31] + Parity */		\
            }																	\
        }																		\
        PIN_SWDIO_OUT(pswd, 1);														\
        return (ack);															\
    }																			\
    \
    /* Protocol error */														\
    for (n = DAP_Data.swd_conf.turnaround + 32 + 1; n != 0; n--)				\
    {																			\
        SW_CLOCK_CYCLE(pswd);	/* Back off data phase */							\
    }																			\
    \
    PIN_SWDIO_OUT(pswd, 1);															\
    return (ack);																\
}
                              


//uint8_t SWD_Transfer_fast1(uint32_t *data)	\
//{																\
//    uint8_t ack;												\
//    uint8_t bit;												\
//    uint32_t val;												\
//    uint8_t parity;												\
//    uint8_t n;													\
//    /* Packet Request */										\
//    SW_WRITE_BIT(1);		/* Start Bit */	
//    SW_WRITE_BIT(1);		/* APnDP Bit */				\
//    SW_WRITE_BIT(0);		/* RnW Bit */
//    SW_WRITE_BIT(1);		/* A2 Bit */
//    SW_WRITE_BIT(1);		/* A3 Bit */
//    SW_WRITE_BIT(1);	        /* Parity Bit */
//    SW_WRITE_BIT(0);		/* Stop Bit */
//    SW_WRITE_BIT(1);		/* Park Bit */
//    /* Turnaround */											\
//    PIN_SWDIO_OUT_DISABLE();												\
//    SW_CLOCK_CYCLE();										\
//    /* Acknowledge response */									\
//    SW_READ_BIT(bit);											\
//    ack  = bit << 0;											\
//    SW_READ_BIT(bit);											\
//    ack |= bit << 1;											\
//    SW_READ_BIT(bit);											\
//    ack |= bit << 2;											\
//    if (ack == DAP_TRANSFER_OK)									\
//    {	/* OK response */												\
//        /* Turnaround */                                                              \
//        SW_CLOCK_CYCLE();								\
//        PIN_SWDIO_OUT_ENABLE();								\
//        /* Write data */									\
//        val = *data;										\
//        parity = 0;											\
//        for (n = 32; n; n--) {								\
//            SW_WRITE_BIT(val);	/* Write WDATA[0:31] */		\
//            parity += val;									\
//            val >>= 1;										\
//        }													\
//        SW_WRITE_BIT(parity);	/* Write Parity Bit */										\
//        PIN_SWDIO_OUT(1);										\
//        return (ack);											\
//    }															\
//    \
//    if (ack == DAP_TRANSFER_WAIT || ack == DAP_TRANSFER_FAULT)	\
//    {																			\
//        /* WAIT or FAULT response */																									\
//        /* Turnaround */																											\
//        SW_CLOCK_CYCLE();													\
//        PIN_SWDIO_OUT_ENABLE();													\
//																	\
//        PIN_SWDIO_OUT(0);													\
//        for (n = 32 + 1; n != 0; n--)										\
//        {																	\
//            SW_CLOCK_CYCLE();	/* Dummy Write WDATA[0:31] + Parity */		\
//        }																															\
//        PIN_SWDIO_OUT(1);														\
//        return (ack);															\
//    }																			\
//    \
//    /* Protocol error */																												\
//    SW_CLOCK_CYCLE();	/* Back off data phase */							\
//    PIN_SWDIO_OUT(1);															\
//    return (ack);																\
//}





//#undef  PIN_DELAY
//#define PIN_DELAY()		PIN_DELAY_FAST()
#undef  PIN_DELAY
//#define PIN_DELAY()
////STM32F407方案必须加延时，STM32F103由于主频低，这里可以为空，即不延时
#define PIN_DELAY() __NOP()

SWD_TransferFunction(Fast);
                              
//#undef  PIN_DELAY
//#define PIN_DELAY()		PIN_DELAY_SLOW(DAP_Data.clock_delay)
//                              SWD_TransferFunction(Slow);
                              
// SWD Transfer I/O
//request: A[3:2] RnW APnDP
//data:	DATA[31:0]
//return:  ACK[2:0]
uint8_t  SWD_Transfer(SWD_HANDLE *pswd, uint8_t request, uint32_t *data)
{
//    if (DAP_Data.fast_clock)
        return SWD_TransferFast(pswd, request, data);
//    else
//        return SWD_TransferSlow(request, data);
}

                              
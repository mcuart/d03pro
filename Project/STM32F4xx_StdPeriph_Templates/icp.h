#ifndef __ICP_H__
#define __ICP_H__


#define GPIO_INIT(port, data)	GPIO_Init(port, (GPIO_InitTypeDef *)&data)
#define PIN_MODE_MASK(pin)		(((uint32_t)0x03) << ((pin) << 1))
#define PIN_MODE(mode,pin)		(((uint32_t)mode) << ((pin) << 1))
#define PIN_MASK(pin)			(((uint16_t)0x01) << (pin))


#define PIN_OTYPE_MASK(pin)		(((uint32_t)0x01) << pin)
#define PIN_OTYPE(otype,pin)		(((uint32_t)otype) << pin)

#define PIN_OSPEED_MASK(pin)		(((uint32_t)0x03) << ((pin) << 1))
#define PIN_OSPEED(ospeed,pin)		(((uint32_t)ospeed) << ((pin) << 1))

#define PIN_PUPD_MASK(pin)		(((uint32_t)0x03) << ((pin) << 1))
#define PIN_PUPD(pupd,pin)		(((uint32_t)pupd) << ((pin) << 1))


// SWDIO/TMS Pin
#define PIN_ICP_DAT_PORT		GPIOB
#define PIN_ICP_DAT_PIN		0

// SWCLK/TCK Pin
#define PIN_ICP_CLK_PORT		GPIOB
#define PIN_ICP_CLK_PIN		1


// nRESET Pin
#define PIN_ICP_nRESET_PORT                 GPIOB
#define PIN_ICP_nRESET_PIN			2



//IAP 打开FLASH操作时序
void ICPCLK_Transfer(void);
//IAP 关闭FLASH操作 并退出ICP模式时序 
void ICPCLK_Transfer_close_flash(void);
u8 N76E003_IAP_CMD_READ_CID(void);
u8 N76E003_IAP_CMD_READ_DID_L(void);
u8 N76E003_IAP_CMD_READ_DID_H(void);
u16 N76E003_IAP_CMD_READ_DID_H_L(void);
u8 N76E003_flash_read_byte(u16 address);
u8 N76E003_flash_read_multi_byte(u8 * dst,u16 address_start,u32 length);
u8 N76E003_flash_write_byte_32(u32 address,u8 *src,u32 length);
u8 N76E003_flash_write_byte_32_start(u32 address,u8 *src);
u8 N76E003_flash_write_byte_32_start_continue(u32 address,u8 *src,u32 length);
u8 N76E003_flash_write_multi_byte(u8 * src,u32 address_start,u32 length);
u8 N76E003_flash_mass_erase(void);
u8 N76E003_config_read(u8 *dst);
u8 N76E003_config_write(u8 *src);
u8 N76E003_write_flash(void);
u8 N76E003_TEST(void);


#endif



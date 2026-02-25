#ifndef __CONFIG_H
#define __CONFIG_H

#include "stm32f4xx.h"

extern const u8 boot[];
extern u32 boot_loader_file_size;
#define USB_DP_DISABLE u_dp_dis(DISABLE)
#define USB_DP_ENABLE u_dp_dis(ENABLE)


#define USB_DP_DISABLE_MASS u_dp_dis_MASS(DISABLE)
#define USB_DP_ENABLE_MASS u_dp_dis_MASS(ENABLE)




void TIM1_Config(void);
void TIM6_Config(void);
void ADC_DMA_Config(void);
void NVIC_Configuration(void);
void u_dp_dis(FunctionalState IsUsbEnable);
void u_dp_dis_MASS(FunctionalState IsUsbEnable);
void TIM7_Config(void);
void USART3_Config(void);
void USART3_Config_DISABLE(void);
void TIM7_Config_DISABLE(void);
void System_Delay_ms(u16 nms);
void System_Delay_us(u32 nus);
#endif /* __CONFIG_H */
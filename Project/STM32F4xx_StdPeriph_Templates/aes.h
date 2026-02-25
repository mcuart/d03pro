#include "stm32f4xx.h"



void AES_Test_Decrypt(u8 *dat, u32 data_length, u32 push_id);
void AES_Test_Encrypt(u8 *dat, u32 data_length, u32 push_id);


void AES_Decrypt_Config_Init(u32 data_length, u32 push_id);
void AES_Decrypt_Calculate(u8 *dat);


void AES_Encrypt_Config_Init(u32 data_length, u32 push_id);
void AES_Encrypt_Calculate(u8 *dat);
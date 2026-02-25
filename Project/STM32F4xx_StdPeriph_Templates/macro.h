#ifndef MACRO_H_
#define MACRO_H_


#define SWIM_CSR					0x00007F80
#define DM_CSR2						0x00007F99

#define STM8S_PUKR   0
#define STM8S_DUKR   0
#define STM8S_CR2    0
#define STM8S_IAPSR 0


#define STM8S_IAPSR_DUL 0x08



//#define STM8L152C6T6

//#define STM8S003F3


//#ifdef STM8S003F3
//
//#define STM8_FLASH_PUKR		0x00005062  //STM8S003F3P6
//#define STM8_FLASH_CR2		0x0000505B  //STM8S003F3P6
//#define STM8_FLASH_IAPSR	0x0000505F  //STM8S003F3P6
#define STM8_FLASH_EOP		0x04        //STM8S003F3P6

//#define STM8_FLASH_DUKR		0x00005064  //STM8S003F3P6
//
//
//#endif



#ifdef STM8L152C6T6


 
#define STM8_FLASH_PUKR		0x00005052
#define STM8_FLASH_CR2		0x00005051
#define STM8_FLASH_IAPSR	0x00005054
#define STM8_FLASH_EOP		0x04

#define STM8_FLASH_DUKR		0x00005053   //STM8L152C6T6


#endif




#endif



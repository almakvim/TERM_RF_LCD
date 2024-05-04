
#ifndef __system_H
#define __system_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "main.h"

//#define OSC							((u32)14.7456e6)							//	Частота внешнего генератора(резонатора)
//#define HCLK						((u32)(OSC / 6 * 9))						//	Системная частота
//#define PCLK1						((u32)(HCLK / 2))							//	Частота 1-ой периферийной шины
//#define PCLK2						((u32)(HCLK / 2))							//	Частота 2-ой периферийной шины

#define SYSTEM_FOSC  8000000L      // 	(14.745600 )

#define SYSTEM_PLLM    9
#define SYSTEM_PLLS    1             

#define SYSTEM_APBD  2
#define SYSTEM_CCLK ((SYSTEM_FOSC/SYSTEM_PLLS)*SYSTEM_PLLM) 
#define SYSTEM_PCLK (SYSTEM_CCLK/SYSTEM_APBD)    				                                    // ??????? ????         (35.9424MHz)

// размещение в функции в RAM
#define __ramfunc  __attribute__ ((section ("ramfunc")))
//#define __ramfunc	__attribute__ ((section (".xram")))
#define ADC1_DR_Address    ((u32)0x4001244C)

// Префикс функций обработчиков прерываний
#define _interrupt
#define  interrupt
// Типы
#define Uint8  unsigned char
#define Uint16 unsigned short
#define Uint32 unsigned long
#define Uint64 unsigned long long


#define MSG_PCONTROL_RECV_1			1700			//	
#define MSG_PCONTROL_SEND_1			1701			//	

//#define AVER_NUM_32 32
//#define AVER_NUM_64 64
//#define AVER_NUM_256 256

/*!< STM32F10x Standard Peripheral Library old types (maintained for legacy purpose) */
typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  /*!< Read Only */
typedef const int16_t sc16;  /*!< Read Only */
typedef const int8_t sc8;   /*!< Read Only */


typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef union  
{
    uint8_t dt_8[2];
    uint16_t dt_16;
}
DATA_2;

typedef union  
{
    uint8_t dt_8[4];
    uint16_t dt_16[2];
    uint32_t dt_32;
}
DATA_4;

typedef union  
{
    uint8_t dt_8[8];
    uint16_t dt_16[4];
    uint32_t dt_32[2];
	uint64_t dt_64;
}
DATA_8;

typedef union  
{
    uint8_t dt_8[16];
    uint16_t dt_16[8];
    uint32_t dt_32[4];
	uint64_t dt_64[2];
}
DATA_16;

typedef union  
{
    uint8_t dt_8[24];
    uint16_t dt_16[12];
    uint32_t dt_32[6];
	uint64_t dt_64[3];
}
DATA_24;

typedef union  
{
    uint8_t dt_8[32];
    uint16_t dt_16[16];
    uint32_t dt_32[8];
	uint64_t dt_64[4];
}
DATA_32;

typedef union
{
    uint8_t dt8[64];
    uint16_t dt16[32];
    uint32_t dt32[16];
	uint64_t dt64[8];
}
DATA_64;

typedef union
{
    char  bt[4];
    float fl;
    uint32_t ui;
    int32_t i;
}
var_4;


// Переменные настроек 
enum SYSTEM_VAR
{
	SYSTEM_VAR_SERIAL,				// серийный номер
	SYSTEM_VAR_WORKTIME,			// время работы
	SYSTEM_VAR_RESET,				// перегрузка системы
	SYSTEM_VAR_SETUP_SAVE,			// соxранить настройки
	SYSTEM_VAR_NUM          		// количество переменных
};

// состояние системы
typedef enum
{
	SYSTEM_MODE_RUN,
	SYSTEM_MODE_RESET
}
SYSTEM_MODE;

// напряжение питания устройства
extern float SYSTEM_Voltage;

// режим работы системы
extern SYSTEM_MODE SYSTEM_Mode;

extern float FIRMWARE_Version;
extern Uint16 adc_calibl;
extern u32 ADC_ConvertedValueTab[32*1];
//extern u16 Aver_M1[AVER_NUM_64 + 3];

// Инициализация системы и всех периферийных устройств и модулей
void SYSTEM_Initialize(void);
void Sys_PerifInit(void);

// Инициализация Clock
void SYSTEM_ClockInitialize(void);

// Конфигурация клока перефирии
void SYSTEM_RCCConfig(void);

void Sys_GPIOInit(void);
void Sys_InterruptInit(void);

void DelayUS(u32 nCount);
void DelayMS(u32 nCount);

// перезапуск контроллера
__ramfunc void SYSTEM_Reset(void);

// Функция обработки режимов
void SYSTEM_Proc(void);

//----------------------------------------------------------------
uint8_t System_prop(uint8_t par, uint8_t * name, uint8_t * prop, uint8_t * nbyte);
void System_get(uint8_t par, void * value);
void System_set(uint8_t par, void * value);
//----------------------------------------------------------------

u16 cont_Aver_u16(u16* aver, u16 value, u16 N_aver);
u16 cont_Aver(u32* aver, u16 value, u16 N_aver);
float cont_Aver_fl(float* aver, float value, u16 N_aver);
void MemCopy( void * src, void * dst, uint32_t size );

uint16_t swapData_16( void *data);
uint32_t swapData_24( void *data);
uint32_t swapData_32( void *data);
void swapBytes_16_24_32( void *data, uint16_t size);

//extern Uint32 uLastControlTime;

#ifdef __cplusplus
}
#endif
#endif /*__ sys_H */

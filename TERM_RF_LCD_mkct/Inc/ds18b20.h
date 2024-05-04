
#ifndef DS18B20_H
#define	DS18B20_H

#include "stdint.h"
#include "main.h"

#ifdef	__cplusplus
extern "C" {
#endif

// определения
#define	TRISDCL	TRISBbits.TRISB13		// регистр управления порта ДАЛЛАСА
#define	DALLAS	LATBbits.LATB13			// шина связи с ДАЛЛАСОМ1
#define	COLDAT	4						// разрешенное количество датчиков в системе

#define _XTAL_FREQ 14745600 //!!! В этом месте необходимо указать с какой тактовой частотой работает конттроллер
    
// Переменные настроек
enum DS18B20_VAR
{
	DS18B20_NDATCH,		// Количество датчиков
	DS18B20_TEMPER1,    // Температура 1
	DS18B20_TEMPER2,    // Температура 2
	DS18B20_TEMPER3,    // Температура 2
	DS18B20_TEMPER4,    // Температура 2
	DS18B20_HUMDATA,    // Влажность (RH%)
	DS18B20_NTERM,		// Количество датчиков

	DS18B20_VAR_NUM    		// количество переменных
};


extern float TEMPDAT[COLDAT]; 	// массив температур датчиков;
extern uint8_t n_term;
extern uint8_t n_datch;
//--------------------------------------------------------------------------------------- 

void port_init(void);
float ds18b20_get_temp (uint8_t curr);		
uint16_t SeachROM (void);		
void ds18b20_proc (void);	

//----------------------------------------------------------------
uint8_t DS18B20_prop(uint8_t par, uint8_t * name, uint8_t * prop, uint8_t * nbyte);
void DS18B20_get(uint8_t par, void * value);
void DS18B20_set(uint8_t par, void * value);
//--------------------------------------------------------------------------------------- 

#ifdef	__cplusplus
}
#endif

#endif	/* SETUP_H */

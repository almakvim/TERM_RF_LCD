
#ifndef DS18B20_H
#define	DS18B20_H

#include "stdint.h"
#include "main.h"

#ifdef	__cplusplus
extern "C" {
#endif

// �����������
#define	TRISDCL	TRISBbits.TRISB13		// ������� ���������� ����� �������
#define	DALLAS	LATBbits.LATB13			// ���� ����� � ��������1
#define	COLDAT	4						// ����������� ���������� �������� � �������

#define _XTAL_FREQ 14745600 //!!! � ���� ����� ���������� ������� � ����� �������� �������� �������� �����������
    
// ���������� ��������
enum DS18B20_VAR
{
	DS18B20_NDATCH,		// ���������� ��������
	DS18B20_TEMPER1,    // ����������� 1
	DS18B20_TEMPER2,    // ����������� 2
	DS18B20_TEMPER3,    // ����������� 2
	DS18B20_TEMPER4,    // ����������� 2
	DS18B20_HUMDATA,    // ��������� (RH%)
	DS18B20_NTERM,		// ���������� ��������

	DS18B20_VAR_NUM    		// ���������� ����������
};


extern float TEMPDAT[COLDAT]; 	// ������ ���������� ��������;
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

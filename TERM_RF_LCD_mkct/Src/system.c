
#include "mk_conf_tree.h"
#include "setup.h"
#include "control.h"
#include "system.h"

float FIRMWARE_Version = 54.002;

// таймаут перегрузки устройства
#define SYSTEM_ResetTimeout 1000

// напряжение питания устройства
float SYSTEM_Voltage;
Uint16 adc_calibl;		

// режим работы системы
SYSTEM_MODE SYSTEM_Mode = SYSTEM_MODE_RUN;

Uint32 uLastControlTime = 0;

u32 ADC_ConvertedValueTab[32*1];
//u16 Aver_M1[AVER_NUM_64 + 3];

// перезапуск контроллера
//__ramfunc void SYSTEM_Reset(void)
//{
//	__disable_irq();
//	SCB->AIRCR  = (SCB_AIRCR_VECTKEY_Pos | (SCB->AIRCR & (0x700)) | (1<<SCB_AIRCR_SYSRESETREQ_Pos)); // Keep priority group unchanged 
//	__DSB();                                                                             // Ensure completion of memory access             
//	while(1);
//}

//============================================================================
// Функция обработки режимов
void SYSTEM_Proc(void)
{
	static Uint32 timeReset = 0;
	
	// перегрузка устройства
	if(SYSTEM_Mode == SYSTEM_MODE_RESET)
    {	
		if(timeReset == 0)
        {
			timeReset = HAL_GetTick();
			return;
		}
        if(timeReset+SYSTEM_ResetTimeout <= HAL_GetTick())
        {
            __NVIC_SystemReset();
        }
    }
}

//============================================================================
void DelayUS(u32 nCount)
{

	nCount *= 13;							//	HCLK = 66.36MHz
	for(; nCount!= 0; nCount--);

}

void DelayMS(u32 nCount)
{

	nCount *= 6172;						//	HCLK = 66.36MHz
		for(; nCount!= 0; nCount--);

}

//============================================================================
uint8_t System_prop( uint8_t par, uint8_t * name, uint8_t * prop, uint8_t * nbyte )
{
	char * str;
    uint8_t size = 0;
	if( name ) { switch( par )
        {
        case SYSTEM_VAR_RESET       :	str ="Перегрузка системы"; break;
        case SYSTEM_VAR_SERIAL    	:	str ="Серийный номер"; break;
        case SYSTEM_VAR_WORKTIME    :	str ="Время работы"; break;
        case SYSTEM_VAR_SETUP_SAVE  :	str ="Соxранить настройки"; break;
        default: return 0;
        }
        while( *str ) { *name++ = *str++; size++; } *name = 0; 
    }
	if( prop ) switch( par )
        {
        case SYSTEM_VAR_RESET       :	*prop = BOOL; break;
        case SYSTEM_VAR_SERIAL    	:	*prop = UINT; break;
        case SYSTEM_VAR_WORKTIME    :	*prop = TIME|RO; break;
        case SYSTEM_VAR_SETUP_SAVE  :	*prop = BOOL; break;
        default: return 0;
        }
	if( nbyte ) switch( par )
        {
        case SYSTEM_VAR_RESET       :	*nbyte = 1; break;
        case SYSTEM_VAR_SERIAL    	:	*nbyte = 2; break;
        case SYSTEM_VAR_WORKTIME    :	*nbyte = 4; break;
        case SYSTEM_VAR_SETUP_SAVE  :	*nbyte = 1; break;
        default: return 0;
        }
    return size;
}
//============================================================================
void System_get(uint8_t par, void * value)
{
    switch( par )
    {
        case SYSTEM_VAR_SERIAL      :	*(u16*)value = Setup.SerialId; break;
        case SYSTEM_VAR_WORKTIME    :	*(u32*)value = HAL_GetTick(); break;
        case SYSTEM_VAR_RESET       :	*(u8*)value = SYSTEM_Mode; break;
        case SYSTEM_VAR_SETUP_SAVE  :	*(u8*)value = 0; break;

        default:;
    }
}
//============================================================================
void System_set(uint8_t par, void * value)
{
    u8 i = *(u8*)value;
    switch( par )
    {
        case SYSTEM_VAR_SERIAL :
            Setup.SerialId = *(u16*)value;
            break;
        case SYSTEM_VAR_RESET : 
            if(i > 0){
                SYSTEM_Mode = SYSTEM_MODE_RESET; 
            }
            break;

        case SYSTEM_VAR_SETUP_SAVE :
            if( i > 0 ) SETUP_Save();
            break;

        default:;
    }
}
//============================================================================
u16 cont_Aver_u16(u16* aver, u16 value, u16 N_aver)
{
 	u16 ptr = (u16)(aver[N_aver]);
 	u32 sum = aver[N_aver+2]*0x10000+aver[N_aver+1];
	if(ptr >= N_aver) ptr = 0;
	sum -= aver[ptr];
	sum += value;
	aver[ptr] = value;
	aver[N_aver] = ++ptr;
	aver[N_aver+1] = sum;
	aver[N_aver+2] = sum>>16;
    return (sum/N_aver);
}
//============================================================
u16 cont_Aver(u32* aver, u16 value, u16 N_aver) 
{
 	u16 ptr = (u16)(aver[N_aver]);
 	u32 sum = aver[N_aver+1];
	if(ptr >= N_aver) ptr = 0;
	sum -= aver[ptr];
	sum += value;
	aver[ptr] = value;
	aver[N_aver] = ++ptr;
	aver[N_aver+1] = sum;
    return (sum/N_aver);
}
//============================================================
float cont_Aver_fl(float* aver, float value, u16 N_aver)
{
 	u16 ptr = (u16)(aver[N_aver]);
 	float sum = aver[N_aver+1];
	if(ptr >= N_aver) ptr = 0;
	sum -= aver[ptr];
	sum += value;
	aver[ptr] = value;
	aver[N_aver] = ++ptr;
	aver[N_aver+1] = sum;
    return (sum/N_aver);
}
//=========================================================================
void MemCopy( void * src, void * dst, uint32_t size )
{
	register uint8_t * s = (uint8_t *)src;
	register uint8_t * d = (uint8_t *)dst;

	while( size-- ) *d++ = *s++;
	return;
}

//===========================================================================
//uint16_t swapData_16( uint16_t data)
//{
//    DATA_MAIN_2 tmp1;
//    DATA_MAIN_2 tmp2;
//
//    tmp1.dt_16 = data;
//
//    tmp2.dt_8[0] = tmp1.dt_8[1];
//    tmp2.dt_8[1] = tmp1.dt_8[0];
//
//    return tmp2.dt_16;
//}
uint16_t swapData_16( void *data)
{
	uint8_t * dt = (uint8_t *)data;
    DATA_2 tmp1;
 
    tmp1.dt_8[1] = dt[0];
    tmp1.dt_8[0] = dt[1];

    return tmp1.dt_16;
}

//===========================================================================
uint32_t swapData_24( void *data)
{
	uint8_t * dt = (uint8_t *)data;
    DATA_4 tmp1;
 
    tmp1.dt_8[3] = 0;
    tmp1.dt_8[2] = dt[0];
    tmp1.dt_8[1] = dt[1];
    tmp1.dt_8[0] = dt[2];

    return tmp1.dt_32;
}

//===========================================================================
uint32_t swapData_32( void *data)
{
	uint8_t * dt = (uint8_t *)data;
    DATA_4 tmp1;
 
    tmp1.dt_8[3] = dt[0];
    tmp1.dt_8[2] = dt[1];
    tmp1.dt_8[1] = dt[2];
    tmp1.dt_8[0] = dt[3];

    return tmp1.dt_32;
}

//===========================================================================
void swapBytes_16_24_32( void *data, uint16_t size)   // size = 2,3,4
{
	uint8_t * dt = (uint8_t *)data;
    DATA_4 tmp1;
    
	switch( size )
	{
        case 2: 
                tmp1.dt_8[1] = dt[0];
                tmp1.dt_8[0] = dt[1];
                break;
        case 3: 
                tmp1.dt_8[2] = dt[0];
                tmp1.dt_8[1] = dt[1];
                tmp1.dt_8[0] = dt[2];
                break;
        case 4: 
                tmp1.dt_8[3] = dt[0];
                tmp1.dt_8[2] = dt[1];
                tmp1.dt_8[1] = dt[2];
                tmp1.dt_8[0] = dt[3];
                break;
    }
    MemCopy(&tmp1.dt_8[0], dt, size);
}

//============================================================


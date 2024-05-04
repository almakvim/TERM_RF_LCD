
#include "string.h"
#include "mk_conf_tree.h"
#include "setup.h"
#include "system.h"
#include "USART.h"
#include "control.h"

VAR_PAR dev_var;
u32 mTimeout_cntrl = 0;
u32 mTimeout_shift = 0;

//============================================================================
uint8_t Control_prop( uint8_t par, uint8_t * name, uint8_t * prop, uint8_t * nbyte )
{
	char * str;
    uint8_t size = 0;
	if( name ) { switch( par )
        {
		case CONTROL_STAT       :	str ="Status"; break;
		case CONTROL_HUMD       :	str ="Влажность SNTC3 (%)"; break;
		case CONTROL_TEMP       :	str ="Температура SNTC3 (C)"; break;
        default: return 0;
        }
        while( *str ) { *name++ = *str++; size++; } *name = 0; 
    }
	if( prop ) switch( par )
        {
		case CONTROL_HUMD       :	
		case CONTROL_TEMP       :	*prop = UINT|RO; break;
		case CONTROL_STAT       :	*prop = HEX|RO; break;
        default: return 0;
        }
	if( nbyte ) switch( par )
        {
        case CONTROL_HUMD:     
        case CONTROL_TEMP:      
        case CONTROL_STAT:	    *nbyte = 2; break;
        default: return 0;
        }
    return size;
}
//============================================================================
void Control_get(uint8_t par, void * value)
{
    switch( par )
    {
        case CONTROL_HUMD     	:	*(u16*)value = dev_var.shtc3_hum; break;
        case CONTROL_TEMP     	:	*(u16*)value = dev_var.shtc3_temp; break;
        case CONTROL_STAT     	:	*(u16*)value = dev_var.state; break;
        default:;
    }
}
//============================================================================
void Control_set(uint8_t par, void * value)
{
    switch( par )
    {
        case CONTROL_HUMD:	    break;
        case CONTROL_TEMP:	    break;
        case CONTROL_STAT:	    break;
        default:;
    }
}
//============================================================================
uint8_t Params_prop( uint8_t par, uint8_t * name, uint8_t * prop, uint8_t * nbyte )
{
	char * str;
    uint8_t size = 0;
	if( name ) {switch( par )
        {
	case PARAM_HUMDET   :	str ="Наличие датчика Влажности"; break;
	case PARAM_HUMPLA   :	str ="Позиция датчика Влажности"; break;
        default: return 0;
        }
        while( *str ) { *name++ = *str++; size++; } *name = 0; 
    }
	if( prop ) switch( par )
        {
    case PARAM_HUMDET     :	*prop = BOOL; break;
    case PARAM_HUMPLA     :	*prop = UINT; break;
        default: return 0;
        }
	if( nbyte ) switch( par )
        {
        case PARAM_HUMDET   :   *nbyte = 1; break;
        case PARAM_HUMPLA  :	*nbyte = 1; break;
        default: return 0;
        }
    return size;
}
//============================================================================
void Params_get(uint8_t par, void * value)
{
    switch( par )
    {
		case PARAM_HUMDET     	:	*(u8*)value = Setup.humd; break;
		case PARAM_HUMPLA     	:	*(u8*)value = Setup.hum_poz; break;

        default:;
    }
}
//============================================================================
void Params_set(uint8_t par, void * value)
{
    u8 i = *(u8*)value;
    switch( par )
    {
		case PARAM_HUMDET	  :	Setup.humd = i; break;
		case PARAM_HUMPLA	  :	if(i<5)Setup.hum_poz = i; break;
        default:;
    }
}
//=========================================================================
// Функция обработки
void CONTROL_Proc(void)
{
	static Uint32 timeout_type = 0;
//	static Uint16 pulse = 0;

	if(dev_var.start == 0)
	{
        dev_var.start = 1;
	}

	if(timeout_type+1000 <= HAL_GetTick())
	{
        timeout_type = HAL_GetTick();
        dev_var.uart1_en = 0;
	}
}
//=========================================================================


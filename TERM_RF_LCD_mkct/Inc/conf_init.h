
#ifndef __conf_init_H
#define __conf_init_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"
#include "control.h"
#include "ds18b20.h"						

typedef uint8_t (*func_prop)(uint8_t par, uint8_t * name, uint8_t * prop, uint8_t * nbyte);
typedef void (*func_cmd)(uint8_t par, void * value);

enum UNIT_NUM   // Parent enum
{
	MAIN_UNIT,			
	SYSTEM_UNIT,				
	CONTROL_UNIT,			
	DS18B20_UNIT,			
	PARAM_UNIT,				
	N_UNIT,     // Number of units (nodes)				
};

#pragma pack (1)
//-------------------------------------------------------
const char *namesUnit[] =	
{
	"DS18B20 + SHTC3",      // name_unit
	"System",               // name_unit
	"Control",              // name_unit
	"Temper + RH",          // name_unit
	"Parameters",           // name_unit
};
//--------------- Variant 1 ------------------------------
const u8 unitProp[][2] =	// Node tree structure 
{
    {MAIN_UNIT, 0},                 //  {parent, num_params },
    {MAIN_UNIT, SYSTEM_VAR_NUM},    //  {parent, num_params },
    {MAIN_UNIT, CONTROL_VAR_NUM},   //  {parent, num_params },
    {MAIN_UNIT, DS18B20_VAR_NUM},   //  {parent, num_params },
    {MAIN_UNIT, PARAM_VAR_NUM},     //  {parent, num_params },
};
// Tree structure:
// DS18B20 + SHTC3-|
//                 |-System
//                 |-Control
//                 |-Temper + RH
//                 |-Parameters

//--------------- Variant 2 ------------------------------
//const u8 unitProp[][2] =	// Node tree structure 
//{
//    {MAIN_UNIT, 0},                  //  {parent, num_params },
//    {MAIN_UNIT, SYSTEM_VAR_NUM},     //  {parent, num_params },
//    {SYSTEM_UNIT, CONTROL_VAR_NUM},  //  {parent, num_params },
//    {SYSTEM_UNIT, DS18B20_VAR_NUM},  //  {parent, num_params },
//    {SYSTEM_UNIT, PARAM_VAR_NUM},    //  {parent, num_params },
//};
// Tree structure:
// DS18B20 + SHTC3-|
//                 |-System-|
//                          |-Control
//                          |-Temper + RH
//                          |-Parameters
//-------------------------------------------------------
func_prop Func_prop[] =		
{
    0,
	System_prop,	
	Control_prop,
	DS18B20_prop,
	Params_prop	
};
//-------------------------------------------------------
func_cmd Func_get[] =		
{
    0,
	System_get,	
	Control_get,
	DS18B20_get,
	Params_get	
};
//-------------------------------------------------------
func_cmd Func_set[] =		
{
    0,
	System_set,	
	Control_set,
	DS18B20_set,
	Params_set	
};
//-------------------------------------------------------

#pragma pack (4)

#ifdef __cplusplus
}
#endif
#endif /*__conf_init_H */


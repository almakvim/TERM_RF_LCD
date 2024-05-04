
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "main.h"
#include "control.h"
#include "usart.h"
#include "setup.h"

// #include "leftpump.h"
// #include "rightpump.h"


// Счетчик повторов
char   SETUP_FWCntr;

// Заводские установки
SETUP* pDefSetup =(SETUP*)FLASH_SETUP_BASE; //( страница 63)

// Текущие установки
SETUP Setup;

// Номер сегмента предыдущей записи
Uint32 SegAddr;

// Размер принятых данных прошивки (байты)
Uint32 SETUP_FWSize = 0;

// Вычисление стандартного CRC-16 (ARC)
Uint16 CRC16( void * addr, Uint32 size )
{
	char * data = addr;
	Uint16 crc = 0xFFFF;
	while( size-- )
		crc = ( crc >> 8 ) ^ CRC_Table[(crc & 0xFF) ^ (*data++ & 0xFF)];
	return crc;
}

// Загрузка настроек
void SETUP_Load()
{
//	SegAddr = FLASH_PR_BASE;
	// переписываем параметры из ROM в RAM
	Setup = *pDefSetup;
	
	// проверяем настройки

	if( Setup.CSum != CRC16( &Setup, sizeof( Setup ) - sizeof( Setup.CSum ) ))
	{
		// инициализируем настройки
		Setup.SerialId = 5040;
		Setup.humd = 1;
		Setup.hum_poz = 4;
	}

	dev_var.count_pkt_in = 0;
	dev_var.count_pkt_out = 0;
}

// Копирование новой прошивки
__ramfunc void SETUP_CopyFlash()
{
//	char * DataAddr = (char*)(FLASH_PR_BASE); 
//	Uint16 Data,RevData;
//	Uint16 Size = 0;

//	Uint32 Addr = SegAddr = FLASH_BOOT_BASE;
//	// стираем 
//	FL_Erase( Addr, Addr+FLASH_PROGRAM_SIZE );
//	
//	// записываем данные пока размер блока не равен FFFF
//	while( 1 )
//	{
//		Size = *DataAddr++;
//		Size += (Uint16)*DataAddr++ << 8;
//		if( Size == 0xFFFF ) break;
//		Addr = (Uint32)*DataAddr++ << 16;
//		Addr += (Uint32)*DataAddr++ << 24; 
//		Addr += *DataAddr++;
//		Addr += (Uint16)*DataAddr++ << 8;

//		while( Size-- )
//		{
//			Data = *((Uint16*)DataAddr);
//			RevData=((Data&0xFF)<<8)+((Data>>8)&0xFF);
//			SETUP_Write( &RevData, Addr , 2 );
//			DataAddr+=2;
//			Addr+=2;
//		}
//	}
//	
//	for(Size=0;Size<0xfff0;Size++);

////	SYSTEM_Reset();
//	__NVIC_SystemReset();
}

// Запись данных
//===========================================================================================
HAL_StatusTypeDef SETUP_Write( void * src, Uint32 addr, int size )
{
    HAL_StatusTypeDef res;
	uint16_t* data = src;
//    size = sizeof(Setup);
    uint16_t cursize = 0;
	size=size/2+((size%2>0)?1:0);
    
	while(cursize<size)
	{
        res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, *data);
        if(res != HAL_OK)
        {
           return res;
        }
		addr+=2;
		data++ ;
		cursize++;
    }
    return res;
}

//===========================================================================================
HAL_StatusTypeDef SETUP_Save()
{
	__disable_irq();
	// вычисляем контрольную сумму параметров
	Setup.CSum = CRC16( &Setup, sizeof(Setup) - sizeof(Setup.CSum ) );

    FLASH_EraseInitTypeDef EraseInitStruct;

    HAL_StatusTypeDef res;
    uint32_t Error = 1;
    
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_SETUP_START;
    EraseInitStruct.NbPages = 1;

    HAL_FLASH_Unlock();
	res = HAL_FLASHEx_Erase(&EraseInitStruct, &Error);

    res = SETUP_Write(&Setup, FLASH_SETUP_START, sizeof(Setup));
        
    HAL_FLASH_Lock();
    __enable_irq();
    
    HAL_Delay(50);
    
	return res;
}
//===========================================================================================


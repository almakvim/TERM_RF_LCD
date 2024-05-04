//--------------------------------------------------------------------------------------- 
#include "string.h"
#include "mk_conf_tree.h"
#include "setup.h"
#include "system.h"						
#include "control.h"
#include "ds18b20.h"						
//--------------------------------------------------------------------------------------- 


uint8_t crc,                // crc
n_datch = 0,                // количество датчиков установленых(записанных в EEPROM)
n_term = 0;                 // количество датчиков 
DATA_8 bufdt;               // буфер для чтения данных от DS18B20
DATA_8 rom_store[COLDAT];
float TEMPDAT[COLDAT];      // массив температур датчиков;
uint32_t DS_RespTime;
uint16_t term_num = 0;
uint16_t ds_count_start = 0;
uint32_t count_convert = 0;

//========================================================================================
__STATIC_INLINE void delay_uS(__IO uint32_t micros)
{
micros *= 8;//(SystemCoreClock / 1000000) / 6;
/* Wait till done */
while (micros--) ;
}
//--------------------------------------------------
//--------------------------------------------------
void port_init(void)
{
  HAL_GPIO_DeInit(TERM_GPIO_Port, TERM_Pin);
  TERM_GPIO_Port->CRH |= GPIO_CRH_MODE11;
  TERM_GPIO_Port->CRH |= GPIO_CRH_CNF11_0;
  TERM_GPIO_Port->CRH &= ~GPIO_CRH_CNF11_1;
}
//========================================================================================
//Функции работы с дачиками температуры DS18B20
//========================================================================================
//;Level_High: формирует высокий уровень на шине - режим чтение данных от DS18B20
uint8_t Level_HIGH (void)
{
    TERM_GPIO_Port->ODR |= GPIO_ODR_ODR11;//высокий уровень
	delay_uS(1);			//при 3 была ошибка 
    uint8_t status = TERM_GPIO_Port->IDR & GPIO_IDR_IDR11;//проверяем уровень
    return status;
}//--------------------------------------------------------------------------------------- 

//========================================================================================
// Level_LOW: формирует низкий уровень на шине
uint8_t Level_LOW (void)
{
    TERM_GPIO_Port->ODR &= ~GPIO_ODR_ODR11;//низкий уровень
	delay_uS(1);			//при 3 была ошибка 
    uint8_t status = TERM_GPIO_Port->IDR & GPIO_IDR_IDR11;//проверяем уровень
    return status;
}
//---------------------------------------------------------------------------------------

//========================================================================================
uint8_t DQ_StateGet (void)
{
    uint8_t bit = (TERM_GPIO_Port->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень	
    return bit;
}
//========================================================================================
// DLIT_WR: формирование паузы ожидания
// задержка 60 mks, в которая необходима DS1820 для чтения или записи бита данных.
void Waiting_WR (void)
{
	delay_uS(33);			//52 необходима длительность функции
}//---------------------------------------------------------------------------------------

//========================================================================================
//crc_bits: вычисление контрольной суммы crc8 для DS18B20
uint8_t CRC_BITS (int data) 
{
  int i = (data ^ crc) & 0xff;
  crc = 0;
	  if(i & 0b00000001)crc ^= 0x5e;
	  if(i & 0b00000010)crc ^= 0xbc;
	  if(i & 0b00000100)crc ^= 0x61;
	  if(i & 0b00001000)crc ^= 0xc2;
	  if(i & 0b00010000)crc ^= 0x9d;
	  if(i & 0b00100000)crc ^= 0x23;
	  if(i & 0b01000000)crc ^= 0x46;
	  if(i & 0b10000000)crc ^= 0x8c;
  return crc;
}//---------------------------------------------------------------------------------------

//========================================================================================
//RESET_DALLAS:формирование импульса сброса и тестирование присутствия датчика
// возвращает 	0 - связь с датчиком установлена
//				1 - нет связи постоянно высокий уровень (обрыв)
//				2 - нет связи постоянно низкий уровень  (замыкание)
uint8_t Reset_Dallas (void)
{
    uint8_t ERR_LINE;       // линния исправна связи датчика иcправна
    uint8_t a;
    Level_LOW();			//сформировать импульс сброса >480 mks
    delay_uS(490);			//задержка 700 mks

// контроль импульса присутсвия, проверка состояниия линии данных
    ERR_LINE=0;     // инициализация бита ошибки - линния исправна связи датчика иcправна
//    di();           //запретить прерывания			

    Level_HIGH();				//установить высокий уровень
//	delay_uS(15);				//задержка 15 mks
// ожидание низкого уровня импульса присутствия
    a=0;
    while (ERR_LINE==0&&DQ_StateGet()==1)
    {
        delay_uS(4);			//задержка 5 mks
        if(++a>120)ERR_LINE=1; 	// нет ответа - обрыв линии данных
    }	 
//    ei();//разрешить прерывания

// ожидание окончания импульса присутствия, ждем высокий уровень	
    a=0;
    while (ERR_LINE==0&&DQ_StateGet()==0)
    {
        delay_uS(4);			//задержка 5 us
        if(++a>120)ERR_LINE=2; 	// нет ответа - замыкание линии данных
    }	 

    delay_uS(490);			//задержка  >480 mks

//задержка аварии, защита от возможных помех		
			
		//	ERR_LINE=0;//test alarm
		return ERR_LINE;
}//---------------------------------------------------------------------------------------




//========================================================================================
// Dreceive: Эта подпрограмма читает данные из DS18B20.
uint8_t Dreceive (void)
{
    uint8_t COM_REG,a;	
	COM_REG=0;
//    	di();                               // запретить прерывания
	for (a=0; a<8; a++)						// цикл чтение от даласса шины
	{										//
		Level_LOW ();						// старт чтения бита установить низкий уровень)
		Level_HIGH ();						// начать чтение
		COM_REG=COM_REG>>1;					// сдвиг + запись в 7 разряд 0
		if(DQ_StateGet()==1)COM_REG |= 0b10000000;// установка в 7 разряде 1 если DALLAS==1
		Waiting_WR();						//ожидать для окончания формирование далласом бита
	}
//		ei();                               // функция включить прерывания
return COM_REG;
}//---------------------------------------------------------------------------------------

//========================================================================================
// Dsend: Эта подпрограмма посылает команды к DS1820.
void Dsend(char COM_REG) 
{
    uint8_t a;
//    	di();					// запретить прерывания
	for (a=0; a<8; a++)	
	{
		Level_LOW ();		
		if(COM_REG & 0b00000001) Level_HIGH ();
		COM_REG=COM_REG>>1;	
		Waiting_WR ();
	 	Level_HIGH ();
	}
//		ei();                   // функция включить прерывания
}//---------------------------------------------------------------------------------------

//========================================================================================
uint8_t Convert_ds18b20 (void)
{
    uint8_t a; // внутреняя переменная цикла

	a=Reset_Dallas();   // формирование импульса сброса
    if (a==0)
    {
        Dsend (0xCC);   // команда для всех устройств (Пропуск ROM)
        Dsend (0x44);   // конверитирование температуры
        return 1;       //запущено конвертирование
    }
    else
    {
        if (a==1) return 3; //ошибка линии данных-нет датчика, обрыв на шине высокий уровень)
        else return 4;      //ошибка линии данных-замыкание на чине (низкий уровень)
    }
//    return 0;
}
//========================================================================================
float ds18b20_tconvert(DATA_2 temp)
{
    float arg, data;
    
    int16_t temperature;
    
    temperature = temp.dt_8[0] | (temp.dt_8[1] << 8);

    arg = (float)temperature;
    data = arg*0.0625f;
    
    return data ;
}
//========================================================================================
float ds18b20_get_temp (uint8_t curr)
{
    uint16_t i; // внутреняя переменная цикла
    DATA_2 temp;
    
    // проверка наличие адреса следующего датчика в ПЗУ
    // если первый байт == 0х28, значит датчик существует.
    if (rom_store[curr].dt_8[0] != 0x28)
    {
        return -56.1f; // ошибка
    }	


    if (Reset_Dallas()==0)// формирование импульса сброса с проверкой импульса присутствия датчиков
    {
        Dsend(0x55); // команда вызов датчика в соотвествии с его адресом

        // загрузка адреса датчика температуры с которым предстоит работать
        for (i=0;i<8;i++)
        {
            Dsend(rom_store[curr].dt_8[i]);
        }
    }
    else
    {
        return -56.2f; // ошибка линии связи
    }						

//	чтение температуры и проверка crc
    Dsend(0xBE);                // команда чтения памяти
    crc=0;                      // обнулить байт контрольной суммы			
    temp.dt_8[0]=Dreceive(); 	// читаем байт 01
    CRC_BITS(temp.dt_8[0]);
    temp.dt_8[1]=Dreceive(); 	// читаем байт 02
    CRC_BITS(temp.dt_8[1]);
    CRC_BITS(Dreceive());       // читаем байт 03
    CRC_BITS(Dreceive());       // читаем байт 04
    CRC_BITS(Dreceive());       // читаем байт 05
    CRC_BITS(Dreceive());       // читаем байт 06
    CRC_BITS(Dreceive());       // читаем байт 07
    CRC_BITS(Dreceive());       // читаем байт 08
    CRC_BITS(Dreceive());       // читаем байт 09

// обработка ошибки, задержка аварии защита от возможных помех	
    if(crc!=0)		// 0=обычный режим работы и количество ошибок <10
    {
        return -56.3f; // ошибка
    }				
    else
    {
//        TEST_RC1_On();
        return( ds18b20_tconvert(temp)); // преобразование кода температуры датчика
    }
        
//	return -56.4f; // ошибка
}//---------------------------------------------------------------------------------------


//========================================================================================
/*seachROM: - функция поиска сетевого адреса датчика
позволяет последовательно читать все адреса датчиков температуры установленные в сети.
внешние переменные 
bayt08,bayt07,bayt06,bayt05,bayt04,bayt03,bayt02,bayt01 - текущее значение РОМ датчика
chetchik_bit - счетчик который определяет какой на данный момент читается бит РОМ датчика
tekuc_nesoot - регистр который хранит на каком шаге зафиксирована текущее несоответствие при чтении битов РОМ датчика
posled_nesoot - счетчик который хранит число несоответствий от предыдущего чтения РОМ датчика
возвращает значение
0- все адреса датчиков считаны
1- считан текущий адрес, но еще есть не считанные адреса
2- в считанном адресе ошибка crc
3- выход по аварии нет связи с ДТ
4- не корректный ответ при чтении бита данных А=В=1 (сбой шины)
 */
uint16_t SeachROM (void)
{
    uint8_t chetchik_bit, tekuc_nesoot;	
    static uint8_t bA, bB, posled_nesoot = 0;	
			
    tekuc_nesoot=0;			// сбросить маркер текущего несоответствия в ноль

    if (Reset_Dallas()!=0)	// формировать начала обмена
    {
        return 3; 			// выход по аварии нет связи с ДТ
    }	

    Dsend (0xF0); 			// послать команду поиск ROM			

    for (chetchik_bit=0;chetchik_bit<64;chetchik_bit++)
    {
        //прочитать 2 байта с шины
//        di();					// запретить прерывания
        Level_LOW ();			// старт чтения бита установить низкий уровень)
        Level_HIGH ();			// начать чтение
        bA=0;
        if (DQ_StateGet()==1)bA=1; 	// 
        Waiting_WR();			// ожидать для окончания формирование далласом бита

        Level_LOW ();			// старт чтения бита установить низкий уровень)
        Level_HIGH ();			// начать чтение
        bB=0;
        if (DQ_StateGet()==1)bB=1; 	// 
        Waiting_WR();			// ожидать для окончания формирование далласом бита
//        ei();					// функция включить прерывания 

        //проверяем биты (А) и (В)
        if (bA && bB) 			// если А=В=1 - то это ошибка
        {
            posled_nesoot=0; 	// очистить индикатор последнего несоответствия	
            return	4; 			// не корректный ответ при чтении бита данных А=В=1
        }

        if(!bA && !bB)			// если А=В=0 на шине присутствуют несколько датчиков
        {
            //ситуация несоответствия, алгоритм "разруливания"
            if (chetchik_bit==posled_nesoot)
            {
                bA=1; // установить 1
            }
            else
            {
                if (chetchik_bit>posled_nesoot)
                {
                    tekuc_nesoot=chetchik_bit; // новое текушее несоответсвие - new current mismatch
                    bA=0;  // установить 0
                }
                else //chetchik_bit<posled_nesoot
                {
                    if (bufdt.dt_8[0] & 0b00000001)
                    {
                         bA=1; // установить 1 - set to 1
                    }
                    else
                    {
                        tekuc_nesoot=chetchik_bit;  // новое текушее несоответсвие - new current mismatch
                        bA=0;  // установить 0 - set to 0	
                    }
                }
            }
        }					
        // формирование получаемого адреса ДТ 				 	

        bufdt.dt_8[0]>>=1;
        if (bufdt.dt_8[1] & 0b00000001)bufdt.dt_8[0] |= 0b10000000;
        bufdt.dt_8[1]>>=1;
        if (bufdt.dt_8[2] & 0b00000001)bufdt.dt_8[1] |= 0b10000000;		
        bufdt.dt_8[2]>>=1;
        if (bufdt.dt_8[3] & 0b00000001)bufdt.dt_8[2] |= 0b10000000;
        bufdt.dt_8[3]>>=1;
        if (bufdt.dt_8[4] & 0b00000001)bufdt.dt_8[3] |= 0b10000000;
        bufdt.dt_8[4]>>=1;
        if (bufdt.dt_8[5] & 0b00000001)bufdt.dt_8[4] |= 0b10000000;
        bufdt.dt_8[5]>>=1;
        if (bufdt.dt_8[6] & 0b00000001)bufdt.dt_8[5] |= 0b10000000;		
        bufdt.dt_8[6]>>=1;
        if (bufdt.dt_8[7] & 0b00000001)bufdt.dt_8[6] |= 0b10000000;
        bufdt.dt_8[7]>>=1;
        if (bA==1)bufdt.dt_8[7] |= 0b10000000;

//        di();						// запретить прерывания
        // формирует на шине 0 или 1 в зависимости от бита (А)	
        Level_LOW(); 				// строб
        if (bA) Level_HIGH(); 		// если 1 формируем единицу
    //	else Level_LOW(); 			// иначе формируем ноль
        Waiting_WR();				// ожидания окончания
        Level_HIGH();				// возврат линии в 1
//        ei();						// функция включить прерывания 
    // повторяем цикл 64 раза
    }						

    // определить текущее несоответствие последним
    posled_nesoot = tekuc_nesoot;

    // проверка crc полученног адреса		
    crc=0;                      // обнулить бaйт контрольной суммы			
    CRC_BITS(bufdt.dt_8[0]); 	// читаем байт 01
    CRC_BITS(bufdt.dt_8[1]);	// читаем байт 02
    CRC_BITS(bufdt.dt_8[2]); 	// читаем байт 03
    CRC_BITS(bufdt.dt_8[3]); 	// читаем байт 04
    CRC_BITS(bufdt.dt_8[4]); 	// читаем байт 05
    CRC_BITS(bufdt.dt_8[5]); 	// читаем байт 06
    CRC_BITS(bufdt.dt_8[6]); 	// читаем байт 07
    CRC_BITS(bufdt.dt_8[7]); 	// читаем байт 08

    // контроль crc
    if (crc==0)
    {
//        TEST_RC1_On();
        if (posled_nesoot == 0)
        {
            return 0; // все адреса датчиков считаны
        }	
        return 1; // считан текущий адрес, но еще есть не считанные адреса
    }		
    return 2; // в считанном адресе ошибка crc

}


//========================================================================================
void ds18b20_proc(void)
{
    static uint32_t timeout_proc = 0;
    static uint16_t flag_start = 0;
    uint16_t curr, m;
    
//    if(start_save) return;
    
    if(flag_start == 0)
    {
        flag_start = 1;
        
        curr = 0; m = 1; //=1
        
        while(!((m==0) || (m==2) || (m==3) || (m==4)))
        {
            m = SeachROM();
            //m = 0;
            if((m==1) || (m==0))
            {
                rom_store[curr++] = bufdt;
                n_datch = curr;
            }
        }
        
        timeout_proc = HAL_GetTick();
        return;
    }
    
    if((timeout_proc+1000 <= HAL_GetTick())/*&&(n_datch)*/)
    {
        timeout_proc = HAL_GetTick();
        if(n_datch<1) 
        {
            flag_start = 0;
        }
        else
        {
            for(curr = 0; curr < n_datch; curr++)
            {
                TEMPDAT[curr] = ds18b20_get_temp(curr);
                if(TEMPDAT[curr] > -56.0f)
                {
                    dev_var.term_real[curr] = TEMPDAT[curr];
                }
            }
            Convert_ds18b20();
        }
    }
}

//========================================================================================
uint8_t DS18B20_prop( uint8_t par, uint8_t * name, uint8_t * prop, uint8_t * nbyte )
{
	char * str;
    uint8_t size = 0;
	if( name ) { switch( par )
        {
        case DS18B20_NTERM     :	str = "Float dig test"; break;
        case DS18B20_NDATCH    :	str = "Кол-во датч. после поиска"; break;
        case DS18B20_TEMPER1   :	str = "Температура 1 (гр.)"; break;
        case DS18B20_TEMPER2   :	str = "Температура 2 (гр.)"; break;
        case DS18B20_TEMPER3   :	str = "Температура 3 (гр.)"; break;
        case DS18B20_TEMPER4   :	str = "Температура 4 (гр.)"; break;
        case DS18B20_HUMDATA   :	str = "Влажность (RH%)"; break;
        default: return 0;
        }
        while( *str ) { *name++ = *str++; size++; } *name = 0; 
    }
	if( prop ) switch( par )
        {
        case DS18B20_NTERM       :	 *prop = REAL; break;
        case DS18B20_HUMDATA     :
        case DS18B20_NDATCH      :	 *prop = UINT|RO; break;
        case DS18B20_TEMPER1     :
        case DS18B20_TEMPER2     :
        case DS18B20_TEMPER3     :
        case DS18B20_TEMPER4     :	 *prop = REAL|RO; break;
        default: return 0;
        }
	if( nbyte ) switch( par )
        {
        case DS18B20_NTERM       :	 *nbyte = 4; break;
        case DS18B20_HUMDATA     :
        case DS18B20_NDATCH      :	 *nbyte = 1; break;
        case DS18B20_TEMPER1     :
        case DS18B20_TEMPER2     :
        case DS18B20_TEMPER3     :
        case DS18B20_TEMPER4     :	 *nbyte = 4; break;
        default: return 0;
        }
    return size;
}
//============================================================================
void DS18B20_get(uint8_t par, void * value)
{
    switch( par )
    {
		case DS18B20_NTERM      :	*(float*)value = dev_var.term_test; break;
		case DS18B20_NDATCH     :	*(u8*)value = n_datch; break;
		case DS18B20_TEMPER1    :	*(float*)value = dev_var.term_real[0]; break;
		case DS18B20_TEMPER2    :	*(float*)value = dev_var.term_real[1]; break;
		case DS18B20_TEMPER3    :	*(float*)value = dev_var.term_real[2]; break;
		case DS18B20_TEMPER4    :	*(float*)value = dev_var.term_real[3]; break;
		case DS18B20_HUMDATA    :	*(u8*)value = (u8)dev_var.shtc3_hum; break;
        default:;
    }
}
//============================================================================
void DS18B20_set(uint8_t par, void * value)
{
	float f = *(float*)value;
    switch( par )
    {
		case DS18B20_NTERM     :	dev_var.term_test = f; break;
        default:;
    }
}
//============================================================================

//--------------------------------------------------------------------------------------- 
#include "string.h"
#include "mk_conf_tree.h"
#include "setup.h"
#include "system.h"						
#include "control.h"
#include "ds18b20.h"						
//--------------------------------------------------------------------------------------- 


uint8_t crc,                // crc
n_datch = 0,                // ���������� �������� ������������(���������� � EEPROM)
n_term = 0;                 // ���������� �������� 
DATA_8 bufdt;               // ����� ��� ������ ������ �� DS18B20
DATA_8 rom_store[COLDAT];
float TEMPDAT[COLDAT];      // ������ ���������� ��������;
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
//������� ������ � �������� ����������� DS18B20
//========================================================================================
//;Level_High: ��������� ������� ������� �� ���� - ����� ������ ������ �� DS18B20
uint8_t Level_HIGH (void)
{
    TERM_GPIO_Port->ODR |= GPIO_ODR_ODR11;//������� �������
	delay_uS(1);			//��� 3 ���� ������ 
    uint8_t status = TERM_GPIO_Port->IDR & GPIO_IDR_IDR11;//��������� �������
    return status;
}//--------------------------------------------------------------------------------------- 

//========================================================================================
// Level_LOW: ��������� ������ ������� �� ����
uint8_t Level_LOW (void)
{
    TERM_GPIO_Port->ODR &= ~GPIO_ODR_ODR11;//������ �������
	delay_uS(1);			//��� 3 ���� ������ 
    uint8_t status = TERM_GPIO_Port->IDR & GPIO_IDR_IDR11;//��������� �������
    return status;
}
//---------------------------------------------------------------------------------------

//========================================================================================
uint8_t DQ_StateGet (void)
{
    uint8_t bit = (TERM_GPIO_Port->IDR & GPIO_IDR_IDR11 ? 1 : 0);//��������� �������	
    return bit;
}
//========================================================================================
// DLIT_WR: ������������ ����� ��������
// �������� 60 mks, � ������� ���������� DS1820 ��� ������ ��� ������ ���� ������.
void Waiting_WR (void)
{
	delay_uS(33);			//52 ���������� ������������ �������
}//---------------------------------------------------------------------------------------

//========================================================================================
//crc_bits: ���������� ����������� ����� crc8 ��� DS18B20
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
//RESET_DALLAS:������������ �������� ������ � ������������ ����������� �������
// ���������� 	0 - ����� � �������� �����������
//				1 - ��� ����� ��������� ������� ������� (�����)
//				2 - ��� ����� ��������� ������ �������  (���������)
uint8_t Reset_Dallas (void)
{
    uint8_t ERR_LINE;       // ������ �������� ����� ������� �c������
    uint8_t a;
    Level_LOW();			//������������ ������� ������ >480 mks
    delay_uS(490);			//�������� 700 mks

// �������� �������� ����������, �������� ���������� ����� ������
    ERR_LINE=0;     // ������������� ���� ������ - ������ �������� ����� ������� �c������
//    di();           //��������� ����������			

    Level_HIGH();				//���������� ������� �������
//	delay_uS(15);				//�������� 15 mks
// �������� ������� ������ �������� �����������
    a=0;
    while (ERR_LINE==0&&DQ_StateGet()==1)
    {
        delay_uS(4);			//�������� 5 mks
        if(++a>120)ERR_LINE=1; 	// ��� ������ - ����� ����� ������
    }	 
//    ei();//��������� ����������

// �������� ��������� �������� �����������, ���� ������� �������	
    a=0;
    while (ERR_LINE==0&&DQ_StateGet()==0)
    {
        delay_uS(4);			//�������� 5 us
        if(++a>120)ERR_LINE=2; 	// ��� ������ - ��������� ����� ������
    }	 

    delay_uS(490);			//��������  >480 mks

//�������� ������, ������ �� ��������� �����		
			
		//	ERR_LINE=0;//test alarm
		return ERR_LINE;
}//---------------------------------------------------------------------------------------




//========================================================================================
// Dreceive: ��� ������������ ������ ������ �� DS18B20.
uint8_t Dreceive (void)
{
    uint8_t COM_REG,a;	
	COM_REG=0;
//    	di();                               // ��������� ����������
	for (a=0; a<8; a++)						// ���� ������ �� ������� ����
	{										//
		Level_LOW ();						// ����� ������ ���� ���������� ������ �������)
		Level_HIGH ();						// ������ ������
		COM_REG=COM_REG>>1;					// ����� + ������ � 7 ������ 0
		if(DQ_StateGet()==1)COM_REG |= 0b10000000;// ��������� � 7 ������� 1 ���� DALLAS==1
		Waiting_WR();						//������� ��� ��������� ������������ �������� ����
	}
//		ei();                               // ������� �������� ����������
return COM_REG;
}//---------------------------------------------------------------------------------------

//========================================================================================
// Dsend: ��� ������������ �������� ������� � DS1820.
void Dsend(char COM_REG) 
{
    uint8_t a;
//    	di();					// ��������� ����������
	for (a=0; a<8; a++)	
	{
		Level_LOW ();		
		if(COM_REG & 0b00000001) Level_HIGH ();
		COM_REG=COM_REG>>1;	
		Waiting_WR ();
	 	Level_HIGH ();
	}
//		ei();                   // ������� �������� ����������
}//---------------------------------------------------------------------------------------

//========================================================================================
uint8_t Convert_ds18b20 (void)
{
    uint8_t a; // ��������� ���������� �����

	a=Reset_Dallas();   // ������������ �������� ������
    if (a==0)
    {
        Dsend (0xCC);   // ������� ��� ���� ��������� (������� ROM)
        Dsend (0x44);   // ���������������� �����������
        return 1;       //�������� ���������������
    }
    else
    {
        if (a==1) return 3; //������ ����� ������-��� �������, ����� �� ���� ������� �������)
        else return 4;      //������ ����� ������-��������� �� ���� (������ �������)
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
    uint16_t i; // ��������� ���������� �����
    DATA_2 temp;
    
    // �������� ������� ������ ���������� ������� � ���
    // ���� ������ ���� == 0�28, ������ ������ ����������.
    if (rom_store[curr].dt_8[0] != 0x28)
    {
        return -56.1f; // ������
    }	


    if (Reset_Dallas()==0)// ������������ �������� ������ � ��������� �������� ����������� ��������
    {
        Dsend(0x55); // ������� ����� ������� � ����������� � ��� �������

        // �������� ������ ������� ����������� � ������� ��������� ��������
        for (i=0;i<8;i++)
        {
            Dsend(rom_store[curr].dt_8[i]);
        }
    }
    else
    {
        return -56.2f; // ������ ����� �����
    }						

//	������ ����������� � �������� crc
    Dsend(0xBE);                // ������� ������ ������
    crc=0;                      // �������� ���� ����������� �����			
    temp.dt_8[0]=Dreceive(); 	// ������ ���� 01
    CRC_BITS(temp.dt_8[0]);
    temp.dt_8[1]=Dreceive(); 	// ������ ���� 02
    CRC_BITS(temp.dt_8[1]);
    CRC_BITS(Dreceive());       // ������ ���� 03
    CRC_BITS(Dreceive());       // ������ ���� 04
    CRC_BITS(Dreceive());       // ������ ���� 05
    CRC_BITS(Dreceive());       // ������ ���� 06
    CRC_BITS(Dreceive());       // ������ ���� 07
    CRC_BITS(Dreceive());       // ������ ���� 08
    CRC_BITS(Dreceive());       // ������ ���� 09

// ��������� ������, �������� ������ ������ �� ��������� �����	
    if(crc!=0)		// 0=������� ����� ������ � ���������� ������ <10
    {
        return -56.3f; // ������
    }				
    else
    {
//        TEST_RC1_On();
        return( ds18b20_tconvert(temp)); // �������������� ���� ����������� �������
    }
        
//	return -56.4f; // ������
}//---------------------------------------------------------------------------------------


//========================================================================================
/*seachROM: - ������� ������ �������� ������ �������
��������� ��������������� ������ ��� ������ �������� ����������� ������������� � ����.
������� ���������� 
bayt08,bayt07,bayt06,bayt05,bayt04,bayt03,bayt02,bayt01 - ������� �������� ��� �������
chetchik_bit - ������� ������� ���������� ����� �� ������ ������ �������� ��� ��� �������
tekuc_nesoot - ������� ������� ������ �� ����� ���� ������������� ������� �������������� ��� ������ ����� ��� �������
posled_nesoot - ������� ������� ������ ����� �������������� �� ����������� ������ ��� �������
���������� ��������
0- ��� ������ �������� �������
1- ������ ������� �����, �� ��� ���� �� ��������� ������
2- � ��������� ������ ������ crc
3- ����� �� ������ ��� ����� � ��
4- �� ���������� ����� ��� ������ ���� ������ �=�=1 (���� ����)
 */
uint16_t SeachROM (void)
{
    uint8_t chetchik_bit, tekuc_nesoot;	
    static uint8_t bA, bB, posled_nesoot = 0;	
			
    tekuc_nesoot=0;			// �������� ������ �������� �������������� � ����

    if (Reset_Dallas()!=0)	// ����������� ������ ������
    {
        return 3; 			// ����� �� ������ ��� ����� � ��
    }	

    Dsend (0xF0); 			// ������� ������� ����� ROM			

    for (chetchik_bit=0;chetchik_bit<64;chetchik_bit++)
    {
        //��������� 2 ����� � ����
//        di();					// ��������� ����������
        Level_LOW ();			// ����� ������ ���� ���������� ������ �������)
        Level_HIGH ();			// ������ ������
        bA=0;
        if (DQ_StateGet()==1)bA=1; 	// 
        Waiting_WR();			// ������� ��� ��������� ������������ �������� ����

        Level_LOW ();			// ����� ������ ���� ���������� ������ �������)
        Level_HIGH ();			// ������ ������
        bB=0;
        if (DQ_StateGet()==1)bB=1; 	// 
        Waiting_WR();			// ������� ��� ��������� ������������ �������� ����
//        ei();					// ������� �������� ���������� 

        //��������� ���� (�) � (�)
        if (bA && bB) 			// ���� �=�=1 - �� ��� ������
        {
            posled_nesoot=0; 	// �������� ��������� ���������� ��������������	
            return	4; 			// �� ���������� ����� ��� ������ ���� ������ �=�=1
        }

        if(!bA && !bB)			// ���� �=�=0 �� ���� ������������ ��������� ��������
        {
            //�������� ��������������, �������� "������������"
            if (chetchik_bit==posled_nesoot)
            {
                bA=1; // ���������� 1
            }
            else
            {
                if (chetchik_bit>posled_nesoot)
                {
                    tekuc_nesoot=chetchik_bit; // ����� ������� ������������� - new current mismatch
                    bA=0;  // ���������� 0
                }
                else //chetchik_bit<posled_nesoot
                {
                    if (bufdt.dt_8[0] & 0b00000001)
                    {
                         bA=1; // ���������� 1 - set to 1
                    }
                    else
                    {
                        tekuc_nesoot=chetchik_bit;  // ����� ������� ������������� - new current mismatch
                        bA=0;  // ���������� 0 - set to 0	
                    }
                }
            }
        }					
        // ������������ ����������� ������ �� 				 	

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

//        di();						// ��������� ����������
        // ��������� �� ���� 0 ��� 1 � ����������� �� ���� (�)	
        Level_LOW(); 				// �����
        if (bA) Level_HIGH(); 		// ���� 1 ��������� �������
    //	else Level_LOW(); 			// ����� ��������� ����
        Waiting_WR();				// �������� ���������
        Level_HIGH();				// ������� ����� � 1
//        ei();						// ������� �������� ���������� 
    // ��������� ���� 64 ����
    }						

    // ���������� ������� �������������� ���������
    posled_nesoot = tekuc_nesoot;

    // �������� crc ���������� ������		
    crc=0;                      // �������� �a�� ����������� �����			
    CRC_BITS(bufdt.dt_8[0]); 	// ������ ���� 01
    CRC_BITS(bufdt.dt_8[1]);	// ������ ���� 02
    CRC_BITS(bufdt.dt_8[2]); 	// ������ ���� 03
    CRC_BITS(bufdt.dt_8[3]); 	// ������ ���� 04
    CRC_BITS(bufdt.dt_8[4]); 	// ������ ���� 05
    CRC_BITS(bufdt.dt_8[5]); 	// ������ ���� 06
    CRC_BITS(bufdt.dt_8[6]); 	// ������ ���� 07
    CRC_BITS(bufdt.dt_8[7]); 	// ������ ���� 08

    // �������� crc
    if (crc==0)
    {
//        TEST_RC1_On();
        if (posled_nesoot == 0)
        {
            return 0; // ��� ������ �������� �������
        }	
        return 1; // ������ ������� �����, �� ��� ���� �� ��������� ������
    }		
    return 2; // � ��������� ������ ������ crc

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
        case DS18B20_NDATCH    :	str = "���-�� ����. ����� ������"; break;
        case DS18B20_TEMPER1   :	str = "����������� 1 (��.)"; break;
        case DS18B20_TEMPER2   :	str = "����������� 2 (��.)"; break;
        case DS18B20_TEMPER3   :	str = "����������� 3 (��.)"; break;
        case DS18B20_TEMPER4   :	str = "����������� 4 (��.)"; break;
        case DS18B20_HUMDATA   :	str = "��������� (RH%)"; break;
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

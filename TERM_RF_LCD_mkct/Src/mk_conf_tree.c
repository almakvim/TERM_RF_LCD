
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "USART.h"
#include "control.h"
#include "conf_init.h"
#include "time.h"
#include "setup.h"
#include "mk_conf_tree.h"

MKBUS mkChan1;
u8 txSend[BUF_SIZE];
DATA_32 dev_simb;
//==============================================================
// Вычисление стандартного CRC-16 (ARC)
// ------------------------------------
uint16_t MKBUS_CRC16( uint8_t * data, uint16_t tail, uint16_t size )
{
	Uint16 crc = 0xFFFF;
	while( size-- )
	{
		crc = ( crc >> 8 ) ^ CRC_Table[(crc & 0xFF) ^ (data[tail] & 0xFF)];
		tail = (tail + 1) & BUF_MASK; 
	}
	return crc;
}
//--------------------------------------------
uint16_t tx_crc16( void * addr, uint16_t size )
{
	char * data = addr;
	uint16_t crc = 0xFFFF;
	while( size-- )
		crc = ( crc >> 8 ) ^ CRC_Table[(crc & 0xFF) ^ (*data++ & 0xFF)];
	return crc;
}
//=========================================================================
void MKBUS_send(u8 *buf, u16 ln)
{
	u16 i = 0, len = ln;
 	u8 * dt = buf;
    
    while(len--)
    {
    	USART_Write(USART2, dt[i]);
    	i++;
    }
	dev_var.count_pkt_out++;
}
//=========================================================================
void MKBUS_msg_cmd1(MKBUS *chan)
{
    u16 len = 0;
    chan->pkt[len++] = 0xca;
    chan->pkt[len++] = 0xfd;
    chan->pkt[len++] = Setup.SerialId;
    chan->pkt[len++] = Setup.SerialId>>8;
    chan->pkt[len++] = 2;
    chan->pkt[len++] = 1;
    chan->pkt[len++] = N_UNIT;
    uint16_t crc = tx_crc16(chan->pkt, 7);
    chan->pkt[len++] = crc;
    chan->pkt[len++] = crc>>8;
    chan->txLen = len;
    chan->time = HAL_GetTick();
}
//=========================================================================
void MKBUS_msg_cmd2(MKBUS *chan, uint8_t unit)
{
    if(unit >= N_UNIT)return;
    
    u16 len = 0;
    chan->pkt[len++] = 0xca;
    chan->pkt[len++] = 0xfd;
    chan->pkt[len++] = Setup.SerialId;
    chan->pkt[len++] = Setup.SerialId>>8;
    chan->pkt[len++] = 0;
    chan->pkt[len++] = 2;
    chan->pkt[len++] = unitProp[unit][0];
    chan->pkt[len++] = unit;
    chan->pkt[len++] = unitProp[unit][1];
    sprintf((char*)&chan->pkt[len], "%s",namesUnit[unit]);       // Имя узла
	len += strlen(namesUnit[unit]);
    chan->pkt[4] = len - 5;
    uint16_t crc = tx_crc16(chan->pkt, len);
    chan->pkt[len++] = crc;
    chan->pkt[len++] = crc>>8;
    chan->txLen = len;
    chan->time = HAL_GetTick();
}
//=========================================================================
void MKBUS_msg_cmd3(MKBUS *chan, uint8_t unit, uint8_t par)
{
    if(unit >= N_UNIT)return;
    if(par >= unitProp[unit][1])return;
    
    u8 buf[32];
    u8 prop = 0;
    u8 parlen = 0;
    u16 len = 0;
    u8 ln = Func_prop[unit](par, buf, &prop, &parlen );
    chan->pkt[len++] = 0xca;
    chan->pkt[len++] = 0xfd;
    chan->pkt[len++] = Setup.SerialId;
    chan->pkt[len++] = Setup.SerialId>>8;
    chan->pkt[len++] = 0;
    chan->pkt[len++] = 3;
    chan->pkt[len++] = unit;
    chan->pkt[len++] = par;
    chan->pkt[len++] = prop;
    chan->pkt[len++] = parlen;
    MemCopy(buf, &chan->pkt[len], ln);
	len += ln;
    chan->pkt[4] = len - 5;
    uint16_t crc = tx_crc16(chan->pkt, len);
    chan->pkt[len++] = crc;
    chan->pkt[len++] = crc>>8;
    chan->txLen = len;
    chan->time = HAL_GetTick();
}
//=========================================================================
void MKBUS_msg_cmd4(MKBUS *chan, uint8_t unit, uint8_t par, uint8_t num)
{
    if(unit >= N_UNIT)return;
    if(par >= unitProp[unit][1])return;
    if(num == 0)return;
    if((unitProp[unit][1]-par) < num)return;
    
    u16 len = 0;
    chan->pkt[len++] = 0xca;
    chan->pkt[len++] = 0xfd;
    chan->pkt[len++] = Setup.SerialId;
    chan->pkt[len++] = Setup.SerialId>>8;
    chan->pkt[len++] = 0;   // Длина ответа
    chan->pkt[len++] = 4;
    chan->pkt[len++] = unit;
    chan->pkt[len++] = par; // Номер первой переменной  (начиная с нуля) 
    chan->pkt[len++] = num; // Количество переменных
    for(u16 i=par; i<num+par; i++){
        u8 parlen = 0;
        u8 buf[32];
        Func_prop[unit](i, 0, 0, &parlen );
        u16 ln = chan->pkt[len++] = parlen;
        Func_get[unit](i, buf);
        MemCopy(buf, &chan->pkt[len], ln);
        len += ln;
    }
    chan->pkt[4] = len - 5;
    uint16_t crc = tx_crc16(chan->pkt, len);
    chan->pkt[len++] = crc;
    chan->pkt[len++] = crc>>8;
    chan->txLen = len;
    chan->time = HAL_GetTick();
}

//=========================================================================
void MKBUS_msg_cmd5(MKBUS *chan, uint8_t unit, uint8_t par)
{
    u16 len = 0;
    chan->pkt[len++] = 0xca;
    chan->pkt[len++] = 0xfd;
    chan->pkt[len++] = Setup.SerialId;
    chan->pkt[len++] = Setup.SerialId>>8;
    chan->pkt[len++] = 3;
    chan->pkt[len++] = 5;
    chan->pkt[len++] = unit;
    chan->pkt[len++] = par;
    uint16_t crc = tx_crc16(chan->pkt, len);
    chan->pkt[len++] = crc;
    chan->pkt[len++] = crc>>8;
    chan->txLen = len;
    chan->time = HAL_GetTick();
}

//=========================================================================
Uint16 RW( uint8_t * buf, int tail  )
{
	return (Uint16)buf[tail] + ((Uint16)buf[(tail + 1) & BUF_MASK] << 8);
}

//-------------------------------------
Uint32 RDW( uint8_t * buf, int tail )
{
	return (Uint32)buf[tail] + ((Uint32)buf[(tail + 1) & BUF_MASK] << 8) +
	((Uint32)buf[(tail + 2) & BUF_MASK] << 16) + ((Uint32)buf[(tail + 3) & BUF_MASK] << 24);
}

//-------------------------------------
void WW( uint8_t * buf, Uint16 word )
{
	*buf++ = word & 0xFF;
	*buf++ = word >> 8;
}

//-------------------------------------
void WDW( uint8_t * buf, Uint32 dword )
{
	*buf++ = dword & 0xFF;
	*buf++ = ( dword >> 8  ) & 0xFF;
	*buf++ = ( dword >> 16 ) & 0xFF;
	*buf   = ( dword >> 24 ) & 0xFF;
}
//=========================================================================
void MKBUS_rx(MKBUS *chan, u8 dt)
{
	chan->rxBuf[chan->tail = (chan->tail + 1) & BUF_MASK] = dt;
    
    if(chan->state == WAIT_HEAD){
        u16 header = RW( &chan->rxBuf[0],  (chan->tail - 4) & BUF_MASK);
        u16 id = RW( &chan->rxBuf[0],  (chan->tail - 2) & BUF_MASK);
        if((header == 0xfeca)&&(id == Setup.SerialId)){
            chan->head = (chan->tail - 4) & BUF_MASK;
            chan->state = WAIT_DATA;
            chan->rxLen = dt + 5;
            chan->rxSize = dt + 1;
        }
    } else if(chan->state == WAIT_DATA){
        if( chan->rxSize-- != 0 ) return;
        chan->state = WAIT_HEAD;
        u16 crc_1 = MKBUS_CRC16(&chan->rxBuf[0], chan->head, chan->rxLen );
        u16 crc_2 = RW(&chan->rxBuf[0], (chan->tail - 1) & BUF_MASK);
        u8 unit = chan->rxBuf[(chan->head + 6) & BUF_MASK];
        u8 par = chan->rxBuf[(chan->head + 7) & BUF_MASK];
        u8 num = chan->rxBuf[(chan->head + 8) & BUF_MASK];
        u8 buf[32];
        if( crc_1 ==  crc_2){
            uint8_t cmd = chan->rxBuf[ (chan->head + 5) & BUF_MASK];
            u8 parlen = 0;
            switch( cmd )
            {
            case CMD_1:
                MKBUS_msg_cmd1(chan);
                break;
            case CMD_2:
                MKBUS_msg_cmd2(chan, unit);
                break;
            case CMD_3:
                MKBUS_msg_cmd3(chan, unit, par);
                break;
            case CMD_4:
                MKBUS_msg_cmd4(chan, unit, par, num);
                break;
            case CMD_5:
                Func_prop[unit](par, 0, 0, &parlen );
                if(unit >= N_UNIT)break;
                if(par >= unitProp[unit][1])break;
                if(num > parlen) num = parlen;
                MemCopy(&chan->rxBuf[(chan->head + 9) & BUF_MASK], &buf,  num);
                Func_set[unit](par, buf);
                MKBUS_msg_cmd5(chan, unit, par);
                break;
            }
        }
    }
}

//=========================================================================

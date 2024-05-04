
#ifndef __mk_conf_tree_H
#define __mk_conf_tree_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "system.h"
#include "main.h"

#define BUF_SIZE  512    //

// ����� ��������
#define BUF_MASK (BUF_SIZE - 1)

// ��������� ���������
#define WAIT_HEAD   0
#define WAIT_DATA   1

#define CMD_1	1
#define CMD_2	2
#define CMD_3	3
#define CMD_4	4
#define CMD_5	5

#define CHAR  	0x00	// 1 � char
#define INT  	0x01	// 1 � int
#define UINT	0x02	// 2 � uint
#define BOOL  	0x03	// 3 � bool
#define REAL  	0x04	// 4 � float
#define HEX  	0x05	// 5 � hex
#define RES  	0x06	// 6 � reserve
#define TIME  	0x07	// 7 � time
#define RO 		0x10	// 8 � ������ ��� ������
#define CONST   0x20    // ��������� (����������� �� ����)
#define ERR     0x40    // �������� ��� ������
#define TRACE   0x80    // ����� �����������
#define MASK    0xF0    // ����� ��������
     
#pragma pack (1)

// ��������� ���������
typedef struct
{
    uint8_t rxBuf[BUF_SIZE];
    uint8_t pkt[BUF_SIZE];
    uint16_t state;
    uint16_t tail;
    uint16_t head;
    uint16_t rxLen;
    uint16_t rxSize;
    uint16_t txLen;
    uint32_t time;
} MKBUS;

#pragma pack (4)

extern MKBUS mkChan1;
extern DATA_32 dev_simb;


// ������� ��������� ������������
void MKBUS_send(uint8_t *buf, uint16_t ln);
uint16_t mkbus_CRC16( uint8_t * data, uint16_t tail, uint16_t size );
void MKBUS_rx(MKBUS *bus, uint8_t dt);

#ifdef __cplusplus
}
#endif
#endif /*__vkbus_H */


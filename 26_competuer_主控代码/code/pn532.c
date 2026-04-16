#include <string.h>
#include "headfile.h"
#include "PN532.h"
#ifdef PN532_PRINT_DEBUG
#include <stdio.h>
#endif

const uint8_t pn532_ack[] = {PN532_PREAMBLE, PN532_STARTCODE1, PN532_STARTCODE2, 0x00, 0xFF, PN532_POSTAMBLE};


static uint8_t PN532_send_buf[PN532_SEND_BUF_SIZE];
static uint8_t * PN532_send_buf_data = PN532_send_buf + 5;
static uint8_t PN532_recv_buf[PN532_RECV_BUF_SIZE];
uint8_t * get_PN532_recv_buf(void) { return PN532_recv_buf; }

#ifdef PN532_PRINT_DEBUG
void PN532_print_mem(uint8_t *mem, uint8_t cnt, char * str)
{

    char buf[128];
    char tmp[3];

    if (cnt) return;

    buf[0] = '\0';
    for (uint8_t i = 0; i < cnt; ++i) {
      sprintf(tmp, "%02X", mem[i]);
      strcat(buf, tmp);
    }

    printf("PN532: ");
    printf(str);
    printf(" ");
    printf(buf);
    printf("\r\n");
}
#endif


/**********************************************************************************************************************************/
uint8_t result;
uint8_t version[4];
uint8_t uid[10];
uint8_t uid_len = 0;  // 添加初始化
char msg[100];

// 定义多张授权卡片
typedef struct {
    uint8_t uid[10];
    uint8_t uid_len;
    char name[20];
    uint8_t access_level;  // 1=普通用户, 2=管理员
} Card_t;
Card_t authorized_cards[] = {
{{0x56, 0x15, 0x2B, 0x07}, 4, "Admin Card", 2},  // 正确的UID
{{0xCB, 0x8B, 0x0B, 0x07}, 4, "User Card 1", 1},
{{0x04, 0x29 ,0xC8 ,0x58 ,0xD1 ,0x2A ,0x81}, 7, "User Card 2", 5},
{{0x04 ,0x8E ,0x66 ,0x56 ,0xD1 ,0x2A ,0x81}, 7, "User Card 3", 6},
};
uint8_t num_cards = sizeof(authorized_cards) / sizeof(Card_t);
void PN532_INIT(void)
{

    
    // 3. 初始化PN532硬件
    PN532_Init_Hardware();
    // 4. 获取固件版本
    result = PN532_GetFirmVersion(version);
    if (result != PN532_ERROR_NONE)
    {
        while(1);
    }
    // 5. 配置SAM
    result = PN532_SetSamConfig(0x01, 0x14, 0x01);
    if (result != PN532_ERROR_NONE)
    {
        while(1);
    }
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    printf("pn532init sucessful\r\n");
}



char str2[100];
void PN532_connect(void)
{
        // 清空uid数组
        memset(uid, 0, sizeof(uid));
        uid_len = 0;
        
        result = PN532_ReadPassTarget(PN532_MIFARE_ISO14443A, uid, sizeof(uid), &uid_len);
        
        if (result == PN532_ERROR_NONE && uid_len > 0)
        {
            for (uint8_t i = 0; i < uid_len; i++)
            {
                printf("%02X ", uid[i]);

            }
            printf("\r\n");
            
            // 查找匹配的卡片
            int8_t card_index = -1;
            for (uint8_t i = 0; i < num_cards; i++)
            {
                if (uid_len == authorized_cards[i].uid_len &&
                    memcmp(uid, authorized_cards[i].uid, uid_len) == 0)
                {
                    card_index = i;
                    break;
                }
            }
            
            if (card_index >= 0)
            {
                // 找到授权卡片
                Card_t *card = &authorized_cards[card_index];
                
                // 根据权限执行不同操作
                if (card->access_level == 2)
                {
                    sprintf(str2, "va0.val=5");
                    tjc_send_string(str2);
                    R_BSP_SoftwareDelay(50, BSP_DELAY_UNITS_MILLISECONDS);
                    sprintf(str2, "t0.txt=\"%d\"",AS608_GetFRNumber());
                    tjc_send_string(str2);
                    

                }
                else if(card->access_level == 1)
                {
                    sprintf(str2, "va0.val=1");
                    tjc_send_string(str2);
                    uart2_send_value(1);
                    uart9_send_value(1);
                    dakai_clock_display();
                    motor_flag =1;
                }
                else if(card->access_level == 5)
                {
                    sprintf(str2, "va0.val=2");
                    tjc_send_string(str2);
                    uart2_send_value(2);
                    uart9_send_value(2);
                    dakai_clock_display();
                    motor_flag =2;
                }
                else if(card->access_level == 6)
                {
                    sprintf(str2, "va0.val=3");
                    tjc_send_string(str2);
                    uart2_send_value(3);
                    uart9_send_value(3);
                    dakai_clock_display();
                    motor_flag =3;
                }
            }
            else
            {
//                sprintf(str2, "va0.val=4");
//                tjc_send_string(str2);
                uart9_send_value(4);
                printf("卡片识别失败\r\n");
                R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
                
            }
            
            // 等待卡片移开
            printf(">>> 100ms  <<<\r\n\r\n");
            R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
        }
}











/**********************************************************************************************************************************/


//послать данные в PN532 с формироваеием фрейма
//send data to PN532 with framing
BOOL PN532_send_data(uint8_t *data, uint8_t data_len) {

    uint8_t i, checksum;

    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    PN532_send_buf[0] = PN532_PREAMBLE;
    PN532_send_buf[1] = PN532_PREAMBLE;
    PN532_send_buf[2] = PN532_STARTCODE2;
    PN532_send_buf[3] = data_len;
    PN532_send_buf[4] = ~data_len + 1;

    if (PN532_send_buf_data != data) memcpy(PN532_send_buf_data, data, data_len);

    for (i = 0; i < data_len; i++) checksum += data[i];

    PN532_send_buf[5 + data_len] = (~checksum) & 0xFF;
    PN532_send_buf[6 + data_len] = PN532_POSTAMBLE;

#ifdef PN532_PRINT_DEBUG
	PN532_print_mem(PN532_send_buf, 7 + data_len, "send");
#endif

    return PN532_TRANSMIT(PN532_send_buf, 7 + data_len, PN532_SEND_TIMEOUT);;
}


//дождаться байта PN532_I2C_READY
//wait for byte PN532_I2C_READY
BOOL PN532_WaitReady(uint32_t timeout) {
    uint8_t b;
    uint32_t tickstart = PN532_GET_TICK();

    while (PN532_TICK_DIFF(tickstart) < timeout) {
    	if (PN532_RECEIVE(&b, 1, timeout)) {
            if (b == PN532_I2C_READY) return TRUE;
    	}
    	PN532_DELAY(1);
    }
    return FALSE;
}



//получить данные из PN532
//get data from PN532
uint8_t PN532_recv(uint8_t * buf, uint8_t how)
{

	if (!PN532_WaitReady(PN532_RECV_READY_TIMEOUT)) return 0;

    if (PN532_RECEIVE(buf, how, PN532_RECV_DATA_TIMEOUT)) return how;

	return 0;
}


//получить ACK из PN532
//get ACK from PN532
BOOL PN532_recv_ack(void) {

	uint8_t cnt;

	memset(PN532_recv_buf, 0, PN532_RECV_BUF_SIZE);

	cnt = PN532_recv(PN532_recv_buf, sizeof(pn532_ack) + 1);
	if (cnt == 0) return FALSE;

	if (PN532_recv_buf[0] != PN532_I2C_READY) return 0;

#ifdef PN532_PRINT_DEBUG
	PN532_print_mem(PN532_recv_buf, cnt, "recv ack");
#endif

	return (0 == memcmp(PN532_recv_buf + 1, pn532_ack, sizeof(pn532_ack)));
}


//парсинг frame-а PN532
//parsing the PN532 frame
uint8_t * PN532_parse_frame(uint8_t * buf, uint8_t buf_size, uint8_t * data_len)
{
    uint8_t i, checksum, u8;

    if (buf[0] != PN532_PREAMBLE) return NULL;
    if (buf[1] != PN532_PREAMBLE) return NULL;
    if (buf[2] != PN532_STARTCODE2) return NULL;
    u8 = ~buf[3] + 1;
    if (buf[4] != u8) return NULL;

    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    for (i = 0; i < buf[3]; ++i)  checksum += buf[5 + i];
    u8 = (~checksum) & 0xFF;


    if (buf[5 + buf[3]] != u8) return NULL;


    *data_len = buf[3];
    return buf + 5;
}

//получить данные из PN532 и парсинг фрэйма
//get data from PN532 and parse frame
uint8_t * PN532_recv_data(uint8_t * buf, uint8_t buf_size, uint8_t * data_len)
{
	uint8_t cnt, * data = NULL, i;

	cnt = PN532_recv(buf, buf_size);

#ifdef PN532_PRINT_DEBUG
	PN532_print_mem(buf, cnt, "recv data");
#endif

	if (cnt < 8) return NULL;

	if (buf[0] != PN532_I2C_READY) return 0;

	//data = PN532_parse_frame(buf + 1, cnt - 1, data_len);

	for (i = 1 ; i < cnt - 6; ++i) {
		data = PN532_parse_frame(buf + i, cnt - i, data_len);
		if (data) break;
	}

	return data;
}


//--------------------------- MIFARE ----------------------------------------


BOOL PN532_SendGetFirmwareVersion(void)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_GETFIRMWAREVERSION;

	return PN532_send_data(PN532_send_buf_data, 2);
}

uint8_t PN532_GetFirmVersion(uint8_t * version)
{
	uint8_t * data, data_len;

	//PN532: send: 0000FF02FED4022A00
	if (!PN532_SendGetFirmwareVersion()) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	//PN532: recv data 01 00 00 FF 06 FA D5 03 32 01 06 07 E8 00
	data = PN532_recv_data(PN532_recv_buf, 14, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data_len < 6) return PN532_ERROR_FRAMING_LEN;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_GETFIRMWAREVERSION + 1)) return PN532_ERROR_RECV_COMMAND;

	memcpy(version, data + 2, 4);

	return PN532_ERROR_NONE;
}

BOOL PN532_SendSamConfig(uint8_t mode, uint8_t timeout, uint8_t use_irq)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_SAMCONFIGURATION;
	PN532_send_buf_data[2] = mode;
	PN532_send_buf_data[3] = timeout;
	PN532_send_buf_data[4] = use_irq;

	return PN532_send_data(PN532_send_buf_data, 5);
}

uint8_t PN532_SetSamConfig(uint8_t mode, uint8_t timeout, uint8_t use_irq)
{
	uint8_t * data, data_len;

	//0x01 -normal mode, 0x14 - timeout 50ms * 20 = 1 second,  0x01- use IRQ pin!
	//PN532: send: 00 00 FF 05 FB D4 14 01 14 01 02 00
	if (!PN532_SendSamConfig(mode, timeout, use_irq)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	//PN532: recv data 01 00 00 FF 02 FE D5 15 16 00 01 02 E8 00 (GOOD)
	//PN532: recv data 01 00 00 FF 01 FF 7F 81 00 (BAD - Error frame)
	data = PN532_recv_data(PN532_recv_buf, 14, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data_len < 2) return PN532_ERROR_FRAMING_LEN;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_SAMCONFIGURATION + 1)) return PN532_ERROR_RECV_COMMAND;

	return PN532_ERROR_NONE;
}


BOOL PN532_SendReadPassiveTarget(uint8_t card_baud)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_INLISTPASSIVETARGET;
	PN532_send_buf_data[2] = 1; // max 1 cards at once (we can set this to 2 later)
	PN532_send_buf_data[3] = card_baud;

	return PN532_send_data(PN532_send_buf_data, 4);
}
uint8_t PN532_ReadPassTarget(uint8_t card_baud, uint8_t * uid, uint8_t uid_size, uint8_t *uid_len)
{
    uint8_t * data, data_len;

    if (!PN532_SendReadPassiveTarget(card_baud)) return PN532_ERROR_SEND_DATA;
    if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

    data = PN532_recv_data(PN532_recv_buf, 14 + MIFARE_UID_MAX_LENGTH + 2, &data_len);
    if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

    if (data_len < 7) return PN532_ERROR_FRAMING_LEN;
    if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
    if (data[1] != (PN532_COMMAND_INLISTPASSIVETARGET + 1)) return PN532_ERROR_RECV_COMMAND;
    if (data[2] != 0x01) return PN532_ERROR_MORE_ONE_CARD;
    if (data_len < 8 + data[7]) return PN532_ERROR_FRAMING_LEN;

    // 计算要复制的UID长度
    *uid_len = (data[7] < uid_size) ? data[7] : uid_size;
    
    // 使用memcpy复制UID
    memcpy(uid, &data[8], *uid_len);

    return PN532_ERROR_NONE;
}

BOOL PN532_SendMifareClassicAuthBlock(uint8_t *uid, uint8_t uid_len,  uint8_t block_number,  uint8_t key_number, uint8_t * key)
{
	uint8_t i;

    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_INDATAEXCHANGE;
	PN532_send_buf_data[2] = 1;        // Max card numbers
	PN532_send_buf_data[3] = key_number; //MIFARE_CMD_AUTH_A or MIFARE_CMD_AUTH_B;
	PN532_send_buf_data[4] = block_number; // Block Number (1K = 0..63, 4K = 0..255
	memcpy(PN532_send_buf_data + 5, key, 6);
	for (i = 0; i < uid_len; i++) PN532_send_buf_data[11 + i] = uid[i]; /* 4 uint8_t card ID */

	return PN532_send_data(PN532_send_buf_data, 11 + uid_len);
}


uint8_t PN532_MifareClassicAuthBlock(uint8_t *uid, uint8_t uid_len,  uint8_t block_number,  uint8_t key_number, uint8_t * key)
{
	uint8_t * data, data_len;


	//PN532: send 00 00 FF 0F F1 D4 40 01 60 0A FF FF FF FF FF FF 5C BB E5 D5 B6 00
	//PN532: send 00 00 FF 0F F1 D4 40 01 60 1F FE E1 DE FE C8 ED 5C BB E5 D5 2B 00

	if (!PN532_SendMifareClassicAuthBlock(uid, uid_len, block_number, key_number, key)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	//PN532: recv 01 00 00 FF 03 FD D5 41 00 EA 00
	data = PN532_recv_data(PN532_recv_buf, 11, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_INDATAEXCHANGE + 1)) return PN532_ERROR_RECV_COMMAND;

	if (data_len < 3) return PN532_ERROR_FRAMING_LEN;

	return data[2];
}

BOOL PN532_SendReadDataBlock(uint8_t block_number)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_INDATAEXCHANGE;
	PN532_send_buf_data[2] = 1;               // Card number
	PN532_send_buf_data[3] = MIFARE_CMD_READ; // Mifare Read command = 0x30
	PN532_send_buf_data[4] = block_number; // Block Number (0..63 for 1K, 0..255 for 4K)

	return PN532_send_data(PN532_send_buf_data, 5);
}


uint8_t PN532_ReadDataBlock(uint8_t block_number, uint8_t * block_data)
{
	uint8_t * data, data_len;

	//PN532: send 00 00 FF 05 FB D4 40 01 30 0A B1 00
	if (!PN532_SendReadDataBlock(block_number)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	//PN532: recv data 01 00 00 FF 13 ED D5 41 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 EA 00
	//PN532: recv data 01 00 00 FF 13 ED D5 41 00 00 00 00 00 00 00 FF 07 80 69 FE E1 DE FE C8 ED 8B 00

	data = PN532_recv_data(PN532_recv_buf, 27, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_INDATAEXCHANGE + 1)) return PN532_ERROR_RECV_COMMAND;

	if (data_len < 19) return  PN532_ERROR_FRAMING_LEN;

	memcpy(block_data, data + 3, 16);

	return data[2];
}

BOOL PN532_SendWriteDataBlock(uint8_t block_number,  uint8_t * block_data)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_INDATAEXCHANGE;
	PN532_send_buf_data[2] = 1;                // Card number
	PN532_send_buf_data[3] = MIFARE_CMD_WRITE; // Mifare Write command = 0xA0
	PN532_send_buf_data[4] = block_number; // Block Number (0..63 for 1K, 0..255 for 4K)

	memcpy(PN532_send_buf_data + 5, block_data, 16); // Data Payload

	return PN532_send_data(PN532_send_buf_data, 21);
}

uint8_t PN532_WriteDataBlock(uint8_t block_number,  uint8_t * block_data)
{
	uint8_t * data, data_len;

	//PN532: send 00 00 FF 15 EB D4 40 01 A0 0A (ron b) FE E1 DE FE C8 ED (ron e) FF 07 80 69 (ron b) FE E1 DE FE C8 ED (ron e) 72 00
	if (!PN532_SendWriteDataBlock(block_number, block_data)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	//PN532: recv data 01 00 00 FF 03 FD D5 41 00 EA 00 E1 DE FE C8 ED FF 07 80 69 FE E1 DE FE C8 ED 72 00
	data = PN532_recv_data(PN532_recv_buf, 28, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data_len < 3) return PN532_ERROR_FRAMING_LEN;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_INDATAEXCHANGE + 1)) return PN532_ERROR_RECV_COMMAND;


	return data[2];
}

BOOL PN532_SendSetParameters(uint8_t params)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_SETPARAMETERS;
	PN532_send_buf_data[2] = params;

	return PN532_send_data(PN532_send_buf_data, 3);
}

uint8_t PN532_SetParameters(uint8_t params)
{
	uint8_t  * data, data_len;

	if (!PN532_SendSetParameters(params)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	data = PN532_recv_data(PN532_recv_buf, PN532_RECV_BUF_SIZE, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data_len < 2) return PN532_ERROR_FRAMING_LEN;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_SETPARAMETERS + 1)) return PN532_ERROR_RECV_COMMAND;

	return PN532_ERROR_NONE;
}

//--------------------------- NTAG ----------------------------------------

BOOL PN532_SendNtag2xxAuth(uint8_t * pwd)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_INCOMMUNICATETHRU;
	PN532_send_buf_data[2] = 0x1B;
	PN532_send_buf_data[3] = pwd[0];
	PN532_send_buf_data[4] = pwd[1];
	PN532_send_buf_data[5] = pwd[2];
	PN532_send_buf_data[6] = pwd[3];

	return PN532_send_data(PN532_send_buf_data, 7);
}


uint8_t PN532_Ntag2xxAuth(uint8_t *pwd, uint8_t* response) {
	uint8_t  * data, data_len, i;

	//PN532: send 00 00 FF 07 F9 D4 42 1B 52 84 00 08 F1 00

	if (!PN532_SendNtag2xxAuth(pwd)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	//PN532: recv 01 00 00 FF 03 FD D5 43 01 E7 00 00 08 F1 08 FC 70 30 05 78 80 71 00 5D 00
	//PN532: recv 01 00 00 FF 03 FD D5 43 01 E7 00 70 30 2B 08 FC 70 30 05 78 80 71 00 5D 00
	data = PN532_recv_data(PN532_recv_buf, PN532_RECV_BUF_SIZE, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data_len < 9) return PN532_ERROR_FRAMING_LEN;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_INCOMMUNICATETHRU + 1)) return PN532_ERROR_RECV_COMMAND;

	for (i = 0; i < 6; ++i) response[i] = data[3 + i];

	return data[2];
}


BOOL PN532_SendNtag2xxReadBlock(uint8_t block_number)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_INDATAEXCHANGE;
	PN532_send_buf_data[2] = 0x01;
	PN532_send_buf_data[3] = MIFARE_CMD_READ;
	PN532_send_buf_data[4] = block_number;

	return PN532_send_data(PN532_send_buf_data, 5);
}

uint8_t PN532_Ntag2xxReadBlock(uint8_t block_number, uint8_t * block_data)
{
	uint8_t  * data, data_len, i;

	//PN532: send 00 00 FF 05 FB D4 40 01 30 04 B7 00
	if (!PN532_SendNtag2xxReadBlock(block_number)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	//PN532: recv data 01 00 00 FF 05 FB D5 41 00 6E 00 7C 00
	data = PN532_recv_data(PN532_recv_buf, PN532_RECV_BUF_SIZE, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data_len < 3 + NTAG2XX_BLOCK_LENGTH) return PN532_ERROR_FRAMING_LEN;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_INDATAEXCHANGE + 1)) return PN532_ERROR_RECV_COMMAND;

    for (i = 0; i < NTAG2XX_BLOCK_LENGTH; i++)  block_data[i] = data[3 + 1];

	return data[2];
}


BOOL PN532_SendNtag2xxWriteBlock(uint8_t block_number, uint8_t * block_data)
{
    PN532_send_buf_data[0] = PN532_HOSTTOPN532;
	PN532_send_buf_data[1] = PN532_COMMAND_INDATAEXCHANGE;
	PN532_send_buf_data[2] = 0x01;
	PN532_send_buf_data[3] = MIFARE_ULTRALIGHT_CMD_WRITE;
	PN532_send_buf_data[4] = block_number;

    for (uint8_t i = 0; i < NTAG2XX_BLOCK_LENGTH; i++) PN532_send_buf_data[5 + i] = block_data[i];

	return PN532_send_data(PN532_send_buf_data, 9);
}


uint8_t PN532_Ntag2xxWriteBlock(uint8_t block_number, uint8_t * block_data)
{
	uint8_t  * data, data_len;

	if (!PN532_SendNtag2xxWriteBlock(block_number, block_data)) return PN532_ERROR_SEND_DATA;

	if (!PN532_recv_ack()) return PN532_ERROR_RECV_ACK;

	data = PN532_recv_data(PN532_recv_buf, PN532_RECV_BUF_SIZE, &data_len);
	if ( (data == NULL) || (data_len == 0) ) return PN532_ERROR_RECV_DATA;

	if (data_len < 3) return PN532_ERROR_FRAMING_LEN;

	if (data[0] != PN532_PN532TOHOST) return PN532_ERROR_RECV_TOHOST;
	if (data[1] != (PN532_COMMAND_INDATAEXCHANGE + 1)) return PN532_ERROR_RECV_COMMAND;

	return data[2];
}


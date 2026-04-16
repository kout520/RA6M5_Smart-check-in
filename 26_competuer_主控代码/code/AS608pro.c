#include "AS608pro.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "headfile.h"
#if AS608_START_STATE

#define U8(x)  ((uint8_t)(x))
#define U16(x) ((uint16_t)(x))
#define U32(x) ((uint32_t)(x))

//接收状态
//bit15，    接收完成标志
//bit14，    接收到0x0d
//bit13~0，    接收到的有效字节数目
// 全局变量
uint32_t AS608Addr = 0XFFFFFFFF;
uint16_t AS608_USART_RX_STA = 0;
uint8_t AS608_USART_RX_BUF[400];

char str1[100];
int aa;
bsp_io_level_t level = BSP_IO_LEVEL_LOW; 
static volatile uint8_t g_uart3_tx_complete = 0;
static volatile uint8_t g_uart3_rx_complete = 0;
static volatile uint32_t rx_timeout_counter = 0;
/*********************************TIM4********************************************/
void finger_addtion(void)//添加指纹
{
// 等待10秒，看用户是否要录入指纹
    bool add_mode = false;
    for(uint8_t i = 0; i < 5; i++)  // 10秒
    {
        if(PS_Sta)  // 检测到手指
        {
            add_mode = true;
            break;
        }
        printf("failed!\r\n");
        R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    }
    
    if(add_mode)
    {
        
        sprintf(str1, "t0.txt=\"确认中\"");
        tjc_send_string(str1);
        
        
        printf(">>> Starting fingerprint enrollment <<<\r\n\r\n");
        
        // 第一次按指纹
        printf("Step 1: Place your finger on sensor...\r\n");
        
        bool first_success = false;
        for(uint8_t i = 0; i < 10; i++)  // 最多等待10秒
        {
            uint8_t result = AS608_AddFR();
            
            if(result == 0x01)  // 录入成功
            {
                printf(">>> Fingerprint enrolled successfully! <<<\r\n\r\n");
                first_success = true;
                break;
            }
            else if(result == 0x00)  // 录入失败
            {
                printf(">>> Enrollment failed! Please try again. <<<\r\n\r\n");
                break;
            }
            // result == 0x02 表示没检测到手指，继续等待
            
            R_BSP_SoftwareDelay(300, BSP_DELAY_UNITS_MILLISECONDS);
        }
        
        if(!first_success)
        {
            printf(">>> Enrollment timeout or failed! <<<\r\n\r\n");
        }
        
        R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
    }
    else
    {
        printf(">>> Skipping enrollment, entering verification mode <<<\r\n\r\n");
    }

}



    //检索指纹
void finger_search(void)
{
    uint16_t matched_id = 0;
    uint8_t result1 = AS608_PressFR(&matched_id);
    
    switch(result1)
    {
        case 1:  // 验证成功
                printf(">>> Fingerprint MATCHED! ID=%d <<<\r\n\r\n", matched_id);
            // 根据匹配ID做不同操作
            if (matched_id == 1) {
            sprintf(str1, "va0.val=1");
            tjc_send_string(str1);
                uart2_send_value(1);
                uart9_send_value(1);
                motor_flag =1;
                    dakai_clock_display();
                // 执行管理员相关操作
            } else if (matched_id == 2) {
            sprintf(str1, "va0.val=2");
            tjc_send_string(str1);
                uart2_send_value(2);
                uart9_send_value(2);
                motor_flag =2;
                    dakai_clock_display();
                // 执行访客相关操作
            } else if (matched_id == 3) {
            sprintf(str1, "va0.val=3");
            tjc_send_string(str1);
                uart2_send_value(3);
                uart9_send_value(3);
                motor_flag =3;
                    dakai_clock_display();
                // 执行访客相关操作
            } else {
                printf("Hello User %d\r\n", matched_id);
            }
            break;
            
        case 2:  // 验证失败
//            sprintf(str1, "va0.val=4");
//            tjc_send_string(str1);
        uart9_send_value(4);
            printf("指纹识别失败\r\n");
        R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
            break;
            
        case 0:  // 手指未松开
        case 3:  // 无手指
        default:
            // 不输出，避免刷屏
            break;
    }

    R_BSP_SoftwareDelay(10, BSP_DELAY_UNITS_MILLISECONDS);
}


// 接收中断回调函数
void uart3_callback(uart_callback_args_t * p_args)
{
    level = !level; 
    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE: // 发送完毕
        {
            g_uart3_tx_complete = 1;
            break;
        }
        
        case UART_EVENT_RX_CHAR: // 单个字符接收中断（重要！）
        {
            // 获取数据
            uint8_t res = (uint8_t)p_args->data;
            
            if((AS608_USART_RX_STA & (1<<15)) == 0) {
                if(AS608_USART_RX_STA < 400) {
                    rx_timeout_counter = 100;  // 100ms超时
                    AS608_USART_RX_BUF[AS608_USART_RX_STA++] = res;
                } else {
                    AS608_USART_RX_STA |= 1 << 15;
                }
            }
            
            
            // 重要：重新启动接收，以便接收下一个字符
            // 如果不重新启动，只会接收一个字符就停止
            //R_SCI_UART_Receive(&g_uart5_ctrl, &rx_data, 1);
            
            break;
        }
        
        case UART_EVENT_RX_COMPLETE: // 块接收完成（DMA或接收指定数量后）
        {
            g_uart3_rx_complete = 1;
            // 这里可以处理批量数据接收完成
            break;
        }
        
        case UART_EVENT_ERR_PARITY:   // 奇偶校验错误
        case UART_EVENT_ERR_FRAMING:  // 帧错误
        {
            // 错误处理
            // 可以设置错误标志或进行其他处理
            break;
        }
        
        default:
        {
            break;
        }
    }
}




//1ms定时器回调（用于接收超时检测）
void timeout_search_1ms(void)
{
    if(rx_timeout_counter > 0) {
    rx_timeout_counter--;
    if(rx_timeout_counter == 0 && AS608_USART_RX_STA > 0) {
        AS608_USART_RX_STA |= 1 << 15;  // 标记接收完成
    }
}
}



// 初始化函数
void AS608_Init(void)
{
    fsp_err_t err3;
    //char msg5[100];
    
    // 打开UART5（AS608通信）
    err3 = g_uart3.p_api->open(g_uart3.p_ctrl, g_uart3.p_cfg);
    if(FSP_SUCCESS != err3)
    {
        return;
    }

    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
    
    AS608_Check();
}

// 发送一个字节
static void MYUSART_SendData(uint8_t data) {
    g_uart3_tx_complete = 0;
    g_uart3.p_api->write(g_uart3.p_ctrl, &data, 1);
    while(!g_uart3_tx_complete);  // 等待发送完成
}

//发送包头
void SendHead(void)
{
    MYUSART_SendData(0xEF);
    MYUSART_SendData(0x01);
}
//发送地址
static void SendAddr(void)
{
    MYUSART_SendData(AS608Addr>>24);
    MYUSART_SendData(AS608Addr>>16);
    MYUSART_SendData(AS608Addr>>8);
    MYUSART_SendData(AS608Addr);
}
//发送包标识,
static void SendFlag(uint8_t flag)
{
    MYUSART_SendData(flag);
}
//发送包长度
static void SendLength(int length)
{
    MYUSART_SendData(length>>8);
    MYUSART_SendData(length);
}
//发送指令码
static void Sendcmd(uint8_t cmd)
{
    MYUSART_SendData(cmd);
}
//发送校验和
static void SendCheck(uint16_t check)
{
    MYUSART_SendData(check>>8);
    MYUSART_SendData(check);
}
//判断中断接收的数组有没有应答包
//waittime为等待中断接收数据的时间（单位1ms）
//返回值：数据包首地址
static uint8_t *JudgeStr(uint16_t waittime)
{
    char *data;
    uint8_t str[8];
    str[0]=0xef;                    str[1]=0x01;
    str[2]=AS608Addr>>24;    str[3]=AS608Addr>>16;        
    str[4]=AS608Addr>>8;    str[5]=AS608Addr;                
    str[6]=0x07;                    str[7]='\0';
    AS608_USART_RX_STA=0;
    while(--waittime)
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        if(AS608_USART_RX_STA&0X8000)//接收到一次数据
        {
            AS608_USART_RX_STA=0;
            data=strstr((const char*)AS608_USART_RX_BUF,(const char*)str);
            if(data)
                return (uint8_t*)data;    
        }
    }
    return 0;
}
//录入图像 PS_GetImage
//功能:探测手指，探测到后录入指纹图像存于ImageBuffer。 
//模块返回确认字
uint8_t PS_GetImage(void)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x03);
    Sendcmd(0x01);
    temp =  0x01+0x03+0x01;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;
}
//生成特征 PS_GenChar
//功能:将ImageBuffer中的原始图像生成指纹特征文件存于CharBuffer1或CharBuffer2             
//参数:BufferID --> charBuffer1:0x01    charBuffer1:0x02                                                
//模块返回确认字
uint8_t PS_GenChar(uint8_t BufferID)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x04);
    Sendcmd(0x02);
    MYUSART_SendData(BufferID);
    temp = 0x01+0x04+0x02+BufferID;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;
}
//精确比对两枚指纹特征 PS_Match
//功能:精确比对CharBuffer1 与CharBuffer2 中的特征文件 
//模块返回确认字
uint8_t PS_Match(void)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x03);
    Sendcmd(0x03);
    temp = 0x01+0x03+0x03;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;
}
//搜索指纹 PS_Search
//功能:以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库.若搜索到，则返回页码。            
//参数:  BufferID @ref CharBuffer1    CharBuffer2
//说明:  模块返回确认字，页码（相配指纹模板）
uint8_t PS_Search(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,SearchResult *p)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x08);
    Sendcmd(0x04);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(StartPage>>8);
    MYUSART_SendData(StartPage);
    MYUSART_SendData(PageNum>>8);
    MYUSART_SendData(PageNum);
    temp = 0x01+0x08+0x04+BufferID
    +(StartPage>>8)+(uint8_t)StartPage
    +(PageNum>>8)+(uint8_t)PageNum;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
    {
        ensure = data[9];
        p->pageID   =(data[10]<<8)+data[11];
        p->mathscore=(data[12]<<8)+data[13];    
    }
    else
        ensure = 0xff;
    return ensure;    
}
//合并特征（生成模板）PS_RegModel
//功能:将CharBuffer1与CharBuffer2中的特征文件合并生成 模板,结果存于CharBuffer1与CharBuffer2    
//说明:  模块返回确认字
uint8_t PS_RegModel(void)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x03);
    Sendcmd(0x05);
    temp = 0x01+0x03+0x05;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;        
}
//储存模板 PS_StoreChar
//功能:将 CharBuffer1 或 CharBuffer2 中的模板文件存到 PageID 号flash数据库位置。            
//参数:  BufferID @ref charBuffer1:0x01    charBuffer1:0x02
//       PageID（指纹库位置号）
//说明:  模块返回确认字
uint8_t PS_StoreChar(uint8_t BufferID,uint16_t PageID)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x06);
    Sendcmd(0x06);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(PageID>>8);
    MYUSART_SendData(PageID);
    temp = 0x01+0x06+0x06+BufferID
    +(PageID>>8)+(uint8_t)PageID;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;    
}
//删除模板 PS_DeletChar
//功能:  删除flash数据库中指定ID号开始的N个指纹模板
//参数:  PageID(指纹库模板号)，N删除的模板个数。
//说明:  模块返回确认字
uint8_t PS_DeletChar(uint16_t PageID,uint16_t N)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x07);
    Sendcmd(0x0C);
    MYUSART_SendData(PageID>>8);
    MYUSART_SendData(PageID);
    MYUSART_SendData(N>>8);
    MYUSART_SendData(N);
    temp = 0x01+0x07+0x0C
    +(PageID>>8)+(uint8_t)PageID
    +(N>>8)+(uint8_t)N;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;
}
//清空指纹库 PS_Empty
//功能:  删除flash数据库中所有指纹模板
//参数:  无
//说明:  模块返回确认字
uint8_t PS_Empty(void)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x03);
    Sendcmd(0x0D);
    temp = 0x01+0x03+0x0D;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;
}
//写系统寄存器 PS_WriteReg
//功能:  写模块寄存器
//参数:  寄存器序号RegNum:4\5\6
//说明:  模块返回确认字
uint8_t PS_WriteReg(uint8_t RegNum,uint8_t DATA)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x05);
    Sendcmd(0x0E);
    MYUSART_SendData(RegNum);
    MYUSART_SendData(DATA);
    temp = RegNum+DATA+0x01+0x05+0x0E;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    if(ensure==0)
        //printf("\r\n设置参数成功！");
    //else
        //printf("\r\n%s",EnsureMessage(ensure));
    return ensure;
}
//读系统基本参数 PS_ReadSysPara
//功能:  读取模块的基本参数（波特率，包大小等)
//参数:  无
//说明:  模块返回确认字 + 基本参数（16bytes）
uint8_t PS_ReadSysPara(SysPara *p)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x03);
    Sendcmd(0x0F);
    temp = 0x01+0x03+0x0F;
    SendCheck(temp);
    data=JudgeStr(1000);
    if(data)
    {
        ensure = data[9];
        p->PS_max = (data[14]<<8)+data[15];
        p->PS_level = data[17];
        p->PS_addr=(data[18]<<24)+(data[19]<<16)+(data[20]<<8)+data[21];
        p->PS_size = data[23];
        p->PS_N = data[25];
    }        
    else
        ensure=0xff;
    if(ensure==0x00)
    {
//        printf("\r\n模块最大指纹容量=%d",p->PS_max);
//        printf("\r\n对比等级=%d",p->PS_level);
//        printf("\r\n地址=%x",p->PS_addr);
//        printf("\r\n波特率=%d",p->PS_N*9600);
    }
    //else 
            //printf("\r\n%s",EnsureMessage(ensure));
    return ensure;
}
//设置模块地址 PS_SetAddr
//功能:  设置模块地址
//参数:  PS_addr
//说明:  模块返回确认字
uint8_t PS_SetAddr(uint32_t PS_addr)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x07);
    Sendcmd(0x15);
    MYUSART_SendData(PS_addr>>24);
    MYUSART_SendData(PS_addr>>16);
    MYUSART_SendData(PS_addr>>8);
    MYUSART_SendData(PS_addr);
    temp = 0x01+0x07+0x15
    +(uint8_t)(PS_addr>>24)+(uint8_t)(PS_addr>>16)
    +(uint8_t)(PS_addr>>8) +(uint8_t)PS_addr;                
    SendCheck(temp);
    AS608Addr=PS_addr;//发送完指令，更换地址
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;    
        AS608Addr = PS_addr;
    if(ensure==0x00)
        //printf("\r\n设置地址成功！");
    //else
    //    printf("\r\n%s",EnsureMessage(ensure));
    return ensure;
    return ensure;
}
//功能： 模块内部为用户开辟了256bytes的FLASH空间用于存用户记事本,
//    该记事本逻辑上被分成 16 个页。
//参数:  NotePageNum(0~15),Byte32(要写入内容，32个字节)
//说明:  模块返回确认字
uint8_t PS_WriteNotepad(uint8_t NotePageNum,uint8_t *Byte32)
{
    uint16_t temp;
    uint8_t  ensure,i;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(36);
    Sendcmd(0x18);
    MYUSART_SendData(NotePageNum);
    for(i=0;i<32;i++)
     {
         MYUSART_SendData(Byte32[i]);
         temp += Byte32[i];
     }
    temp =0x01+36+0x18+NotePageNum+temp;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
        ensure=data[9];
    else
        ensure=0xff;
    return ensure;
}
//读记事PS_ReadNotepad
//功能：  读取FLASH用户区的128bytes数据
//参数:  NotePageNum(0~15)
//说明:  模块返回确认字+用户信息
uint8_t PS_ReadNotepad(uint8_t NotePageNum,uint8_t *Byte32)
{
    uint16_t temp;
    uint8_t  ensure,i;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x04);
    Sendcmd(0x19);
    MYUSART_SendData(NotePageNum);
    temp = 0x01+0x04+0x19+NotePageNum;
    SendCheck(temp);
    data=JudgeStr(2000);
    if(data)
    {
        ensure=data[9];
        for(i=0;i<32;i++)
        {
            Byte32[i]=data[10+i];
        }
    }
    else
        ensure=0xff;
    return ensure;
}
//高速搜索PS_HighSpeedSearch
//功能：以 CharBuffer1或CharBuffer2中的特征文件高速搜索整个或部分指纹库。
//          若搜索到，则返回页码,该指令对于的确存在于指纹库中 ，且登录时质量
//          很好的指纹，会很快给出搜索结果。
//参数:  BufferID， StartPage(起始页)，PageNum（页数）
//说明:  模块返回确认字+页码（相配指纹模板）
uint8_t PS_HighSpeedSearch(uint8_t BufferID,uint16_t StartPage,uint16_t PageNum,SearchResult *p)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x08);
    Sendcmd(0x1b);
    MYUSART_SendData(BufferID);
    MYUSART_SendData(StartPage>>8);
    MYUSART_SendData(StartPage);
    MYUSART_SendData(PageNum>>8);
    MYUSART_SendData(PageNum);
    temp = 0x01+0x08+0x1b+BufferID
    +(StartPage>>8)+(uint8_t)StartPage
    +(PageNum>>8)+(uint8_t)PageNum;
    SendCheck(temp);
    data=JudgeStr(2000);
     if(data)
    {
        ensure=data[9];
        p->pageID     =(data[10]<<8) +data[11];
        p->mathscore=(data[12]<<8) +data[13];
    }
    else
        ensure=0xff;
    return ensure;
}
//读有效模板个数 PS_ValidTempleteNum
//功能：读有效模板个数
//参数: 无
//说明: 模块返回确认字+有效模板个数ValidN
uint8_t PS_ValidTempleteNum(uint16_t *ValidN)
{
    uint16_t temp;
    uint8_t  ensure;
    uint8_t  *data;
    SendHead();
    SendAddr();
    SendFlag(0x01);//命令包标识
    SendLength(0x03);
    Sendcmd(0x1d);
    temp = 0x01+0x03+0x1d;
    SendCheck(temp);
    data=JudgeStr(200);
    if(data)
    {
        ensure=data[9];
        *ValidN = (data[10]<<8) +data[11];
    }        
    else
        ensure=0xff;
    
    if(ensure==0x00)
    {
        //printf("\r\n有效指纹个数=%d",(data[10]<<8)+data[11]);
    }
    //else
        //printf("\r\n%s",EnsureMessage(ensure));
    return ensure;
}
//与AS608握手 PS_HandShake
//参数: PS_Addr地址指针
//说明: 模块返新地址（正确地址）    
uint8_t PS_HandShake(uint32_t *PS_Addr)
{
    SendHead();
    SendAddr();
    MYUSART_SendData(0X01);
    MYUSART_SendData(0X00);
    MYUSART_SendData(0X00);    
    R_BSP_SoftwareDelay(200, BSP_DELAY_UNITS_MILLISECONDS);
    if(AS608_USART_RX_STA&0X8000)//接收到数据
    {        
        if(//判断是不是模块返回的应答包                
                AS608_USART_RX_BUF[0]==0XEF
                &&AS608_USART_RX_BUF[1]==0X01
                &&AS608_USART_RX_BUF[6]==0X07
            )
            {
                *PS_Addr=(AS608_USART_RX_BUF[2]<<24) + (AS608_USART_RX_BUF[3]<<16)
                                +(AS608_USART_RX_BUF[4]<<8) + (AS608_USART_RX_BUF[5]);
                AS608_USART_RX_STA=0;
                return 0;
            }
        AS608_USART_RX_STA=0;                    
    }
    return 1;        
}


SysPara AS608Para;
uint16_t ValidN;
void AS608_Check(void)
{
    uint8_t cnt = 0;
    volatile uint8_t ensure;
    
    while(PS_HandShake(&AS608Addr))  // 与AS608模块握手
    {
        cnt++;
        printf("Handshake retry %d...\r\n", cnt);
        if(cnt > 10) 
        {
            printf("AS608 handshake FAILED!\r\n");
            return;  // 握手失败，直接返回
        }
        R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
    }
    
//    sprintf(debug_msg, "AS608 connected! Addr=0x%08lX\r\n", AS608Addr);
//    g_uart7.p_api->write(g_uart7.p_ctrl, (uint8_t*)debug_msg, strlen(debug_msg));
    
    ensure = PS_ValidTempleteNum(&ValidN);  // 读库指纹个数
    if(ensure != 0x00)
    {
        printf("Read sys para failed! ");
    }
    else
    {
        printf( "Fingerprint count: %d\r\n", ValidN);
    }
    
    ensure = PS_ReadSysPara(&AS608Para);  // 读参数
    if(ensure != 0x00)
    {
        printf("Read sys para failed! ");
    }

}
/**
 * @brief   刷指纹函数 只会返回一次查询成功结果 直到下次按下查询
 * @param   none
 * @retval  0:上次查询指纹成功至今还未松开 1:指纹查询成功 2:指纹查询失败 3:没有检测到指纹 
 */
uint8_t AS608_PressFR(uint16_t *matched_id)
{
    static uint8_t press_state = 0;     /* 指纹按下状态 */
    if(PS_Sta && press_state == 0)                  /* 检测PS_Sta状态，高电平为有手指按下 */
    {
        SearchResult seach;     /* 存放指纹搜索结果 */
        uint8_t ensure;         /* 确认字 */
        ensure = PS_GetImage(); /* 获取图像 */
        if(ensure == 0x00)      /* 获取图像成功 */
        {
            ensure=PS_GenChar(CharBuffer2); /* 生成特征 */
            if(ensure == 0x00)  /* 生成特征成功 */
            {
                ensure=PS_HighSpeedSearch(CharBuffer2,0,AS608Para.PS_max,&seach);/* 搜索指纹 */
                if(ensure == 0x00)/* 搜索指纹成功 */
                {
                    //seach.pageID;   /* 页码ID */
                    //seach.mathscore;/* 匹配分数 */
                    if (matched_id) *matched_id = seach.pageID;  // 返回匹配ID
                    press_state = 1;    /* 成功扫描到指纹 - 下次扫描指纹：本次松开后 */
                    return 1;
                }
            }
        }
        return 2;
    }
    else if(press_state == 1 && PS_Sta)
    {
        /* 手指还没有松开 */
        return 0x00;
    }
    else
    {
        if(press_state == 1)
        {
            press_state = 0;    /* 可以扫描下一次指纹 */
        }
    }
    return 0x03;
}
/**
 * @brief   录入指纹
 * @param   none
 * @retval  0:录入指纹失败 1:录入指纹成功 2:没有检测到指纹
 */
uint8_t AS608_AddFR(void)
{
    uint8_t ensure;
    if(PS_Sta)
    {
        ensure=PS_GetImage();

        if(ensure==0x00) 
        {
            ensure=PS_GenChar(CharBuffer1);         /* 指纹特征1 */
            if(ensure==0x00)
            {
                /* 第二次按指纹前可以加入延时 这里没有添加 */
                if(PS_Sta)
                {
                    ensure=PS_GetImage();
                    if(ensure == 0x00)
                    {
                        ensure=PS_GenChar(CharBuffer2); /* 指纹特征2 */
                        if(ensure == 0x00)
                        {
                            ensure = PS_Match();        /* 匹配两次指纹是否相同 */
                            if(ensure == 0x00)
                            {
                                ensure=PS_RegModel();   /* 生成指纹模板 */
                                if(ensure==0x00) 
                                {
                                    PS_ValidTempleteNum(&ValidN);               /* 读库指纹个数 */
                                    ensure=PS_StoreChar(CharBuffer2,ValidN + 1);/* 储存模板 */
                                    if(ensure == 0x00)
                                    {
                                        return 0x01;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0x00;
    }
    return 0x02;
}
/**
 * @brief   删除指定的指纹ID或者清空指纹库
 * @param   delete_id   :   需要删除的指纹ID，0xFFFF删除所有指纹
 * @retval  1:删除成功 0:删除失败
 */
uint8_t AS608_DeleteFR(uint16_t delete_id)
{
    uint8_t ensure;
    if(delete_id==0xFFFF)
        ensure=PS_Empty();  /* 删除所有指纹 */
    else 
        ensure = PS_DeletChar(delete_id,1); /*  删除单个指纹 */
    if(ensure == 0x00)
    {
        return 1;
    }
    return 0;
}
/**
 * @brief   获取当前指纹库中指纹的个数
 * @param   none
 * @retval  返回当前指纹数量
 */
uint8_t AS608_GetFRNumber(void)
{
    PS_ValidTempleteNum(&ValidN);
    return ValidN;
}
#endif


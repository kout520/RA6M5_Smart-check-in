#include "headfile.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// 接收缓冲区配置
#define UART9_RX_BUFFER_SIZE    10
#define UART9_FRAME_START       '@'
#define UART9_FRAME_END         '\n'

// 全局变量
static volatile int g_uart9_tx_complete = 0;
static volatile int g_uart9_rx_complete = 0;

static uint8_t uart9_rx_buffer[UART9_RX_BUFFER_SIZE];
static volatile uint8_t uart9_rx_index = 0;
static volatile uint8_t uart9_frame_ready = 0;

static volatile int16_t g_received_value = 6;      // 接收到的数值
static volatile uint8_t g_data_valid = 0;          // 数据有效标志



void uart9_wait_for_tx(void)
{
    while (!g_uart9_tx_complete);
    g_uart9_tx_complete = 0;
}

void uart9_wait_for_rx(void)
{
    while (!g_uart9_rx_complete);
    g_uart9_rx_complete = 0;
}
/**
 * @brief 处理接收到的数据
 * @param value 接收到的数值
 */
void process_uart9_data(void)
{
    // 根据接收到的值执行不同操作
    switch (g_received_value) {
        case 0://zhangsan
//            sprintf(str3, "va0.val=1");
//            tjc_send_string(str3);
            printf("0\r\n");
            break;
            
        case 1://lisi
//            sprintf(str3, "va0.val=2");
//            tjc_send_string(str3);
            printf("1\r\n");
            break;
            
        case 2://wanwu
//            sprintf(str3, "va0.val=3");
//            tjc_send_string(str3);
            printf("2\r\n");
            break;
            
        case 3:
//            sprintf(str3, "va0.val=5");
//            tjc_send_string(str3);
            break;
            
        default:

            break;
    }
}

/**
 * @brief UART2中断回调函数
 */
void uart9_callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE:
        {
            g_uart9_tx_complete = 1;
            break;
        }
        
        case UART_EVENT_RX_CHAR:
        {
            uint8_t ch = (uint8_t)p_args->data;
            
            // 帧头检测
            if (ch == '@') {
                uart9_rx_index = 0;
                uart9_rx_buffer[uart9_rx_index++] = ch;
            }
            // 数据接收
            else if (uart9_rx_index > 0 && uart9_rx_index < UART9_RX_BUFFER_SIZE) {
                uart9_rx_buffer[uart9_rx_index++] = ch;
                
                // 帧尾检测
                if (ch == '\n' && uart9_rx_index >= 4) {
                    // 提取数字: @1\r\n -> "1"
                    char temp[8];
                    uint8_t len = uart9_rx_index - 3;  // 去掉@和\r\n
                    memcpy(temp, uart9_rx_buffer + 1, len);
                    temp[len] = '\0';
                    
                    g_received_value = atoi(temp);
                    uart9_rx_index = 0;
                }
            }
            break;
        }
        
        case UART_EVENT_ERR_PARITY:
        case UART_EVENT_ERR_FRAMING:
        {
//            uart2_rx_index = 0;
            break;
        }
        
        default:
            break;
    }
}


/**
 * @brief 发送数据到UART2
 * @param value 要发送的数值
 */
void uart9_send_value(int16_t value)
{
    char buffer[16];
    sprintf(buffer, "@%d\r\n", value);
    
    fsp_err_t err = R_SCI_UART_Write(&g_uart9_ctrl, (uint8_t*)buffer, strlen(buffer));
    if (FSP_SUCCESS == err) {
        uart9_wait_for_tx();
    }
}


/**
 * @brief 初始化函数
 */
void uart9_init(void)
{

    fsp_err_t err;
    err = g_uart9.p_api->open(g_uart9.p_ctrl, g_uart9.p_cfg);
    
}

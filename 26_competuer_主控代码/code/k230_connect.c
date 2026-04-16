#include "headfile.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
// 接收缓冲区配置
#define UART8_RX_BUFFER_SIZE    10
#define UART8_FRAME_START       '@'
#define UART8_FRAME_END         '\n'

// 全局变量
static volatile int g_uart8_tx_complete = 0;
static volatile int g_uart8_rx_complete = 0;

static uint8_t uart8_rx_buffer[UART8_RX_BUFFER_SIZE];
static volatile uint8_t uart8_rx_index = 0;


static volatile int16_t g_received_value;      // 接收到的数值
static volatile int16_t last_g_received_value;      // 接收到的数值



void uart8_wait_for_tx(void)
{
    while (!g_uart8_tx_complete);
    g_uart8_tx_complete = 0;
}

void uart8_wait_for_rx(void)
{
    while (!g_uart8_rx_complete);
    g_uart8_rx_complete = 0;
}

char str3[100];
/**
 * @brief 处理接收到的数据
 * @param value 接收到的数值
 */
void process_uart8_data(void)
{
    // 根据接收到的值执行不同操作
    
    if(g_received_value == last_g_received_value)return;
    
    switch (g_received_value) {
        case 0://zhangsan
            break;
             
        case 1://lisi
            sprintf(str3, "va0.val=11");
            tjc_send_string(str3);
        uart2_send_value(4);
        uart9_send_value(1);
            dakai_clock_display();
            motor_flag =1;
            break;
            
        case 2://wanwu
            sprintf(str3, "va0.val=22");
            tjc_send_string(str3);
        uart2_send_value(5);
        uart9_send_value(2);
            dakai_clock_display();
            motor_flag =2;
            break;
            
        case 3:
            sprintf(str3, "va0.val=33");
            tjc_send_string(str3);
        uart2_send_value(6);
        uart9_send_value(3);
            dakai_clock_display();
            motor_flag =3;
            break;
            
        default:

            break;
    }
    last_g_received_value = g_received_value;
}

/**
 * @brief UART8中断回调函数
 */
void uart8_callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE:
        {
            g_uart8_tx_complete = 1;
            break;
        }
        
        case UART_EVENT_RX_CHAR:
        {
            uint8_t ch = (uint8_t)p_args->data;
            
            // 帧头检测
            if (ch == '@') {
                uart8_rx_index = 0;
                uart8_rx_buffer[uart8_rx_index++] = ch;
            }
            // 数据接收
            else if (uart8_rx_index > 0 && uart8_rx_index < UART8_RX_BUFFER_SIZE) {
                uart8_rx_buffer[uart8_rx_index++] = ch;
                
                // 帧尾检测
                if (ch == '\n' && uart8_rx_index >= 4) {
                    // 提取数字: @1\r\n -> "1"
                    char temp[8];
                    uint8_t len = uart8_rx_index - 3;  // 去掉@和\r\n
                    memcpy(temp, uart8_rx_buffer + 1, len);
                    temp[len] = '\0';
                    
                    g_received_value = atoi(temp);
                    uart8_rx_index = 0;
                }
            }
            break;
        }
        
        case UART_EVENT_ERR_PARITY:
        case UART_EVENT_ERR_FRAMING:
        {
            uart8_rx_index = 0;
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
void uart8_send_value(int16_t value)
{
    char buffer[16];
    sprintf(buffer, "@%d\r\n", value);
    
    fsp_err_t err = R_SCI_UART_Write(&g_uart8_ctrl, (uint8_t*)buffer, strlen(buffer));
    if (FSP_SUCCESS == err) {
        uart8_wait_for_tx();
    }
}

/**
 * @brief 主循环中调用的处理函数
 */
void uart8_process(void)
{
    
    process_uart8_data();

}

/**
 * @brief 初始化函数
 */
void uart8_init(void)
{
    // 清空接收缓冲区
    memset(uart8_rx_buffer, 0, UART8_RX_BUFFER_SIZE);
    uart8_rx_index = 0;
    g_received_value = 0;
    
    // 打开UART8
    fsp_err_t err;
    err = g_uart8.p_api->open(g_uart8.p_ctrl, g_uart8.p_cfg);
    
}

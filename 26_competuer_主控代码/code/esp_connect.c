#include "headfile.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// 接收缓冲区配置
#define UART2_RX_BUFFER_SIZE    64
#define UART2_FRAME_START       '@'
#define UART2_FRAME_END         '\n'

// 全局变量
static volatile int g_uart2_tx_complete = 0;
static volatile int g_uart2_rx_complete = 0;

static uint8_t uart2_rx_buffer[UART2_RX_BUFFER_SIZE];
static volatile uint8_t uart2_rx_index = 0;
static volatile uint8_t uart2_frame_ready = 0;

static volatile int16_t g_received_value = 0;      // ? 改为0，避免误触发
static volatile uint8_t g_data_valid = 0;          // 数据有效标志

// 时间相关
static char g_received_time[16];
static volatile uint8_t g_time_valid = 0;          // ? 新增：时间数据有效标志

void uart2_wait_for_tx(void)
{
    while (!g_uart2_tx_complete);
    g_uart2_tx_complete = 0;
}

void uart2_wait_for_rx(void)
{
    while (!g_uart2_rx_complete);
    g_uart2_rx_complete = 0;
}

// 全局变量
static volatile uint32_t g_timer_count = 0;  // 1ms计数器
static volatile uint8_t g_clock_running = 0;
char str5[30];
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} simple_time_t;

static volatile simple_time_t g_current_time = {0, 0, 0};

/**
 * @brief 1ms定时器中断
 */
void timer_callback(void)
{
    if (!g_clock_running) return;
    
    g_timer_count++;
    
    // 每1000ms更新一次秒
    if (g_timer_count >= 1000) {
        g_timer_count = 0;
        g_current_time.second++;
        
        if (g_current_time.second >= 60) {
            g_current_time.second = 0;
            g_current_time.minute++;
            
            if (g_current_time.minute >= 60) {
                g_current_time.minute = 0;
                g_current_time.hour++;
                
                if (g_current_time.hour >= 24) {
                    g_current_time.hour = 0;
                }
            }
        }
    }
}

/**
 * @brief 处理UART2数据
 */
void process_uart2_data(void)
{
    if (g_time_valid) {
        printf("收到时间: %s\r\n", g_received_time);
        
        // 解析时间
        int h, m, s;
        if (sscanf(g_received_time, "%d:%d:%d", &h, &m, &s) == 3) {
            g_current_time.hour = (uint8_t)h;
            g_current_time.minute = (uint8_t)m;
            g_current_time.second = (uint8_t)s;
            g_timer_count = 0;
            g_clock_running = 1;
            
            // 显示初始时间
            sprintf(str5, "t9.txt=\"%02d:%02d:%02d\"", 
                    g_current_time.hour, 
                    g_current_time.minute, 
                    g_current_time.second);
            tjc_send_string(str5);
        }
        
        g_time_valid = 0;
    }
    
    // 数值命令处理
    if (g_received_value != 0) {
        switch (g_received_value) {
            case 1:
                printf("连接失败\r\n");
                sprintf(str5, "va0.val=8");
                tjc_send_string(str5);
                break;
            case 2:
                printf("连接成功\r\n");
                sprintf(str5, "va0.val=7");
                tjc_send_string(str5);
                break;
        }
        g_received_value = 0;
    }
}

/**
 * @brief 主循环中更新显示
 */
void update_clock_display(void)
{
    if (!g_clock_running) return;
    
    static uint8_t last_second = 0xFF;
    
    if (g_current_time.second != last_second) {
        last_second = g_current_time.second;
        
        sprintf(str5, "t9.txt=\"%02d:%02d:%02d\"", 
                g_current_time.hour, 
                g_current_time.minute, 
                g_current_time.second);
        tjc_send_string(str5);
    }
}
/**
 * @brief 打卡时间更新显示
 */
void dakai_clock_display(void)
{
    R_BSP_SoftwareDelay(100, BSP_DELAY_UNITS_MILLISECONDS);
        sprintf(str5, "t8.txt=\"%02d:%02d:%02d\"", 
                g_current_time.hour, 
                g_current_time.minute, 
                g_current_time.second);
        tjc_send_string(str5);
}


/**
 * @brief UART2中断回调函数
 */
void uart2_callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE:
        {
            g_uart2_tx_complete = 1;
            break;
        }
        
        case UART_EVENT_RX_CHAR:
        {
            uint8_t ch = (uint8_t)p_args->data;
            
            if (ch == '@') {
                uart2_rx_index = 0;
                uart2_rx_buffer[uart2_rx_index++] = ch;
            }
            else if (uart2_rx_index > 0 && uart2_rx_index < UART2_RX_BUFFER_SIZE) {
                uart2_rx_buffer[uart2_rx_index++] = ch;
                
                if (ch == '\n' && uart2_rx_index >= 4) {
                    uart2_rx_buffer[uart2_rx_index] = '\0';
                    
                    // ? 判断是时间格式还是数字
                    if (strchr((char*)uart2_rx_buffer + 1, ':') != NULL) {
                        // 时间格式: @16:20:21\n
                        // 去掉 @ 符号，只保留时间部分
                        strcpy(g_received_time, (char*)uart2_rx_buffer + 1);
                        
                        // 去掉末尾的 \n（如果有）
                        size_t len = strlen(g_received_time);
                        if (len > 0 && g_received_time[len - 1] == '\n') {
                            g_received_time[len - 1] = '\0';
                        }
                        
                        g_time_valid = 1;  // ? 设置时间有效标志
                        
                    } 
                    else {
                        // 数字格式: @1\n
                        char temp[8];
                        uint8_t len = uart2_rx_index - 2;  // 去掉 @ 和 \n
                        memcpy(temp, uart2_rx_buffer + 1, len);
                        temp[len] = '\0';
                        g_received_value = atoi(temp);
                        
                    }
                    
                    uart2_rx_index = 0;
                }
            }
            break;
        }
        
        case UART_EVENT_ERR_PARITY:
        case UART_EVENT_ERR_FRAMING:
        {
            uart2_rx_index = 0;
            break;
        }
        
        default:
            break;
    }
}

void uart2_send_value(int16_t value)
{
    char buffer[16];
    sprintf(buffer, "@%d\r\n", value);
    uart2_send_string(buffer);
}

void send_wifi_to_esp32(const char* ssid, const char* pwd)
{
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "@WIFI:%s,%s\r\n", ssid, pwd);
    uart2_send_string(buffer);
    printf("发送到ESP32: %s", buffer);
}

void uart2_send_string(const char* str)
{
    if (str == NULL) return;
    
    fsp_err_t err = R_SCI_UART_Write(&g_uart2_ctrl, (uint8_t*)str, strlen(str));
    if (FSP_SUCCESS == err) {
        uart2_wait_for_tx();
    }
}

void uart2_init(void)
{
    memset(uart2_rx_buffer, 0, UART2_RX_BUFFER_SIZE);
    uart2_rx_index = 0;
    uart2_frame_ready = 0;
    g_data_valid = 0;
    g_received_value = 0;
    g_time_valid = 0;  // ? 初始化时间标志
    memset(g_received_time, 0, sizeof(g_received_time));
    
    fsp_err_t err;
    err = g_uart2.p_api->open(g_uart2.p_ctrl, g_uart2.p_cfg);
}

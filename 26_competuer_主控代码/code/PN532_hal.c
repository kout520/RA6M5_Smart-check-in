// PN532_hal.c - 修复UART输出问题

#include "PN532.h"
#include "hal_data.h"
#include <stdio.h>

extern const i2c_master_instance_t g_i2c1;
extern const uart_instance_t g_uart7;

static volatile uint8_t g_i2c_tx_complete = 0;
static volatile uint8_t g_i2c_rx_complete = 0;
static volatile uint32_t g_system_tick_ms = 0;
static volatile fsp_err_t g_i2c_last_error = FSP_SUCCESS;

// 添加安全的UART打印函数
static void debug_print(const char *msg)
{
    uint16_t len = strlen(msg);
    g_uart7.p_api->write(g_uart7.p_ctrl, (uint8_t*)msg, len);
    // 等待发送完成 (简单延时，根据波特率调整)
    R_BSP_SoftwareDelay(len / 10 + 2, BSP_DELAY_UNITS_MILLISECONDS);
}

void i2c_master_callback(i2c_master_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case I2C_MASTER_EVENT_TX_COMPLETE:
            g_i2c_tx_complete = 1;
            g_i2c_last_error = FSP_SUCCESS;
            break;
            
        case I2C_MASTER_EVENT_RX_COMPLETE:
            g_i2c_rx_complete = 1;
            g_i2c_last_error = FSP_SUCCESS;
            break;
            
        case I2C_MASTER_EVENT_ABORTED:
            g_i2c_tx_complete = 1;
            g_i2c_rx_complete = 1;
            g_i2c_last_error = FSP_ERR_ABORTED;
            break;
            
        default:
            break;
    }
}

void g_system_tick(void)
{

        g_system_tick_ms++;
    
}

uint32_t PN532_GetTick(void)
{
    return g_system_tick_ms;
}

BOOL PN532_I2C_Transmit(uint8_t *data, uint16_t len, uint32_t timeout)
{
    fsp_err_t err;
    uint32_t start_tick = PN532_GetTick();
    char msg[128];
    
    g_i2c_tx_complete = 0;
    g_i2c_last_error = FSP_SUCCESS;
    
//    sprintf(msg, "[TX] Addr:0x%02X Len:%d\r\n", PN532_I2C_ADDRESS, len);
//    debug_print(msg);
    
    // 设置从机地址
    err = g_i2c1.p_api->slaveAddressSet(g_i2c1.p_ctrl, PN532_I2C_ADDRESS, I2C_MASTER_ADDR_MODE_7BIT);
    if (FSP_SUCCESS != err)
    {
//        sprintf(msg, "[TX] SetAddr ERR:0x%X\r\n", err);
//        debug_print(msg);
        return FALSE;
    }
    
    // 打印发送数据
//    debug_print("[TX] Data: ");
//    for (uint16_t i = 0; i < len && i < 20; i++)
//    {
//        sprintf(msg, "%02X ", data[i]);
//        debug_print(msg);
//    }
//    debug_print("\r\n");
    
    // 发送数据
    err = g_i2c1.p_api->write(g_i2c1.p_ctrl, data, len, false);
    if (FSP_SUCCESS != err)
    {
//        sprintf(msg, "[TX] Write ERR:0x%X\r\n", err);
//        debug_print(msg);
        return FALSE;
    }
    
    // 等待发送完成
    while (!g_i2c_tx_complete)
    {
        if (PN532_TICK_DIFF(start_tick) > timeout)
        {
//            sprintf(msg, "[TX] Timeout %lums\r\n", timeout);
//            debug_print(msg);
            return FALSE;
        }
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    }
    
    if (g_i2c_last_error != FSP_SUCCESS)
    {
//        sprintf(msg, "[TX] Xfer ERR:0x%X\r\n", g_i2c_last_error);
//        debug_print(msg);
        return FALSE;
    }
    
    return TRUE;
}

BOOL PN532_I2C_Receive(uint8_t *data, uint16_t len, uint32_t timeout)
{
    fsp_err_t err;
    uint32_t start_tick = PN532_GetTick();
    char msg[128];
    
    g_i2c_rx_complete = 0;
    g_i2c_last_error = FSP_SUCCESS;
    
//    sprintf(msg, "[RX] Addr:0x%02X Len:%d\r\n", PN532_I2C_ADDRESS, len);
//    debug_print(msg);
    
    // 设置从机地址
    err = g_i2c1.p_api->slaveAddressSet(g_i2c1.p_ctrl, PN532_I2C_ADDRESS, I2C_MASTER_ADDR_MODE_7BIT);
    if (FSP_SUCCESS != err)
    {
//        sprintf(msg, "[RX] SetAddr ERR:0x%X\r\n", err);
//        debug_print(msg);
        return FALSE;
    }
    
    // 接收数据
    err = g_i2c1.p_api->read(g_i2c1.p_ctrl, data, len, false);
    if (FSP_SUCCESS != err)
    {
//        sprintf(msg, "[RX] Read ERR:0x%X\r\n", err);
//        debug_print(msg);
        return FALSE;
    }
    
    // 等待接收完成
    while (!g_i2c_rx_complete)
    {
        if (PN532_TICK_DIFF(start_tick) > timeout)
        {
//            sprintf(msg, "[RX] Timeout %lums\r\n", timeout);
//            debug_print(msg);
            return FALSE;
        }
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
    }
    
    if (g_i2c_last_error != FSP_SUCCESS)
    {
//        sprintf(msg, "[RX] Xfer ERR:0x%X\r\n", g_i2c_last_error);
//        debug_print(msg);
        return FALSE;
    }
    
    // 打印接收数据
//    printf("[RX] Data: ");
//    for (uint16_t i = 0; i < len && i < 20; i++)
//    {
//        
//        printf("%02X ", data[i]);
//    }
//    printf("\r\n");
    
    
    // 添加短暂延时，让PN532准备下一次通信
    R_BSP_SoftwareDelay(2, BSP_DELAY_UNITS_MILLISECONDS);
    
    return TRUE;
}


void PN532_Init_Hardware(void)
{
    fsp_err_t err;
    // 打开I2C
    err = g_i2c1.p_api->open(g_i2c1.p_ctrl, g_i2c1.p_cfg);
    if (FSP_SUCCESS != err)
    {
        while(1);
    }
    
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);

}

/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_gpt.h"
#include "r_timer_api.h"
#include "r_sci_uart.h"
            #include "r_uart_api.h"
#include "r_iic_master.h"
#include "r_i2c_master_api.h"
FSP_HEADER
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer2;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer2_ctrl;
extern const timer_cfg_t g_timer2_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer1;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer1_ctrl;
extern const timer_cfg_t g_timer1_cfg;

#ifndef NULL
void NULL(timer_callback_args_t * p_args);
#endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart9;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_uart_instance_ctrl_t     g_uart9_ctrl;
            extern const uart_cfg_t g_uart9_cfg;
            extern const sci_uart_extended_cfg_t g_uart9_cfg_extend;

            #ifndef uart9_callback
            void uart9_callback(uart_callback_args_t * p_args);
            #endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart8;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_uart_instance_ctrl_t     g_uart8_ctrl;
            extern const uart_cfg_t g_uart8_cfg;
            extern const sci_uart_extended_cfg_t g_uart8_cfg_extend;

            #ifndef uart8_callback
            void uart8_callback(uart_callback_args_t * p_args);
            #endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart2;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_uart_instance_ctrl_t     g_uart2_ctrl;
            extern const uart_cfg_t g_uart2_cfg;
            extern const sci_uart_extended_cfg_t g_uart2_cfg_extend;

            #ifndef uart2_callback
            void uart2_callback(uart_callback_args_t * p_args);
            #endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart7;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_uart_instance_ctrl_t     g_uart7_ctrl;
            extern const uart_cfg_t g_uart7_cfg;
            extern const sci_uart_extended_cfg_t g_uart7_cfg_extend;

            #ifndef uart7_callback
            void uart7_callback(uart_callback_args_t * p_args);
            #endif
/** Timer on GPT Instance. */
extern const timer_instance_t g_timer_1ms;

/** Access the GPT instance using these structures when calling API functions directly (::p_api is not used). */
extern gpt_instance_ctrl_t g_timer_1ms_ctrl;
extern const timer_cfg_t g_timer_1ms_cfg;

#ifndef timer_1ms_callback
void timer_1ms_callback(timer_callback_args_t * p_args);
#endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart3;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_uart_instance_ctrl_t     g_uart3_ctrl;
            extern const uart_cfg_t g_uart3_cfg;
            extern const sci_uart_extended_cfg_t g_uart3_cfg_extend;

            #ifndef uart3_callback
            void uart3_callback(uart_callback_args_t * p_args);
            #endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart5;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_uart_instance_ctrl_t     g_uart5_ctrl;
            extern const uart_cfg_t g_uart5_cfg;
            extern const sci_uart_extended_cfg_t g_uart5_cfg_extend;

            #ifndef uart5_callback
            void uart5_callback(uart_callback_args_t * p_args);
            #endif
/* I2C Master on IIC Instance. */
extern const i2c_master_instance_t g_i2c1;

/** Access the I2C Master instance using these structures when calling API functions directly (::p_api is not used). */
extern iic_master_instance_ctrl_t g_i2c1_ctrl;
extern const i2c_master_cfg_t g_i2c1_cfg;

#ifndef i2c_master_callback
void i2c_master_callback(i2c_master_callback_args_t * p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */

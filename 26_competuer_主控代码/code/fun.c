#include "headfile.h"



/*******************************************************************************************/
static volatile int g_uart7_tx_complete = 0;
static volatile int g_uart7_rx_complete = 0;
uint32_t count;

uint8_t dakai_flag=0;

// 简单的调试输出
void print_msg(const char *msg)
{
    g_uart7.p_api->write(g_uart7.p_ctrl, (uint8_t*)msg, strlen(msg));
}



//1ms定时器回调（用于接收超时检测）
void timer_1ms_callback(timer_callback_args_t *p_args)
{
    if (TIMER_EVENT_CYCLE_END == p_args->event)
    {
        timeout_search_1ms();
        g_system_tick();
        //时间流动
        timer_callback();
        if(lock_flag && a == 30)
        {
            R_IOPORT_PinWrite(g_ioport.p_ctrl, BSP_IO_PORT_04_PIN_11, BSP_IO_LEVEL_LOW);
            lock_flag = 0;
        }   
        if(motor_flag !=0)
        {
            count++;
            if(count >=700)
            {
                count=0;
                motor_flag = 0;
            }
        }
    }
}

void uart7_callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE: //发送完毕 
        {
            g_uart7_tx_complete  = 1;
            break;
        }
        case UART_EVENT_RX_COMPLETE: // 使用read函数启动接收后，接收完毕 
        {
            g_uart7_rx_complete = 1;
            break;
        }
        case UART_EVENT_RX_CHAR:    //未调用read函数接收到了数据
        {
            g_rx_buf.put(&g_rx_buf, (uint8_t)p_args->data);
            break;
        }
        default:
        {
            break;
        }
    }
}

void uart7_wait_for_tx(void)
{
    while (!g_uart7_tx_complete);
    g_uart7_tx_complete = 0;
}

void uart7_wait_for_rx(void)
{
    while (!g_uart7_rx_complete);
    g_uart7_rx_complete = 0;
}

//按键相关
typedef enum {
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESSED,
    KEY_STATE_LONG_PRESS
} key_state_t;

bsp_io_level_t level_2 = BSP_IO_LEVEL_HIGH;
key_state_t key_state = KEY_STATE_IDLE;
uint16_t key_press_time = 0;

#define KEY_DEBOUNCE_TIME   3      // 消抖时间（扫描次数）
#define KEY_LONG_PRESS_TIME 50     // 长按时间（扫描次数，如10ms扫描一次，50次=500ms）

void key_scan(void)
{
    static uint8_t debounce_cnt = 0;
    static bsp_io_level_t stable_level = BSP_IO_LEVEL_HIGH;  // 初始化为高电平（未按下）
    
    g_ioport.p_api->pinRead(&g_ioport_ctrl, BSP_IO_PORT_00_PIN_00, &level_2);
    
    // 消抖处理
    if(level_2 == stable_level)
    {
        debounce_cnt = 0;
    }
    else
    {
        debounce_cnt++;
        if(debounce_cnt >= KEY_DEBOUNCE_TIME)
        {
            debounce_cnt = 0;
            stable_level = level_2;
        }
        else
        {
            return;  // 还在抖动中，直接返回
        }
    }
    
    // 状态机处理（低电平有效）
    switch(key_state)
    {
        case KEY_STATE_IDLE:
            if(stable_level == BSP_IO_LEVEL_LOW)  // 检测到低电平 = 按键按下
            {
                key_state = KEY_STATE_PRESSED;
                key_press_time = 0;
            }
            break;
            
        case KEY_STATE_PRESSED:
            if(stable_level == BSP_IO_LEVEL_HIGH)  // 检测到高电平 = 按键释放
            {
                // 短按释放
                key_state = KEY_STATE_IDLE;
                printf("Short press detected!\r\n");
                // 在这里处理短按事件
                //录入一个指纹
                
                
                
            }
            else  // 仍然是低电平 = 按键持续按下
            {
                key_press_time++;
                if(key_press_time >= KEY_LONG_PRESS_TIME)
                {
                    key_state = KEY_STATE_LONG_PRESS;
                    printf("Long press detected!\r\n");
                    // 在这里处理长按事件
                    // 例如：清空指纹库
                }
            }
            break;
            
        case KEY_STATE_LONG_PRESS:
            if(stable_level == BSP_IO_LEVEL_HIGH)  // 检测到高电平 = 长按后释放
            {
                key_state = KEY_STATE_IDLE;
                // 长按释放后的处理（如果需要）
            }
            // 如果还是低电平，保持长按状态，不做处理
            break;
            
        default:
            key_state = KEY_STATE_IDLE;
            break;
    }
}















/*******************************************************************************************/

static int32_t circlebuf_put(struct circle_buf *pcb, uint8_t v)
{
    uint32_t next_w;

    /* 计算"下一个写位置", 如果越界就要设置为0 */
    next_w = pcb->w + 1;
    if (next_w == pcb->max_len)
        next_w = 0;
    
    /* 未满? */
    if (next_w != pcb->r)
    {
        /* 写入数据 */
        pcb->buffer[pcb->w] = v;

        /* 设置下一个写位置 */
        pcb->w = next_w;
        return 0;
    }
    return -1;
}

static int32_t circlebuf_get(struct circle_buf *pcb, uint8_t *pv)
{
    /* 不空? */
    if (pcb->w != pcb->r)
    {
        /* 读出数据 */
        *pv = pcb->buffer[pcb->r];
        
        /* 计算"下一个读位置", 如果越界就要设置为0 */
        pcb->r++;
        if (pcb->r == pcb->max_len)
            pcb->r = 0;
        
        return 0;
    }
    return -1;
}


static uint8_t rx_buf[64];
circle_buf_t g_rx_buf;

void circlebuf_init(void)
{
    g_rx_buf.r = g_rx_buf.w = 0;
    g_rx_buf.buffer  = rx_buf;
    g_rx_buf.max_len = sizeof(rx_buf);
    g_rx_buf.get = circlebuf_get;
    g_rx_buf.put = circlebuf_put;
}

/* 重写这个函数,重定向printf */
int fputc(int ch, FILE * f)
{
     (void)f;

     /* 启动发送字符 */
     g_uart7.p_api->write(g_uart7.p_ctrl, (uint8_t const * const)&ch, 1);

     /* 等待发送完毕 */
     uart7_wait_for_tx();

     return ch;
}


/* 重写这个函数,重定向scanf */
int fgetc(FILE *f)
{
	uint8_t ch;

    (void)f;
	while (g_rx_buf.get(&g_rx_buf, &ch)){};

    /* 回显 */
    {
        fputc(ch, NULL);

        /* 回车之后加换行 */
        if (ch == '\r')
        {
            fputc('\n', NULL);;
        }
    }
    
    return (int)ch;
}









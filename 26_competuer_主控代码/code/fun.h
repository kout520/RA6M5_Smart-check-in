#ifndef FUN_H_
#define FUN_H_

typedef struct circle_buf {
    uint32_t r;
    uint32_t w;
    uint32_t max_len;
    uint8_t  *buffer;    
    int32_t (*put)(struct circle_buf *pcb, uint8_t v);
    int32_t (*get)(struct circle_buf *pcb, uint8_t *pv);
}circle_buf_t;

#define FRAME_LENGTH 7



void circlebuf_init(void);

extern circle_buf_t g_rx_buf;



void uart7_wait_for_tx(void);
void uart7_wait_for_rx(void);
void print_msg(const char *msg);
void key_scan(void);


extern uint8_t dakai_flag;


#endif

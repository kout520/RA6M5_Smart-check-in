#ifndef ESP_CONNECT_H_
#define ESP_CONNECT_H_

// º¯ÊýÉùÃ÷




void uart2_init(void);
void uart2_process(void);
void uart2_send_value(int16_t value);

void process_uart2_data(void);
void send_wifi_to_esp32(const char* ssid, const char* pwd);
void uart2_send_value(int16_t value);
void uart2_send_string(const char* str);
void timer_callback(void);
void update_clock_display(void);
void dakai_clock_display(void);


#endif



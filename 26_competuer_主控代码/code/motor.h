#ifndef MOTOR_H
#define MOTOR_H



// 舵机ID定义
typedef enum {
    SERVO_1 = 0,
    SERVO_2,
    SERVO_3,
    SERVO_4
} servo_id_t;



// 函数声明
bool motor_init(void);
void motor_set_pulse(servo_id_t servo_id, uint16_t pulse_us);
void motor_stop(servo_id_t servo_id);
void motor_set_angle(servo_id_t servo_id, uint8_t angle);
void motor_task(void);

extern uint8_t motor_flag;

#endif // MOTOR_H

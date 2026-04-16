#include "headfile.h"  // 使用RASC生成的头文件

// 舵机参数
#define SERVO_MIN_PULSE_US  500    // 0.5ms
#define SERVO_MAX_PULSE_US  2500   // 2.5ms
#define SERVO_CENTER_PULSE_US 1200 // 1.5ms

// 全局变量存储定时器信息
static timer_info_t timer1_info;
static timer_info_t timer2_info;

/**
 * @brief 初始化舵机
 * @return true: 成功, false: 失败
 */
bool motor_init(void)
{
    fsp_err_t err;
    
    // 1. 打开定时器1
    err = R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);
    if (FSP_SUCCESS != err)
    {
        // 错误处理：可以打印日志或LED指示
        return false;
    }
    
    // 2. 打开定时器2
    err = R_GPT_Open(&g_timer2_ctrl, &g_timer2_cfg);
    if (FSP_SUCCESS != err)
    {
        // 关闭已打开的定时器1
        R_GPT_Close(&g_timer1_ctrl);
        return false;
    }
    
    // 3. 获取定时器信息（必须在Open之后，Start之前）
    err = R_GPT_InfoGet(&g_timer1_ctrl, &timer1_info);
    if (FSP_SUCCESS != err) return false;
    
    err = R_GPT_InfoGet(&g_timer2_ctrl, &timer2_info);
    if (FSP_SUCCESS != err) return false;
    
    // 4. 启动PWM输出
    err = R_GPT_Start(&g_timer1_ctrl);
    if (FSP_SUCCESS != err)
    {
        R_GPT_Close(&g_timer1_ctrl);
        R_GPT_Close(&g_timer2_ctrl);
        return false;
    }
    
    err = R_GPT_Start(&g_timer2_ctrl);
    if (FSP_SUCCESS != err)
    {
        R_GPT_Stop(&g_timer1_ctrl);
        R_GPT_Close(&g_timer1_ctrl);
        R_GPT_Close(&g_timer2_ctrl);
        return false;
    }
    
    // 5. 初始化到中位
    motor_set_pulse(SERVO_1, SERVO_CENTER_PULSE_US);
    motor_set_pulse(SERVO_2, SERVO_CENTER_PULSE_US);
    motor_set_pulse(SERVO_3, SERVO_CENTER_PULSE_US);
    motor_set_pulse(SERVO_4, SERVO_CENTER_PULSE_US);
    
    return true;
}

/**
 * @brief 计算占空比计数值
 * @param pulse_us 脉冲宽度（微秒）
 * @param period_counts 定时器周期计数值
 * @return 占空比计数值
 */
static uint32_t calculate_duty(uint32_t pulse_us, uint32_t period_counts)
{
    // 注意：pulse_us是微秒，但RASC配置周期是20ms = 20000us
    // 所以计算公式：duty = (pulse_us * period_counts) / 20000us
    return (pulse_us * period_counts) / 20000UL;
}

/**
 * @brief 设置舵机脉冲宽度
 * @param servo_id 舵机ID
 * @param pulse_us 脉冲宽度（500-2500微秒）
 */
void motor_set_pulse(servo_id_t servo_id, uint16_t pulse_us)
{
    // 限制范围
    if (pulse_us < SERVO_MIN_PULSE_US) pulse_us = SERVO_MIN_PULSE_US;
    if (pulse_us > SERVO_MAX_PULSE_US) pulse_us = SERVO_MAX_PULSE_US;
    
    uint32_t duty_counts;
    
    switch (servo_id)
    {
        case SERVO_1:
            duty_counts = calculate_duty(pulse_us, timer1_info.period_counts);
            R_GPT_DutyCycleSet(&g_timer1_ctrl, duty_counts, GPT_IO_PIN_GTIOCA);
            break;
            
        case SERVO_2:
            duty_counts = calculate_duty(pulse_us, timer1_info.period_counts);
            R_GPT_DutyCycleSet(&g_timer1_ctrl, duty_counts, GPT_IO_PIN_GTIOCB);
            break;
            
        case SERVO_3:
            duty_counts = calculate_duty(pulse_us, timer2_info.period_counts);
            R_GPT_DutyCycleSet(&g_timer2_ctrl, duty_counts, GPT_IO_PIN_GTIOCA);
            break;
            
        case SERVO_4:
            duty_counts = calculate_duty(pulse_us, timer2_info.period_counts);
            R_GPT_DutyCycleSet(&g_timer2_ctrl, duty_counts, GPT_IO_PIN_GTIOCB);
            break;
            
        default:
            // 无效的舵机ID
            break;
    }
}

/**
 * @brief 设置舵机角度（0-180度）
 * @param servo_id 舵机ID
 * @param angle 角度（0-180度）
 */
void motor_set_angle(servo_id_t servo_id, uint8_t angle)
{
    // 限制角度范围
    if (angle > 180) angle = 180;
    
    // 角度转脉冲宽度：0°=500us, 180°=2500us
    // 公式：pulse = 500 + (angle / 180) * 2000
    uint16_t pulse_us = 500 + (angle * 2000UL) / 180;
    
    motor_set_pulse(servo_id, pulse_us);
}

/**
 * @brief 停止舵机输出（设置为0占空比）
 * @param servo_id 舵机ID
 */
void motor_stop(servo_id_t servo_id)
{
    switch (servo_id)
    {
        case SERVO_1:
            R_GPT_DutyCycleSet(&g_timer1_ctrl, 0, GPT_IO_PIN_GTIOCA);
            break;
            
        case SERVO_2:
            R_GPT_DutyCycleSet(&g_timer1_ctrl, 0, GPT_IO_PIN_GTIOCB);
            break;
            
        case SERVO_3:
            R_GPT_DutyCycleSet(&g_timer2_ctrl, 0, GPT_IO_PIN_GTIOCA);
            break;
            
        case SERVO_4:
            R_GPT_DutyCycleSet(&g_timer2_ctrl, 0, GPT_IO_PIN_GTIOCB);
            break;
            
        default:
            break;
    }
}

/**
 * @brief 关闭所有舵机并释放资源
 */
void motor_deinit(void)
{
    // 停止所有舵机
    motor_stop(SERVO_1);
    motor_stop(SERVO_2);
    motor_stop(SERVO_3);
    motor_stop(SERVO_4);
    
    // 停止定时器
    R_GPT_Stop(&g_timer1_ctrl);
    R_GPT_Stop(&g_timer2_ctrl);
    
    // 关闭定时器
    R_GPT_Close(&g_timer1_ctrl);
    R_GPT_Close(&g_timer2_ctrl);
}

uint8_t motor_flag;

void motor_task(void)
{
    switch(motor_flag){
        case 0:
            motor_set_pulse(SERVO_1, 1200); 
            motor_set_pulse(SERVO_2, 1200); 
            motor_set_pulse(SERVO_3, 1200); 
            motor_set_pulse(SERVO_4, 1200); 
            break;
        case 1:
            motor_set_pulse(SERVO_1, 1800); 
            break;
        case 2:
            motor_set_pulse(SERVO_3, 1800); 
            break;
        case 3:
            motor_set_pulse(SERVO_2, 1800); 
            break;
    }
        
}








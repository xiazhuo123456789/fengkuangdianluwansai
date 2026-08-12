/*
 * motor.c
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#include "motor.h"

int pwm_l,pwm_r;

void motor_proc(int duty1, int duty2)
{
    // ===== 总限幅（原始逻辑，保持不变）=====
    if (duty1 > MOTOR_DUTY_LIMIT)
        duty1 = MOTOR_DUTY_LIMIT;
    else if (duty1 < -MOTOR_DUTY_LIMIT)
        duty1 = -MOTOR_DUTY_LIMIT;

    if (duty2 > MOTOR_DUTY_LIMIT)
        duty2 = MOTOR_DUTY_LIMIT;
    else if (duty2 < -MOTOR_DUTY_LIMIT)
        duty2 = -MOTOR_DUTY_LIMIT;

    // ===== 方案2：PWM 下限保护 =====
#if MOTOR_DZ_LOWER_PROTECT
    if (duty1 > 0 && duty1 < MOTOR_DZ_LOWER_LIMIT)
        duty1 = MOTOR_DZ_LOWER_LIMIT;
    if (duty2 > 0 && duty2 < MOTOR_DZ_LOWER_LIMIT)
        duty2 = MOTOR_DZ_LOWER_LIMIT;
#endif

    // ===== 方案5：左右死区补偿 =====
#if MOTOR_DZ_COMPENSATE
    duty1 += MOTOR_DZ_LEFT_OFFSET;
    duty2 += MOTOR_DZ_RIGHT_OFFSET;

    // 补偿后再限幅一次
    if (duty1 > MOTOR_DUTY_LIMIT)  duty1 = MOTOR_DUTY_LIMIT;
    if (duty1 < -MOTOR_DUTY_LIMIT) duty1 = -MOTOR_DUTY_LIMIT;
    if (duty2 > MOTOR_DUTY_LIMIT)  duty2 = MOTOR_DUTY_LIMIT;
    if (duty2 < -MOTOR_DUTY_LIMIT) duty2 = -MOTOR_DUTY_LIMIT;
#endif

    // ===== 原始输出逻辑 =====
    if (duty1 >= 0)
    {
        gpio_set_level(MOTOR_DIR_L, GPIO_LOW);
        pwm_set_duty(MOTOR_PWM_L, duty1);
    }
    else
    {
        gpio_set_level(MOTOR_DIR_L, GPIO_HIGH);
        pwm_set_duty(MOTOR_PWM_L, (-duty1));
    }

    if (duty2 >= 0)
    {
        gpio_set_level(MOTOR_DIR_R, GPIO_HIGH);
        pwm_set_duty(MOTOR_PWM_R, duty2);
    }
    else
    {
        gpio_set_level(MOTOR_DIR_R, GPIO_LOW);
        pwm_set_duty(MOTOR_PWM_R, (-duty2));
    }

    pwm_l = duty1;
    pwm_r = duty2;
}

void set_fuya_pwm(int fuya)
{
    if (fuya >= 0)
    {
        pwm_set_duty(FUYA_PWM, fuya);
    }
    else
    {
        pwm_set_duty(FUYA_PWM, (-fuya));
    }
}

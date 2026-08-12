/*
 * motor.h
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#ifndef CODE_MOTOR_H_
#define CODE_MOTOR_H_

#include "zf_common_headfile.h"

#define MOTOR_DUTY_LIMIT    6000

// ============ 电机死区保护（开关宏） ============

// 方案2：PWM 下限保护 —— 正向 PWM 低于阈值时强制拉高，防止掉到死区电压以下
//         设为 1 开启，0 关闭
#define MOTOR_DZ_LOWER_PROTECT   1
#define MOTOR_DZ_LOWER_LIMIT     400     // 正向最低 PWM

// 方案5：左右死区补偿 —— 左右电机各加偏置，补偿个体差异
//         设为 1 开启，0 关闭
#define MOTOR_DZ_COMPENSATE      1
#define MOTOR_DZ_LEFT_OFFSET     100     // 左电机补偿量
#define MOTOR_DZ_RIGHT_OFFSET    0       // 右电机补偿量

void motor_proc(int duty1, int duty2);
void set_fuya_pwm(int fuya);

#endif /* CODE_MOTOR_H_ */

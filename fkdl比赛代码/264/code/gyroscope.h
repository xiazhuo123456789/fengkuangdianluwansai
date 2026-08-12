/*
 * gyroscope.h
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#ifndef CODE_GYROSCOPE_H_
#define CODE_GYROSCOPE_H_

#include "zf_common_headfile.h"

typedef struct
{
    float Xdata;
    float Ydata;
    float Zdata;
} gyro_param_t;

typedef struct
{
    float acc_x;
    float acc_y;
    float acc_z;

    float gyro_x;
    float gyro_y;
    float gyro_z;
} IMU_param_t;

extern IMU_param_t IMU_Data;
extern uint8 gyro_Offset_flag;
extern float yaw_total;

void IMU_gyro_Offset_Init(void);
void IMU_GetValues(void);
void gyro_init(void);
void gyro_proc(void);

#endif /* CODE_GYROSCOPE_H_ */

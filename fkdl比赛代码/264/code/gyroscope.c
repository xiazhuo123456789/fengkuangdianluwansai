/*
 * gyroscope.c
 *
 *  Created on: 2026å¹´6æœˆ1æ—¥
 *      Author: xz123
 */

#include "gyroscope.h"

gyro_param_t Gyro_Offset;
IMU_param_t IMU_Data;

uint8 gyro_Offset_flag = 0;

void IMU_gyro_Offset_Init(void)
{
    Gyro_Offset.Xdata = 0;
    Gyro_Offset.Ydata = 0;
    Gyro_Offset.Zdata = 0;

    for (uint16_t i = 0; i < 100; i++)
    {
        Gyro_Offset.Xdata += imu660rb_gyro_x;
        Gyro_Offset.Ydata += imu660rb_gyro_y;
        Gyro_Offset.Zdata += imu660rb_gyro_z;
        system_delay_ms(5);
    }

    Gyro_Offset.Xdata /= 100;
    Gyro_Offset.Ydata /= 100;
    Gyro_Offset.Zdata /= 100;

    gyro_Offset_flag = 1;
}

void IMU_GetValues(void)
{
    imu660rb_get_gyro();
    imu660rb_get_acc();

    IMU_Data.gyro_x = ((float)imu660rb_gyro_x - Gyro_Offset.Xdata) * PI / 180 / 14.3f;
    IMU_Data.gyro_y = ((float)imu660rb_gyro_y - Gyro_Offset.Ydata) * PI / 180 / 14.3f;
    IMU_Data.gyro_z = ((float)imu660rb_gyro_z - Gyro_Offset.Zdata) * PI / 180 / 14.3f;

    IMU_Data.acc_x = (((float)imu660rb_acc_x) * 0.3f) + IMU_Data.acc_x * 0.7f;
    IMU_Data.acc_y = (((float)imu660rb_acc_y) * 0.3f) + IMU_Data.acc_y * 0.7f;
    IMU_Data.acc_z = (((float)imu660rb_acc_z) * 0.3f) + IMU_Data.acc_z * 0.7f;
}

float yaw_total = 0;

static void IMU_Handle_0(void)
{
    yaw_total += RAD_TO_ANGLE(IMU_Data.gyro_z * 0.005);
}

static void IMU_YAW_integral(void)
{
    // ZÖáËÀÇø£ºÐ¡ÓÚãÐÖµ²»»ý·Ö
    if (IMU_Data.gyro_z >= 0.015f || IMU_Data.gyro_z <= -0.015f)
    {
        IMU_Handle_0();
    }
}

void gyro_init(void)
{
    imu660rb_init();
    IMU_gyro_Offset_Init();
}

void gyro_proc(void)
{
    IMU_GetValues();
    IMU_YAW_integral();
}

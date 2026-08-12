/*
 * utils.c
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#include "utils.h"

int abs_int(int value)
{
    if (value >= 0)
        return value;
    else
        return -value;
}

float abs_float(float value)
{
    if (value >= 0)
        return value;
    else
        return -value;
}

int compare_int(int x, int y)
{
    if (x >= y)
        return 1;
    else
        return 0;
}

int limit_int(int value, int max, int min)
{
    if (value > max)
        return max;
    if (value < min)
        return min;
    return value;
}

float limit_float(float value, float max, float min)
{
    if (value > max)
        return max;
    if (value < min)
        return min;
    return value;
}

float fast_sqrt(float num)
{
    if (num <= 0.0f) return 0.0f;
    return num * fast_inv_sqrt(num);
}

float fast_inv_sqrt(float num)
{
    float halfx = 0.5f * num;
    float y = num;
    long i = *(long *)&y;
    i = 0x5f375a86 - (i >> 1);

    y = *(float *)&i;
    y = y * (1.5f - (halfx * y * y));
    y = y * (1.5f - (halfx * y * y));
    return y;
}

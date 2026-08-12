/*
 * utils.h
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#ifndef CODE_UTILS_H_
#define CODE_UTILS_H_

#include "zf_common_headfile.h"

int abs_int(int value);
float abs_float(float value);
int compare_int(int x, int y);
int limit_int(int value, int max, int min);
float limit_float(float value, float max, float min);
float fast_sqrt(float num);
float fast_inv_sqrt(float num);

#endif /* CODE_UTILS_H_ */

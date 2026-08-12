/*
 * control.h
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#ifndef CODE_ENCODER_H_
#define CODE_ENCODER_H_

#include "zf_common_headfile.h"

#define DELTA_T             0.005f
#define I_TERM_LIMIT        2000
#define ONE_METER           36000.0f

extern int l_encoder;
extern int r_encoder;
extern float total_dist;
extern float l_now_vel;
extern float r_now_vel;
extern float car_vel;

void encoder_proc(void);

#endif /* CODE_ENCODER_H_ */

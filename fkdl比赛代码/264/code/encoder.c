/*
 * control.c
 *
 *  Created on: 2026Âπ¥6Êúà1Êó•
 *      Author: xz123
 */

#include "encoder.h"

int l_encoder = 0;
int r_encoder = 0;
float total_dist = 0.0f;
float l_now_vel = 0.0f;
float r_now_vel = 0.0f;
float car_vel = 0.0f;

void encoder_proc(void)
{
    float now_vel = 0;
    float l_dis, r_dis, dis = 0.0;

    l_encoder = -encoder_get_count(ENCODER_L);
    r_encoder =  encoder_get_count(ENCODER_R);

    encoder_clear_count(ENCODER_L);
    encoder_clear_count(ENCODER_R);

    // ÀŸ∂»º∆À„
    l_now_vel = (float)(l_encoder / ONE_METER / DELTA_T * 100);
    r_now_vel = (float)(r_encoder / ONE_METER / DELTA_T * 100);
    car_vel = (l_now_vel + r_now_vel) / 2;
    now_vel = car_vel;

    // æ‡¿Îº∆À„
    l_dis = (float)l_encoder / ONE_METER;
    r_dis = (float)r_encoder / ONE_METER;
    dis = (l_dis + r_dis) / 2;
    total_dist += dis;
}

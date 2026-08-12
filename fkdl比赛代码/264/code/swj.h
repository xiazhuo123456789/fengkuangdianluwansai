/*
 * swj.h
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#ifndef CODE_SWJ_H_
#define CODE_SWJ_H_

#include "zf_common_headfile.h"

extern volatile uint8 run_flag;

void wireless_uart_proc(void);
void ble6a20_proc(void);
void wifi_spi_proc(void);
void debug_proc(void);

#endif /* CODE_SWJ_H_ */

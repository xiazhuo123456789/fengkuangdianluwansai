/*
 * menu.h
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#ifndef CODE_MENU_H_
#define CODE_MENU_H_

#include "zf_common_headfile.h"

// ============ 显示页面模式 ============
#define PAGE_CAMERA     0   // 摄像头图像页
#define PAGE_SETTING    1   // 参数设置页

extern uint8 display_page;      // 当前显示页面
extern uint8 path_select_idx;   // 当前路径表编号（0~6，对应 PATH 1~7）

void menu_proc(uint8 key_val, uint8 key_down, uint8 key_up);           // 按键菜单处理（在 key_proc 中调用）
void menu_display(void);        // 屏幕显示调度（在主循环中调用）

#endif /* CODE_MENU_H_ */

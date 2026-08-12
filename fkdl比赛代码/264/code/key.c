/*
 * key.c
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#include "key.h"
#include "menu.h"

uint8 key_val, key_down, key_up, key_old;
uint8 start_flag;

void key_proc(void)
{
    key_val = key_read();
    key_down = key_val & (key_val ^ key_old);
    key_up  = ~key_val & (key_val ^ key_old);
    key_old = key_val;

    switch (key_down)
    {
        case 4: // 按键4：启动/复位
            if (++start_flag >= 2)
            {
                start_flag = 0;
            }
            break;

        case 1: // 按键1,2 → 边沿触发菜单
        case 2:
            menu_proc(key_val, key_down, key_up);
            break;

        default:
            break;
    }

    // 按键3：按下/按住/松开都调用 menu_proc（实现长按/短按检测）
    if (key_val == 3 || key_up == 3)
    {
        menu_proc(key_val, key_down, key_up);
    }
}

uint8 key_read(void)
{
    uint8 temp = 0;

    if (gpio_get_level(KEY1) == 0) temp = 1;
    if (gpio_get_level(KEY2) == 0) temp = 2;
    if (gpio_get_level(KEY3) == 0) temp = 3;
    if (gpio_get_level(KEY4) == 0) temp = 4;

    return temp;
}

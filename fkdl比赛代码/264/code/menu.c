/*
 * menu.c
 *
 *  Created on: 2026年6月1日
 *      Author: xz123
 */

#include "menu.h"
#include "image.h"
#include "control.h"
#include "init.h"

uint8 display_page = PAGE_CAMERA;   // 默认：摄像头页面
uint8 path_select_idx = PATH_SELECT - 1;  // 默认：编译时 PATH_SELECT 决定

// 路径表名称（用于屏幕显示）
static const char *path_names[] = {
    "PATH 1", "PATH 2", "PATH 3", "PATH 4",
    "PATH 5", "PATH 6", "PATH 7"
};
#define PATH_COUNT  (sizeof(path_names) / sizeof(path_names[0]))

/**
 * @brief 按键菜单处理
 *        按键1: 切换显示页面（摄像头 <-> 设置页）
 *        按键2: 设置页中切换路径表（轮回切换）
 *        按键3: 设置页中 短按 base_speed +50 / 长按(≥500ms) base_speed -50
 *        按键4: 保持不动（启动/复位）
 */
void menu_proc(uint8 key_val, uint8 key_down, uint8 key_up)
{
    static uint8  key3_holding = 0;
    static uint16 key3_hold_cnt = 0;

    // ===== 按键3 长按/短按检测 =====
    if (key_down == 3)
    {
        key3_holding = 1;
        key3_hold_cnt = 0;
    }

    if (key3_holding && key_val == 3)
    {
        key3_hold_cnt++;
    }

    if (key3_holding && key_up == 3)
    {
        key3_holding = 0;
        if (display_page == PAGE_SETTING)
        {
            if (key3_hold_cnt >= 50)  // 长按 ≥ 500ms  →  -50
            {
                base_speed -= 50;
                if (base_speed < 0) base_speed = 0;
            }
            else                      // 短按  →  +50
            {
                base_speed += 50;
            }
        }
    }

    switch (key_down)
    {
        case 1: // 按键1：切换显示页面
            if (display_page == PAGE_CAMERA)
                display_page = PAGE_SETTING;
            else
                display_page = PAGE_CAMERA;
            break;

        case 2: // 按键2：设置页中轮回切换路径表
            if (display_page == PAGE_SETTING)
            {
                path_select_idx++;
                if (path_select_idx >= PATH_COUNT)
                    path_select_idx = 0;
                path_update();  // 同步到 image.c 的运行时路径表
            }
            break;

        // 按键3：短按+50 / 长按-50（见上方状态机）

        // 按键4 不动，保留给 start_flag

        default:
            break;
    }
}

/**
 * @brief 屏幕显示调度
 *        摄像头页面: 显示灰度和图像信息
 *        参数设置页: 显示当前路径表编号和基础速度
 */
void menu_display(void)
{
    if (display_page == PAGE_CAMERA)
    {
        // 摄像头页面（原逻辑）
        ips114_show_gray_image(0, 0, bin_image[0], COL, ROW, COL, ROW, 0);
        ips114_show_int(200, 0,  (int)image_err, 3);
        ips114_show_int(200, 20, node_state, 3);
        ips114_show_int(200, 40, max_gray, 3);        // 原图最大灰度值
    }
    else // PAGE_SETTING
    {
        // 清屏
        ips114_set_color(RGB565_WHITE, RGB565_BLACK);
        ips114_full(RGB565_BLACK);
        ips114_set_color(RGB565_WHITE, RGB565_BLACK);

        // 标题
        ips114_show_string(5, 5, "=== SETTING ===");

        // 路径表编号
        ips114_show_string(5, 25, "Path: ");
        ips114_show_int(55, 25, (int)(path_select_idx + 1), 1);
        ips114_show_string(75, 25, (char *)path_names[path_select_idx]);

        // 基础速度
        ips114_show_string(5, 45, "BaseSpeed: ");
        ips114_show_int(100, 45, base_speed, 5);

        // 提示
        ips114_show_string(5, 75, "KEY2: Path  KEY3:+/-50");
        ips114_show_string(5, 93, "KEY1: Back  KEY4: Run");
    }
}

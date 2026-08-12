#include "zf_common_headfile.h"
#include "menu.h"
#pragma section all "cpu0_dsram"

int core0_main(void)
{
    peripheral_init();

    // 程序启动时初始化路径表
    path_update();

    while (TRUE)
    {
        debug_proc();
        image_proc();
        fuya_proc();

        // 屏幕显示调度
        if (!start_flag)
        {
            menu_display();  // 替代原来的直接显示，支持页面切换
        }

        printf("%d, %d\r\n", l_encoder, r_encoder);
    }
}

#pragma section all restore

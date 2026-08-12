#include "isr_config.h"
#include "isr.h"

IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU60_CH1);

    static uint8 timer_10ms;

    if(++timer_10ms >= 10)
    {
        key_proc();

        timer_10ms = 0;
    }
    control_proc();
}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH0);

    gyro_proc();
    encoder_proc();
}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH1);

    run_time_proc();
}
// **************************** PIT中断函数 ****************************


// **************************** 外部中断函数 ****************************
IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    if(exti_flag_get(ERU_CH4_REQ8_P33_7))           // 通道4中断
    {
        exti_flag_clear(ERU_CH4_REQ8_P33_7);
        camera_vsync_handler_1();                   // 摄像头1 触发采集统一回调函数
    }

    if(exti_flag_get(ERU_CH0_REQ0_P15_4))           // 通道0中断
    {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        imu660rc_callback();                         // 660RC 模块 INT 更新中断
    }

}

// 由于摄像头pclk引脚默认占用了 1通道，用于触发DMA，因此这里不再定义中断函数
IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

    if(exti_flag_get(ERU_CH1_REQ10_P14_3))          // 通道1中断
    {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
    }

    if(exti_flag_get(ERU_CH5_REQ1_P15_8))           // 通道5中断
    {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
    }
}

IFX_INTERRUPT(exti_ch2_ch6_isr, 0, EXTI_CH2_CH6_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    if(exti_flag_get(ERU_CH2_REQ7_P00_4))           // 通道2中断
    {
        exti_flag_clear(ERU_CH2_REQ7_P00_4);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
    }
    if(exti_flag_get(ERU_CH6_REQ9_P20_0))           // 通道6中断
    {
        exti_flag_clear(ERU_CH6_REQ9_P20_0);
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头PCLK 若已使用摄像头 请不要再添加其他内容
    }
}

IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

    if(exti_flag_get(ERU_CH3_REQ3_P10_3))           // 通道3中断
    {
        exti_flag_clear(ERU_CH3_REQ3_P10_3);
        camera_vsync_handler_2();                   // 摄像头2触发采集统一回调函数
    }

    if(exti_flag_get(ERU_CH7_REQ16_P15_1))          // 通道7中断
    {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
        // 注意: 该外部中断为摄像头场中断 若已使用摄像头 请不要再添加其他内容
    }
}
// **************************** 外部中断函数 ****************************


// **************************** DMA中断函数 ****************************
IFX_INTERRUPT(dma_ch6_isr, 0, DMA_INT_PRIO_1)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    camera_dma_handler_1();                         // 摄像头1 采集完成统一回调函数
}

IFX_INTERRUPT(dma_ch7_isr, 0, DMA_INT_PRIO_2)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    camera_dma_handler_2();                         // 摄像头2 采集完成统一回调函数
}
// **************************** DMA中断函数 ****************************


// **************************** 串口中断函数 ****************************
// 串口0默认作为调试串口
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套



}
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

#if DEBUG_UART_USE_INTERRUPT                        // 如果开启 debug 串口中断
        debug_interrupr_handler();                  // 调用 debug 串口接收处理函数 数据会被 debug 环形缓冲区读取
#endif                                              // 如果修改了 DEBUG_UART_INDEX 那这段代码需要放到对应的串口中断去
}


// 串口1默认连接到摄像头配置串口
IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套




}
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    camera_uart_handler_1();                        // 摄像头参数配置统一回调函数
}

// 串口2默认连接到无线转串口模块
IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套



}

IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    wireless_module_uart_handler();                 // 无线模块统一回调函数



}
// 串口3默认连接到GPS定位模块
IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套



}

IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套

    gnss_uart_callback();                           // GNSS串口回调函数



}

// 串口通讯错误中断
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart0_handle);
}
IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart1_handle);
}
IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart2_handle);
}
IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // 开启中断嵌套
    IfxAsclin_Asc_isrError(&uart3_handle);
}

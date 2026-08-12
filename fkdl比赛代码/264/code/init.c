/*
 * init.c
 *
 *  Created on: 2026å¹´6æœˆ1æ—¥
 *      Author: xz123
 */

#include "init.h"

void peripheral_init(void)
{
    clock_init();                   // »ñÈ¡Ê±ÖÓÆµÂÊ<Îñ±Ø±£Áô>
    debug_init();                   // ³õÊ¼»¯Ä¬ÈÏµ÷ÊÔ´®¿Ú

    // ÎÞÏß´®¿Ú³õÊ¼»¯
#if WIRELESS_UART_ENABLE
    if(wireless_uart_init())        // ÅÐ¶ÏÊÇ·ñÍ¨¹ý³õÊ¼»¯
    {
        while(1)
            ;
    }
    wireless_uart_send_byte('\r');
    wireless_uart_send_byte('\n');
    wireless_uart_send_string("wireless_uart_init successful.\r\n");
#endif

    // À¶ÑÀ³õÊ¼»¯
#if BLE6A20_ENABLE
    while(ble6a20_init())           // ÅÐ¶ÏÊÇ·ñÍ¨¹ý³õÊ¼»¯
    {
        printf("ble6a20 init error.\r\n");
        printf("restart ble6a20 init.\r\n");
    }
    ble6a20_send_byte('\r');
    ble6a20_send_byte('\n');
    ble6a20_send_string("BLE6A20_init successful.\r\n");
#endif

    // wifi_spi³õÊ¼»¯
#if WIFI_SPI_ENABLE
    while(wifi_spi_init(WIFI_SSID_TEST, WIFI_PASSWORD_TEST))
    {
        printf("\r\n connect wifi failed. \r\n");
        system_delay_ms(100);               // ³õÊ¼»¯Ê§°Ü µÈ´ý 100ms
    }
    // zf_device_wifi_spi.h ÎÄ¼þÄÚµÄºê¶¨Òå¿ÉÒÔ¸ü¸ÄÄ£¿éÁ¬½Ó(½¨Á¢) WIFI Ö®ºó£¬ÊÇ·ñ×Ô¶¯Á¬½Ó TCP ·þÎñÆ÷¡¢´´½¨ UDP Á¬½Ó
    if(1 != WIFI_SPI_AUTO_CONNECT)          // Èç¹ûÃ»ÓÐ¿ªÆô×Ô¶¯Á¬½Ó ¾ÍÐèÒªÊÖ¶¯Á¬½ÓÄ¿±ê IP
    {
        while(wifi_spi_socket_connect(      // ÏòÖ¸¶¨Ä¿±ê IP µÄ¶Ë¿Ú½¨Á¢ TCP Á¬½Ó
            "TCP",                          // Ö¸¶¨Ê¹ÓÃTCP·½Ê½Í¨Ñ¶
            TCP_TARGET_IP,                  // Ö¸¶¨Ô¶¶ËµÄIPµØÖ·£¬ÌîÐ´ÉÏÎ»»úµÄIPµØÖ·
            TCP_TARGET_PORT,                // Ö¸¶¨Ô¶¶ËµÄ¶Ë¿ÚºÅ£¬ÌîÐ´ÉÏÎ»»úµÄ¶Ë¿ÚºÅ£¬Í¨³£ÉÏÎ»»úÄ¬ÈÏÊÇ8080
            WIFI_LOCAL_PORT))               // Ö¸¶¨±¾»úµÄ¶Ë¿ÚºÅ
        {
            // Èç¹ûÒ»Ö±½¨Á¢Ê§°Ü ¿¼ÂÇÒ»ÏÂÊÇ²»ÊÇÃ»ÓÐ½ÓÓ²¼þ¸´Î»
            printf("\r\n Connect TCP Servers error, try again.");
            system_delay_ms(100);           // ½¨Á¢Á¬½ÓÊ§°Ü µÈ´ý 100ms
        }
    }
#endif

    // ÉãÏñÍ·³õÊ¼»¯
    while(mt9v03x_double_init(mt9v03x_1))
        ;

    // Öð·ÉÖúÊÖ³õÊ¼»¯
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, bin_image[0], COL, ROW);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, ROW, l_border, mid_line, r_border, NULL, NULL ,NULL);

    // ÆÁÄ»³õÊ¼»¯
    ips114_set_color(RGB565_BLACK, RGB565_WHITE);
    ips114_set_font(IPS114_8X16_FONT);
    ips114_set_dir(IPS114_PORTAIT_180);
    ips114_init();

    // ²Ëµ¥³õÊ¼»¯
//    menu_init();

    // ±àÂëÆ÷³õÊ¼»¯
    encoder_dir_init(ENCODER_L, ENCODER_PULSE_L, ENCODER_DIR_L);
    encoder_dir_init(ENCODER_R, ENCODER_PULSE_R, ENCODER_DIR_R);

    // Çý¶¯µç»ú³õÊ¼»¯
    pwm_init(MOTOR_PWM_L, 17000, 0);
    pwm_init(MOTOR_PWM_R, 17000, 0);
    gpio_init(MOTOR_DIR_L, GPO, GPIO_HIGH, GPO_PUSH_PULL);
    gpio_init(MOTOR_DIR_R, GPO, GPIO_HIGH, GPO_PUSH_PULL);

    // ¸ºÑ¹µç»ú³õÊ¼»¯
    pwm_init(FUYA_PWM, 17000, 0);

    // ·äÃùÆ÷³õÊ¼»¯
//    gpio_init(BUZZER_PIN, GPO, GPIO_LOW, GPO_PUSH_PULL);

    // °´¼ü³õÊ¼»¯
    gpio_init(KEY1, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY2, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY3, GPI, GPIO_HIGH, GPI_PULL_UP);
    gpio_init(KEY4, GPI, GPIO_HIGH, GPI_PULL_UP);

    // ÉÁ´æ³õÊ¼»¯
//    flash_init();

    // ÍÓÂÝÒÇ³õÊ¼»¯
    gyro_init();

    // PID²ÎÊý³õÊ¼»¯
    pid_param_init();

    // ¶¨Ê±Æ÷³õÊ¼»¯
    pit_ms_init(CCU60_CH1, 1);
    pit_ms_init(CCU61_CH0, 5);
    pit_ms_init(CCU61_CH1, 10);

    // µÈ´ýËùÓÐºËÐÄ³õÊ¼»¯Íê±Ï
    cpu_wait_event_ready();
}

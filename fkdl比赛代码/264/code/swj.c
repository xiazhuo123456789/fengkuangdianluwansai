/*
 * swj.c
 *
 *  Created on: 2026å¹´6æœˆ1æ—¥
 *      Author: xz123
 */

#include "swj.h"

volatile uint8 run_flag = 0;
uint8 threshold = 135;

void wireless_uart_proc(void)
{
    uint8 data_buffer[32];
    uint8 data_len;
    char data_string[30];
    uint8 i = 2; // ½ÓÊÕÊı¾İÆğÊ¼Î»Îª2

    /************************* wireless_uart½ÓÊÕÏûÏ¢ *************************/
    data_len = (uint8)wireless_uart_read_buffer(data_buffer, 32);
    if (data_len != 0)
    {
        if (data_buffer[0] == 'r')
        {
            run_flag = 1;
        }
        if (data_buffer[0] == 's')
        {
            run_flag = 0;
        }
        if (data_buffer[0] == 't' && data_buffer[1] == ':')
        {
            while (data_buffer[i] != 0)
            {
                data_string[i - 2] = data_buffer[i];
                i++;
            }
            data_string[i - 2] = '\0';
            threshold = (uint8)strtod(data_string, NULL);
        }
    }
    /************************* wireless_uart·¢ËÍĞÅÏ¢ *************************/
    static uint8 last_node_count = 0;
    static uint8 last_verify_flag = 0;

    // ===== ½Úµã¼ì²âÉÏ±¨ =====
    if (node_count != last_node_count && node_count > 0)
    {
        char node_string[120];
        const char *plan_str = "Î´Öª";
        uint8 cur_index = (node_count - 1) % path_len;

        switch (path[cur_index])
        {
            case TURN_LEFT:   plan_str = "×ó×ª"; break;
            case TURN_RIGHT:  plan_str = "ÓÒ×ª"; break;
            case GO_STRAIGHT: plan_str = "Ö±ĞĞ"; break;
            default:          plan_str = "Î´Öª"; break;
        }

        // Èç¹ûplan_turn·¢ÏÖ¹æ»®·½ÏòÔÚÍ¼ÏñÖĞÕÒ²»µ½¶ÔÓ¦Â·¾¶
        if (plan_warn_flag)
        {
            sprintf(node_string, "[!¾¯¸æ] ½Úµã%d ¹æ»®:%s µ«Í¼ÏñÎ´¼ì²âµ½¸Ã·½ÏòÂ·¾¶!\r\n",
                    node_count, plan_str);
            wireless_uart_send_string(node_string);
        }

        sprintf(node_string, "×Ü½Úµã:%d | µ±Ç°µÚ%d¸ö | ¹æ»®:%s\r\n",
                path_len, node_count, plan_str);
        wireless_uart_send_string(node_string);

        last_node_count = node_count;
        last_verify_flag = 0;   // µÈ´ı×ªÏòÍê³ÉºóµÄĞ£Ñé½á¹û
    }

    // ===== ×ªÏòÍê³ÉĞ£ÑéÉÏ±¨ =====
    if (turn_verify_flag != 0 && last_verify_flag == 0)
    {
        char verify_string[120];
        const char *actual_str = "Î´Öª";

        switch (actual_turn_dir)
        {
            case TURN_LEFT:   actual_str = "×ó×ª"; break;
            case TURN_RIGHT:  actual_str = "ÓÒ×ª"; break;
            case GO_STRAIGHT: actual_str = "Ö±ĞĞ"; break;
            default:          actual_str = "Î´Öª"; break;
        }

        if (turn_verify_flag == 1)
        {
            // Æ¥Åä
            sprintf(verify_string, "[OK] ½Úµã%d ×ªÏòÍê³É Êµ¼Ê:%s (Æ«º½:%.1f->%.1f)\r\n",
                    node_count, actual_str, yaw_before_turn, yaw_after_turn);
        }
        else if (turn_verify_flag == 2)
        {
            // ²»Æ¥Åä
            const char *plan_str = "Î´Öª";
            uint8 cur_index = (node_count - 1) % path_len;
            switch (path[cur_index])
            {
                case TURN_LEFT:   plan_str = "×ó×ª"; break;
                case TURN_RIGHT:  plan_str = "ÓÒ×ª"; break;
                case GO_STRAIGHT: plan_str = "Ö±ĞĞ"; break;
                default:          plan_str = "Î´Öª"; break;
            }

            sprintf(verify_string,
                    "[!!!´íÎó] ½Úµã%d ¹æ»®:%s Êµ¼Ê:%s (Æ«º½:%.1f->%.1f)\r\n",
                    node_count, plan_str, actual_str, yaw_before_turn, yaw_after_turn);
        }

        wireless_uart_send_string(verify_string);
        last_verify_flag = turn_verify_flag;
        turn_verify_flag = 0;   // Çå³ı±êÖ¾£¬µÈ´ıÏÂÒ»¸ö½Úµã
    }
}

void ble6a20_proc(void)
{
    uint8 data_buffer[32];
    uint8 data_len;
    char data_string[30];
    uint8 i = 2;
    float steer_kp, steer_gkd;

    /************************* ble6a20À¶ÑÀ½ÓÊÕÏûÏ¢ *************************/
    data_len = (uint8)ble6a20_read_buffer(data_buffer, 32);
    if (data_len != 0)
    {
        if (data_buffer[0] == 'r')
        {
            run_flag = 1;
        }
        else if (data_buffer[0] == 's')
        {
            run_flag = 0;
        }
        else if (data_buffer[0] == 't' && data_buffer[1] == ':')
        {
            i = 2;
            memset(data_string, 0, sizeof(data_string));
            while (data_buffer[i] != 0 && i < 30)
            {
                data_string[i - 2] = data_buffer[i];
                i++;
            }
            threshold = (uint8)strtod(data_string, NULL);
            sprintf(data_string, "Threshold:%d\n", threshold);
            ble6a20_send_string(data_string);
        }
        else if (data_buffer[0] == 'v' && data_buffer[1] == ':')
        {
            i = 2;
            memset(data_string, 0, sizeof(data_string));
            while (data_buffer[i] != 0 && i < 30)
            {
                data_string[i - 2] = data_buffer[i];
                i++;
            }
            base_speed = atoi(data_string);
            sprintf(data_string, "BaseSpeed:%d\n", base_speed);
            ble6a20_send_string(data_string);
        }
        else if (data_buffer[0] == 's' && data_buffer[1] == 'p' && data_buffer[2] == ':')
        {
            i = 3;
            memset(data_string, 0, sizeof(data_string));
            while (data_buffer[i] != 0 && i < 30)
            {
                data_string[i - 3] = data_buffer[i];
                i++;
            }
            steer_kp = (float)atof(data_string);
            steer_pid.kp = steer_kp;
            sprintf(data_string, "Steer_KP:%.2f\n", steer_kp);
            ble6a20_send_string(data_string);
        }
        else if (data_buffer[0] == 'g' && data_buffer[1] == 'd' && data_buffer[2] == ':')
        {
            i = 3;
            memset(data_string, 0, sizeof(data_string));
            while (data_buffer[i] != 0 && i < 30)
            {
                data_string[i - 3] = data_buffer[i];
                i++;
            }
            steer_gkd = (float)atof(data_string);
            steer_pid.gkd = steer_gkd;
            sprintf(data_string, "Gyro_KD:%.2f\n", steer_gkd);
            ble6a20_send_string(data_string);
        }
        else if (data_buffer[0] == '?')
        {
            char param_str[100];
            sprintf(param_str,
                    "Steer_KP:%.2f,Gyro_KD:%.2f\n",
                    steer_pid.kp, steer_pid.gkd);
            ble6a20_send_string(param_str);
        }
    }
    /************************* ble6a20À¶ÑÀ·¢ËÍÏûÏ¢ *************************/
//    char vofa_string[50];
////    sprintf(vofa_string, "%d\n", run_flag);
////    sprintf(vofa_string, "%d, %d\n", CarInfo.l_encoder, CarInfo.r_encoder);
//    sprintf(vofa_string, "lp:%.1f,li:%.1f | rp:%.1f,ri:%.1f | pp:%.1f,pd:%.1f,gd:%.1f\n",
//            l_speed_kp, l_speed_ki,
//            r_speed_kp, r_speed_ki,
//            place_kp, place_kd, place_gyro_kd);
//    ble6a20_send_string(vofa_string);
    static uint16 send_cnt = 0;
    if (++send_cnt >= 50) // ÖÜÆÚĞÔ·¢ËÍ×´Ì¬£¨±ÜÃâË¢ÆÁ£©
    {
        send_cnt = 0;
        char status_str[50];
        sprintf(status_str, "Run:%d|Thresh:%d\n", run_flag, threshold);
        ble6a20_send_string(status_str);
    }
}

void wifi_spi_proc(void)
{
    // ·¢ËÍÍ¼Ïñ
    seekfree_assistant_camera_send();
}

void debug_proc(void)
{
#if WIRELESS_UART_ENABLE
        wireless_uart_proc();
#endif

#if BLE6A20_ENABLE
        ble6a20_proc();
#endif

#if WIFI_SPI_ENABLE
        wifi_spi_proc();
#endif
}

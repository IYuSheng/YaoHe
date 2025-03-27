#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>
#include "led_driver.h"
#include "Uart3_8266.h"
#include "Beep.h"
#include <board.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/* 定义 LED 设备实例 */
static struct rt_led_device led1;



int main(void)
{
        /* 启动 PWM 蜂鸣器线程 */
        if (pwm_beep_start() != RT_EOK)
        {
            LOG_E("Failed to start PWM beep thread");
        }

        /* 初始化LED并启动控制线程 */
        if (rt_led_init_with_thread(&led1, GET_PIN(A, 4), PIN_HIGH, 512, 20, 10) != RT_EOK) {
            return -RT_ERROR;
        }

       CreatUart3TestEntry();//esp8266收发接收线程函数

       return RT_EOK;
}

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>
#include "led_driver.h"
#include "Uart3_8266.h"
#include "Beep.h"
#include <board.h>
#include "AHT30_driver.h"
#include "Key.h"
#include "Motor_driver.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static struct rt_led_device led1;

// 按键事件回调
static void key_handler(rt_uint8_t key_id)
{
    switch (key_id)
    {
    case 1:
        vibrator_on();  // 按下KEY1启动电机
        rt_kprintf("KEY1 action!\n");
        break;
    case 2:
        vibrator_off(); // 按下KEY2停止电机
        rt_kprintf("KEY2 action!\n");
        break;
    case 3:
        rt_kprintf("KEY3 action!\n");
        break;
    case 4:
        rt_kprintf("KEY4 action!\n");
        break;
    }
}

int main(void)
{
        // 初始化按键
        if (key_init() != RT_EOK)
        {
            rt_kprintf("Key init failed!\n");
            return -RT_ERROR;
        }
        // 注册按键回调函数
        key_set_callback(key_handler);

        //初始化震动电机
        vibrator_init();

        /* 创建 AHT30 线程 */
        aht30_thread_create();

        /* 启动 PWM 蜂鸣器线程 */
        //if (pwm_beep_start() != RT_EOK)
        //{
            //LOG_E("Failed to start PWM beep thread");
        //}

        /* 初始化LED并启动控制线程 */
        if (rt_led_init_with_thread(&led1, GET_PIN(A, 4), PIN_HIGH, 512, 20, 10) != RT_EOK) {
            return -RT_ERROR;
        }
        //esp8266收发接收线程函数
        CreatUart3TestEntry();

       return RT_EOK;
}

#include <rtthread.h>
#include <lvgl.h>
#include "lv_port_disp.h"
#include "AHT30_driver.h"
#include "Key.h"
#include "Motor_driver.h"
#include "led_driver.h"
#include "Uart3_8266.h"
#include "Beep.h"
#include <drv_common.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LV_THREAD_STACK_SIZE 4096

static void lvgl_thread_entry(void *parameter)
{
    while (1) {
        lv_timer_handler();
        rt_thread_mdelay(5);
    }
}

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
    if (rt_led_init_with_thread(&led1, GET_PIN(A, 4), PIN_HIGH, 512, 20, 10) != RT_EOK)
    {
        return -RT_ERROR;
    }

    // 初始化按键
    if (key_init() != RT_EOK)
    {
         rt_kprintf("Key init failed!\n");
         return -RT_ERROR;
    }
    // 注册按键回调函数
    key_set_callback(key_handler);

    //初始化震动电机
    //vibrator_init();

    /* 创建 AHT30 线程 */
    //aht30_thread_create();

    /* 启动 PWM 蜂鸣器线程 */
    if (pwm_beep_start() != RT_EOK)
       {
         LOG_E("Failed to start PWM beep thread");
       }

   //esp8266收发接收线程函数
       CreatUart3TestEntry();

    lv_init();
    lv_port_disp_init();

    // 创建一个显示"Hello World"的标签
    lv_obj_t * label = lv_label_create(lv_scr_act()); // 创建标签对象
    lv_label_set_text(label, "Hello World");          // 设置文本
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);      // 居中显示
    //创建刷新线程
    rt_thread_t tid = rt_thread_create("lvgl", lvgl_thread_entry, RT_NULL, 2048, 10, 10);
    rt_thread_startup(tid);



    // 主循环
    while(1)
    {
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}

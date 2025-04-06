#include "led_driver.h"
#include <rtdbg.h>

/* LED线程入口函数,在这里改变灯的逻辑 */
static void led_thread_entry(void *parameter)
{
    struct rt_led_device *led = (struct rt_led_device *)parameter;
    while (1) {
        rt_thread_mdelay(1000);
        rt_led_off(led);
    }
}

/* 带线程初始化的LED初始化函数 */
int rt_led_init_with_thread(struct rt_led_device *led,
                           rt_base_t pin,
                           rt_uint8_t active_level,
                           rt_uint32_t thread_stack_size,
                           rt_uint8_t thread_priority,
                           rt_uint8_t thread_timeslice)
{
    /*初始化硬件 */
    if (rt_led_init(led, pin, active_level) != RT_EOK) {
        return -RT_ERROR;
    }

    /*创建LED控制线程 */
    led->thread = rt_thread_create("led_ctrl",
                                 led_thread_entry,
                                 led,
                                 thread_stack_size,
                                 thread_priority,
                                 thread_timeslice);
    if (led->thread == RT_NULL) {
        return -RT_ERROR;
    }

    /*启动线程 */
    rt_thread_startup(led->thread);
    rt_kprintf("LED Thread create Success!\n");

    return RT_EOK;
}

/* 基础LED初始化 */
int rt_led_init(struct rt_led_device *led, rt_base_t pin, rt_uint8_t active_level)
{
    if (led == RT_NULL) return -RT_ERROR;

    led->pin = pin;
    led->active = active_level;
    led->thread = RT_NULL;  // 线程句柄初始化为空

    rt_pin_mode(led->pin, PIN_MODE_OUTPUT);
    rt_led_off(led);

    return RT_EOK;
}

/* 关闭 LED */
void rt_led_off(struct rt_led_device *led)
{
    rt_pin_write(led->pin, led->active);
}

/* 点亮 LED */
void rt_led_on(struct rt_led_device *led)
{
    rt_pin_write(led->pin, !led->active);
}

/* 翻转 LED 状态 */
void rt_led_toggle(struct rt_led_device *led)
{
    rt_base_t level = rt_pin_read(led->pin);
    rt_pin_write(led->pin, !level);
}

#ifndef __LED_DRIVER_H__
#define __LED_DRIVER_H__

#include <rtthread.h>
#include <rtdevice.h>

extern volatile uint8_t led_falg;

/* 定义 LED 对象结构体 */
struct rt_led_device
{
    rt_base_t pin;      // 引脚编号
    rt_uint8_t active;  // 激活电平 (PIN_HIGH/PIN_LOW)
    rt_thread_t thread;
};

int rt_led_init_with_thread(struct rt_led_device *led,
                           rt_base_t pin,
                           rt_uint8_t active_level,
                           rt_uint32_t thread_stack_size,
                           rt_uint8_t thread_priority,
                           rt_uint8_t thread_timeslice);
/* API 接口声明 */
int rt_led_init(struct rt_led_device *led, rt_base_t pin, rt_uint8_t active_level);
void rt_led_on(struct rt_led_device *led);
void rt_led_off(struct rt_led_device *led);
void rt_led_toggle(struct rt_led_device *led);

#endif /* __LED_DRIVER_H__ */

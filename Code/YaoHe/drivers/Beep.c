#include "Beep.h"

#define PWM_DEV_NAME "pwm2"
#define PWM_DEV_CHAN 2

static struct rt_device_pwm *pwm_dev;
#define BEEP    GET_PIN(A, 1)
// 蜂鸣器参数
static rt_uint32_t beep_freq = 2700;  // 默认 1kHz
static rt_uint32_t beep_duty = 50;    // 默认 50% 占空比

/* PWM 初始化 */
static int pwm_init(void)
{
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("PWM device %s not found!", PWM_DEV_NAME);
        return -RT_ERROR;
    }
    return RT_EOK;
}

/* PWM 线程入口函数 */
static void pwm_thread_entry(void *parameter)
{
    rt_uint8_t retry_count = 0;
    while (pwm_init() != RT_EOK)
    {
        if (retry_count++ > 5)
        {
           rt_kprintf("PWM init failed after 5 retries!\n");
           return;
        }
        rt_thread_mdelay(1000);
    }

    rt_pwm_enable(pwm_dev, PWM_DEV_CHAN); // 启用 PWM

    if (beep_freq == 0) beep_freq = 1000;  // 默认 1kHz
    rt_uint32_t period = 1000000 / beep_freq;
    rt_uint32_t pulse = (period * beep_duty) / 100;

    while (1)
    {
        rt_pin_write(BEEP,PIN_LOW);
        rt_thread_mdelay(1000);
        // 响 1ms
//        rt_pwm_set(pwm_dev, PWM_DEV_CHAN, period, pulse);
//        rt_thread_mdelay(1);
//
//        // 静音 1ms
//        rt_pwm_set(pwm_dev, PWM_DEV_CHAN, period, 0);
//        rt_thread_mdelay(1);

    }
}

/* 创建并启动 PWM 线程 */
int pwm_beep_start(void)
{
    rt_thread_t tid = rt_thread_create(
        "pwm_beep",           // 线程名称
        pwm_thread_entry,     // 线程入口函数
        RT_NULL,              // 参数
        512,                  // 栈大小
        10,                   // 优先级
        20                    // 时间片
    );

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    else
    {
        rt_kprintf("Failed to create PWM thread!");
        return -RT_ERROR;
    }
}

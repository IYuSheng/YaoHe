#include "Beep.h"
#include <rtdevice.h>

#define BEEP_PIN GET_PIN(A, 1)  // 使用RT-Thread标准引脚定义
#define BEEP_FREQ 500           // 目标频率1000Hz

static rt_timer_t beep_timer = RT_NULL;  // 定时器控制块
static rt_uint8_t current_state = PIN_LOW; // 当前引脚状态

/* 定时器回调函数（实现精准翻转） */
static void beep_toggle(void *param) {
    current_state = !current_state;
    rt_pin_write(BEEP_PIN, current_state);
}

/* 初始化蜂鸣器硬件 */
static int beep_init(void) {
    rt_pin_mode(BEEP_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(BEEP_PIN, PIN_LOW);

    // 创建软件定时器（回调函数在定时器线程的上下文中执行）
    beep_timer = rt_timer_create(
        "beep_tmr",
        beep_toggle,
        RT_NULL,
        RT_TICK_PER_SECOND / BEEP_FREQ / 2,  // 半周期
        RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER  // 使用软件定时器
    );
    return beep_timer ? RT_EOK : -RT_ERROR;
}

/* 蜂鸣器控制线程 */
static void beep_thread_entry(void *param) {
    if (beep_init() != RT_EOK) {
        rt_kprintf("BEEP init failed!\n");
        return;
    }

    while (1) {
        if (Beep_Flag)
        {
            rt_timer_start(beep_timer);  // 启动定时器翻转
        }
        else
        {
            rt_timer_stop(beep_timer);   // 停止定时器
            rt_pin_write(BEEP_PIN, PIN_LOW); // 确保关闭
        }
        rt_thread_mdelay(100); // 避免空转，降低CPU占用
    }
}

/* 创建蜂鸣器线程 */
int pwm_beep_start(void) {
    rt_thread_t tid = rt_thread_create(
        "beep", beep_thread_entry, RT_NULL,
        1024,
        8,
        20
    );
    if (tid != RT_NULL) {
        rt_kprintf("BEEP Start OK!\n");
        return rt_thread_startup(tid);
    } else {
        rt_kprintf("BEEP Start Failed!\n");
        return -RT_ERROR;
    }
}

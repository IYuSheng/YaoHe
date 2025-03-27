#include "Motor_driver.h"

// 初始化PA2为推挽输出
int vibrator_init(void)
{
    rt_pin_mode(VIBRATOR_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(VIBRATOR_PIN, PIN_LOW);  // 默认停止
    rt_kprintf("[Vibrator] PA2 initialized (Low by default)\n");
    return RT_EOK;
}

// 启动电机（输出高电平）
void vibrator_on(void)
{
    rt_pin_write(VIBRATOR_PIN, PIN_HIGH);
    rt_kprintf("[Vibrator] ON\n");
}

// 停止电机（输出低电平）
void vibrator_off(void)
{
    rt_pin_write(VIBRATOR_PIN, PIN_LOW);
    rt_kprintf("[Vibrator] OFF\n");
}

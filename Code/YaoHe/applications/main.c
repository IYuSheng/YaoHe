#include <rtthread.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define LED_PIN     GET_PIN(C, 0)  // 根据 BSP 定义引脚


int main(void)
{
    /* 初始化 LED 引脚为推挽输出 */
        rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT);

    int count = 1;

    while (count++)
    {
        rt_pin_write(LED_PIN, PIN_HIGH);  // 点亮
        rt_thread_mdelay(500);
        rt_pin_write(LED_PIN, PIN_LOW);   // 熄灭
        rt_thread_mdelay(500);
        LOG_D("Hello RT-Thread!");
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}

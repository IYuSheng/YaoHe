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
#include "drv_spi_lcd.h"
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

//void ugui_demo()
//{
//    UG_GUI* gui = UG_GetGui();
//
//    /* 创建按钮 */
//    UG_ButtonCreate(gui, BTN_ID_0, gui, 50, 100, 150, 140,
//                   OBJ_STATE_NORMAL, "Click Me!",
//                   FONT_16B, C_WHITE, C_BLUE, C_GRAY);
//
//    /* 创建进度条 */
//    UG_ProgressBarCreate(gui, PROG_ID_0, gui, 20, 180, 200, 30,
//                        OBJ_STATE_NORMAL, 100, 0,
//                        C_GREEN, C_DARK_GRAY);
//
//    while(1)
//    {
//        /* 更新进度条 */
//        static int val = 0;
//        UG_ProgressBarSetValue(gui, PROG_ID_0, val++ % 100);
//        rt_thread_mdelay(50);
//    }
//}

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
    //LED灯
    if (rt_led_init_with_thread(&led1, GET_PIN(A, 4), PIN_HIGH, 512, 20, 10) != RT_EOK)
    {
        return -RT_ERROR;
    }
    ILI9341_SPI_Init();
    ILI9341_Init();
    ILI9341_Fill_Screen(BLUE);
    // 绘制红色圆角矩形：位置(50,50) 尺寸200x120 圆角半径10 边框厚度3
    ILI9341_Draw_Rounded_Rectangle(20, 50, 150, 120, PURPLE, 10, 10);

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

       return RT_EOK;
}

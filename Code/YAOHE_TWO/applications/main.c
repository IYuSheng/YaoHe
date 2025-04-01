#include <rtthread.h>
#include <lvgl.h>
#include "lv_port_disp.h"

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



int main(void)
{
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

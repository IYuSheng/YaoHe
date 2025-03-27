#include <Key.h>

// 按键引脚定义
static rt_uint8_t key_pins[] = {
    KEY1_PIN, KEY2_PIN, KEY3_PIN, KEY4_PIN
};

static key_event_cb user_cb = RT_NULL;

// 按键检测线程入口
static void key_thread_entry(void *param)
{
    rt_uint8_t last_state[4] = {1, 1, 1, 1}; // 默认高电平（未按下）

    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            rt_uint8_t current = rt_pin_read(key_pins[i]);

            // 检测下降沿（按下事件）
            if (last_state[i] == 1 && current == 0)
            {
                if (user_cb != RT_NULL)
                {
                    user_cb(i + 1); // 触发回调
                }
            }

            last_state[i] = current; // 更新状态
        }
        rt_thread_mdelay(10); // 10ms 扫描间隔
    }
}

// 初始化按键引脚
int key_init(void)
{
    for (int i = 0; i < 4; i++)
    {
        rt_pin_mode(key_pins[i], PIN_MODE_INPUT_PULLUP); // 上拉输入
    }

    // 创建按键检测线程
    rt_thread_t tid = rt_thread_create(
        "key",
        key_thread_entry,
        RT_NULL,
        512,
        10,  // 优先级
        20
    );

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
        return RT_EOK;
    }
    return -RT_ERROR;
}

// 注册回调函数
void key_set_callback(key_event_cb cb)
{
    user_cb = cb;
}

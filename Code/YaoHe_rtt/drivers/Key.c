#include <Key.h>

// 按键引脚定义
static rt_uint8_t key_pins[] = {
    KEY1_PIN, KEY2_PIN, KEY3_PIN, KEY4_PIN, KEY_1_PIN, KEY_2_PIN, KEY_3_PIN, KEY_PIN
};

static key_event_cb user_cb = RT_NULL;
static rt_uint8_t key_state = 0; // KEY_PIN 的状态

// 按键状态结构体
typedef struct {
    rt_uint8_t physical_state;  // 当前物理状态 (1:释放, 0:按下)
    rt_uint8_t last_state;      // 上次检测状态
    rt_uint8_t registered;      // 是否已注册按下事件(仅对按键1-4)
} KeyState;

static void key_thread_entry(void *param)
{
    KeyState keys[7] = {0};

    // 初始化状态
    for (int i = 0; i < 7; i++) {
        keys[i].physical_state = rt_pin_read(key_pins[i]);
        keys[i].last_state = keys[i].physical_state;
    }

    // 将 KEY_PIN 拉高
    rt_pin_mode(KEY_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(KEY_PIN, 1);

    while (1) {
        // 读取 KEY_PIN 的状态
        key_state = rt_pin_read(KEY_PIN);

        for (int i = 0; i < 7; i++) {
            // 读取当前物理状态（反逻辑：1=释放，0=按下）
            keys[i].physical_state = rt_pin_read(key_pins[i]);

            /* 按键1-4：下降沿触发（按下时触发）*/
            if (i < 4) {
                if (keys[i].physical_state == 0 &&
                    keys[i].last_state == 1 &&
                    !keys[i].registered)
                {
                    keys[i].registered = 1;  // 标记已注册按下
                    if (user_cb) user_cb(i+1);
                }
                // 释放时重置注册标记
                if (keys[i].physical_state == 1) {
                    keys[i].registered = 0;
                }
            }
            /* 按键5-7：上升沿触发（释放时触发）*/
            else {
                // 只有在 KEY_PIN 拉高时才处理按键5-7的事件
                if (key_state == 1) { // KEY_PIN 拉高
                    if (keys[i].physical_state == 1 &&
                        keys[i].last_state == 0)
                    {
                        if (user_cb) user_cb(i+1);
                    }
                }
            }

            keys[i].last_state = keys[i].physical_state;
        }
        rt_thread_mdelay(20);  // 更快的5ms扫描周期
    }
}

// 初始化按键引脚
int key_init(void)
{
    for (int i = 0; i < 7; i++) // 初始化所有7个按键引脚
    {
        rt_pin_mode(key_pins[i], PIN_MODE_INPUT_PULLUP); // 上拉输入
    }

    // 创建按键检测线程
    rt_thread_t tid = rt_thread_create(
        "key",
        key_thread_entry,
        RT_NULL,
        1024, // 增加栈大小到 1024
        8,    // 优先级
        20    // 时间片
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

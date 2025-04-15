#include <rtthread.h>
#include <lvgl.h>
#include "lv_port_disp.h"
#include "AHT30_driver.h"
#include "Key.h"
#include "Motor_driver.h"
#include "led_driver.h"
#include "Uart3_8266.h"
#include "Beep.h"
#include <drv_common.h>
#include "drv_spi_lcd.h"
#include <time.h>
#include <sys/time.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static struct rt_semaphore lvgl_sem;

volatile uint8_t Beep_Flag;
volatile uint8_t Yao_flag;
volatile uint8_t new_win_flag;
volatile uint8_t led_falg;
volatile int Yao_State;//代表本次药成功服用成功的标志位

char Name[32];
char Time[32];
char Box[32];
char Number[32];
char LED[32];
char Shake[32];
char Sound[32];

extern volatile uint16_t T,H;
extern CommandData Cmd_data;

// 定义全局时间变量
static time_t sys_time = 0;  // 系统时间（秒）
static struct tm *timeinfo = RT_NULL;
static lv_obj_t *label1 = RT_NULL; // 明日任务标签
static lv_obj_t *label2 = RT_NULL; // 今日任务标签

static lv_obj_t *list_win = RT_NULL;//  List窗口
static lv_obj_t *user_win = RT_NULL; // User窗口


static struct rt_led_device led1;
// 按钮指针
static lv_obj_t *btn1, *btn2;

// 按键事件处理函数
static void key_handler(rt_uint8_t key_id)
{
    static bool screen_on = true; // 用于记录当前屏幕状态，默认为开
    static bool t_flag = false;

    switch (key_id)
    {
    case 1:
        t_flag = !t_flag;
        if (t_flag)
        {
            if(atoi(LED) == 1)
            {
                led_falg = 1;
            }
            else
            {
                led_falg = 0;
            }

            if(atoi(Shake) == 1)
            {
               vibrator_on();
            }
            else
            {
                vibrator_off();
            }

            if(atoi(Sound) == 1)
            {
                Beep_Flag = 1;
            }
            else
            {
                Beep_Flag = 0;
            }
            Yao_flag = 1;
        }
        else
        {
            Beep_Flag = 0;
            led_falg = 0;
            Yao_flag = 2;
            vibrator_off();
        }
        break;
    case 2:
        if (btn2) lv_event_send(btn2, LV_EVENT_CLICKED, NULL);
        break;
    case 3:
        if (btn1) lv_event_send(btn1, LV_EVENT_CLICKED, NULL);
        break;
    case 4:
        // 切换屏幕状态
        screen_on = !screen_on;
        if (screen_on)
        {
            ILI9341_Wake(); // 唤醒屏幕
        }
        else
        {
            ILI9341_Sleep(); // 熄屏
        }
        break;
    case 5:
        rt_kprintf("KEY_2 action!\n");

        if (atoi(Box) == 2) // 检查 Number 是否为空
        {
            int num = atoi(Number); // 将字符串转换为整数
            num--; // 减少数值
            if (num < 0) num = 0; // 确保不会小于0
            snprintf(Number, sizeof(Number), "%d", num); // 将结果保存回数组
        }
        break;
    case 6:
        rt_kprintf("KEY_1 action!\n");

        if (atoi(Box) == 1) // 检查 Number 是否为空
        {
            int num = atoi(Number); // 将字符串转换为整数
            num--; // 减少数值
            if (num < 0) num = 0; // 确保不会小于0
            snprintf(Number, sizeof(Number), "%d", num); // 将结果保存回数组
        }
        break;
    case 7:
        rt_kprintf("KEY_3 action!\n");

        if (atoi(Box) == 3) // 检查 Number 是否为空
        {
            int num = atoi(Number); // 将字符串转换为整数
            num--; // 减少数值
            if (num < 0) num = 0; // 确保不会小于0
            snprintf(Number, sizeof(Number), "%d", num); // 将结果保存回数组
        }
        break;
    }
}

/* User窗口函数 */
static void create_user_window(lv_obj_t *parent)
{

    // 强制删除旧窗口
    if (user_win != RT_NULL) {
        lv_obj_del(user_win);
        user_win = RT_NULL;
    }

    // 创建窗口容器
    user_win = lv_obj_create(parent);
    lv_obj_set_size(user_win, LV_PCT(74), LV_PCT(70));
    lv_obj_align(user_win, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // 窗口样式（与List窗口相同）
    lv_obj_set_style_border_width(user_win, 4, 0);
    lv_obj_set_style_border_color(user_win, lv_color_hex(0xED9121), 0);
    lv_obj_set_style_border_opa(user_win, LV_OPA_100, 0);
    lv_obj_set_style_radius(user_win, 12, 0);
    lv_obj_set_style_bg_color(user_win, lv_color_hex(0xF5DEB3), 0);
    lv_obj_set_scrollbar_mode(user_win, LV_SCROLLBAR_MODE_OFF);

    // 添加标题
    lv_obj_t *title = lv_label_create(user_win);
    lv_label_set_text(title, "User Info");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(title, lv_color_make(0xFF, 0x80, 0x00), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, -10);

    // 添加Name标签
    lv_obj_t *name_label = lv_label_create(user_win);
    lv_label_set_text(name_label, "Name: Old Man");
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_22, 0);
    // 设置Name标签的颜色
    lv_obj_set_style_text_color(name_label, lv_color_make(0xE3, 0xCF, 0x57), 0);
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 30);

    // 添加UID标签
    lv_obj_t *uid_label = lv_label_create(user_win);
    lv_label_set_text(uid_label, "UID: 1010");
    lv_obj_set_style_text_font(uid_label, &lv_font_montserrat_22, 0);
    // 设置UID标签的颜色
    lv_obj_set_style_text_color(uid_label, lv_color_make(0xDA, 0xA5, 0x69), 0);
    lv_obj_align(uid_label, LV_ALIGN_TOP_MID, 0, 70);

    // 添加Guardian标签
    lv_obj_t *guardian_label = lv_label_create(user_win);
    lv_label_set_text(guardian_label, "Guardian: 1145141919");
    lv_obj_set_style_text_font(guardian_label, &lv_font_montserrat_22, 0);
    // 设置Guardian标签的颜色
    lv_obj_set_style_text_color(guardian_label, lv_color_make(0xFF, 0x61, 0x03), 0);
    lv_obj_align(guardian_label, LV_ALIGN_TOP_MID, 0, 110);
}

static void delete_user_windows()
{
    if(user_win != RT_NULL) {
        lv_obj_del(user_win);
        user_win = RT_NULL;
    }
}

/* 新增List窗口函数 */
static void create_list_window(lv_obj_t *parent)
{
    // 创建窗口容器
    list_win = lv_obj_create(parent);
    lv_obj_set_size(list_win, LV_PCT(100), LV_PCT(70));
    lv_obj_align(list_win, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    /* 新增边框样式设置 */
    lv_obj_set_style_border_width(list_win, 4, 0);                  // 边框宽度 3 像素
    lv_obj_set_style_border_color(list_win, lv_color_hex(0x708069), 0); // 使用蓝色系边框
    lv_obj_set_style_border_opa(list_win, LV_OPA_100, 0);           // 不透明度 100%
    lv_obj_set_style_radius(list_win, 12, 0);                       // 设置圆角半径 12 像素

    // 原有样式设置
    lv_obj_set_style_bg_color(list_win, lv_color_hex(0xFFFFCD), 0); // 浅蓝色背景
    lv_obj_set_scrollbar_mode(list_win, LV_SCROLLBAR_MODE_OFF);     // 禁用滚动条

    // 添加标题
    lv_obj_t *title = lv_label_create(list_win);
    lv_label_set_text(title, "Medicine List");
    lv_obj_set_style_text_color(title, lv_color_make(0xFF, 0x61, 0x03), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_30, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, -10);

    // 添加药仓1信息标签
    lv_obj_t *medicine_label1 = lv_label_create(list_win);
    lv_label_set_text(medicine_label1, "1#\n"
                                "Med:Yao1\n"
                                "Daily:2 times\n"
                                "Stock: 10\n"
                                "Days: Mon-Fri\n"
                                "Time:\n"
                                "13:30, 17:00");
    lv_obj_set_style_text_font(medicine_label1, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(medicine_label1, lv_color_make(0xEB, 0x8E, 0x55), 0);
    lv_obj_align(medicine_label1, LV_ALIGN_TOP_LEFT, -10, 20);

    // 添加药仓2信息标签
    lv_obj_t *medicine_label2 = lv_label_create(list_win);
    lv_label_set_text(medicine_label2, "2#\n"
                                "Med:Yao2\n"
                                "Daily:3 times\n"
                                "Stock: 15\n"
                                "Days: Mon-Sat\n"
                                "Time:\n"
                                "09:00, 15:00");
    lv_obj_set_style_text_font(medicine_label2, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(medicine_label2, lv_color_make(0xE3, 0xCF, 0x57), 0);
    lv_obj_align(medicine_label2, LV_ALIGN_TOP_RIGHT, -90, 20);

    // 添加药仓3信息标签
    lv_obj_t *medicine_label3 = lv_label_create(list_win);
    lv_label_set_text(medicine_label3, "3#\n"
                                "Med:Yao3\n"
                                "Daily:1 times\n"
                                "Stock: 5\n"
                                "Days: Daily\n"
                                "Time:\n"
                                "08:00");
    lv_obj_set_style_text_font(medicine_label3, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(medicine_label3, lv_color_make(0x38, 0x5E, 0x0F), 0);
    lv_obj_align(medicine_label3, LV_ALIGN_TOP_RIGHT, 5, 20);
}
static void delete_list_windows(lv_obj_t *parent)
{
    if(list_win != RT_NULL)
    {
        lv_obj_del(list_win);
        list_win = RT_NULL;
    }
}

/* 事件回调函数 */
static void event_cb(lv_event_t *e)
{
    LV_LOG_USER("Button pressed!");
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    if (label) {
        const char *text = lv_label_get_text(label);
        if (strcmp(text, "User") == 0) {
            static uint8_t user_state = 0;
            user_state = !user_state;
            new_win_flag = user_state ? 3 : 4; // 3: 创建User窗口 4: 删除User窗口
            rt_kprintf("User button pressed!\n");
        } else if (strcmp(text, "List") == 0) {
            static uint8_t list_state = 0;
            list_state = !list_state;
            new_win_flag = list_state ? 1 : 2; // 原有逻辑保持不变
            rt_kprintf("List button pressed!\n");
        }
    }
}

void set_manual_time(void)
{
    struct tm time_info = {
        .tm_sec = 0,       // 秒
        .tm_min = 24,       // 分
        .tm_hour = 9,      // 小时
        .tm_mday = 15,      // 日
        .tm_mon = 3,       // 月（从 0 开始）
        .tm_year = 125,    // 年（从 1900 开始）
    };

    // 将时间转换为 Unix 时间戳
    sys_time = mktime(&time_info);
    if (sys_time != (time_t)(-1))
    {
        rt_kprintf("sys_time set to: %ld\n", sys_time);
    }
    else
    {
        rt_kprintf("Failed to set manual time\n");
    }
}

static lv_obj_t *time_label = RT_NULL;
static lv_obj_t *date_label = RT_NULL;

void lvgl_clock_start()
{
    /* 创建时钟容器 */
    lv_obj_t *time_date_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(time_date_obj, 220, 120); // 增加高度以容纳日期显示
    lv_obj_align(time_date_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(time_date_obj, LV_OBJ_FLAG_SCROLLABLE); // 禁用滚动

    // 设置时钟容器样式
    lv_obj_set_style_bg_color(time_date_obj, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(time_date_obj, LV_OPA_0, 0); // 背景透明
    lv_obj_set_style_border_width(time_date_obj, 0, 0); // 移除边框

    /* 时间显示部分 */
    lv_obj_t *time_obj = lv_obj_create(time_date_obj);
    lv_obj_set_size(time_obj, 200, 60);
    lv_obj_align(time_obj, LV_ALIGN_TOP_MID, -5, -15); // 时间显示在顶部中部
    lv_obj_clear_flag(time_obj, LV_OBJ_FLAG_SCROLLABLE);
    time_label = lv_label_create(time_obj);
    lv_obj_center(time_label);

    // 设置时间标签样式
    lv_obj_set_style_bg_color(time_obj, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(time_obj, LV_OPA_0, 0); // 背景透明
    lv_obj_set_style_border_width(time_obj, 0, 0); // 移除边框

    // 设置时间标签字体大小
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_46, 0);

    /* 星期显示部分 */
    lv_obj_t *weekday_obj = lv_obj_create(time_date_obj);
    lv_obj_set_size(weekday_obj, 100, 40);
    lv_obj_align_to(weekday_obj, time_obj, LV_ALIGN_OUT_RIGHT_MID, -105, 30); // 星期显示在时间的右侧
    lv_obj_clear_flag(weekday_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_text_color(weekday_obj, lv_color_make(0xFF, 0x61, 0x03), 0);
    lv_obj_t *weekday_label = lv_label_create(weekday_obj);
    lv_obj_center(weekday_label);

    // 设置星期标签样式
    lv_obj_set_style_bg_color(weekday_obj, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(weekday_obj, LV_OPA_0, 0); // 背景透明
    lv_obj_set_style_border_width(weekday_obj, 0, 0); // 移除边框

    // 设置星期标签字体大小和颜色
    lv_obj_set_style_text_font(weekday_label, &lv_font_montserrat_22, 0);
    //lv_obj_set_style_text_color(weekday_label, lv_color_make(0x43,0xCD,0x80), 0); // 设置字体颜色为浅蓝色
    lv_label_set_text(weekday_label, "Tuesday"); // 初始星期文本

    /* 日期显示部分 */
    lv_obj_t *date_obj = lv_obj_create(time_date_obj);
    lv_obj_set_size(date_obj, 200, 40);
    lv_obj_align(date_obj, LV_ALIGN_BOTTOM_MID, -55, -27); // 日期显示在底部中部
    lv_obj_clear_flag(date_obj, LV_OBJ_FLAG_SCROLLABLE);
    date_label = lv_label_create(date_obj);
    lv_obj_center(date_label);

    // 设置日期标签样式
    lv_obj_set_style_bg_color(date_obj, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(date_obj, LV_OPA_0, 0); // 背景透明
    lv_obj_set_style_border_width(date_obj, 0, 0); // 移除边框

    // 设置日期标签字体大小
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_16, 0);
}

// LVGL 线程
void lvgl_thread(void *parameter)
{
    snprintf(Number, sizeof(Number), "%d", 2);

    // 初始化 LVGL
    lv_init();
    lv_port_disp_init();

    // 注册按键回调函数
    key_set_callback(key_handler);

    // 创建一个屏幕
    lv_obj_t *screen = lv_scr_act();

    // 设置背景颜色
    lv_obj_set_style_bg_color(screen, lv_color_make(0xF0, 0xFF, 0xF0), 0); // 浅蓝色背景
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // 创建温度圆弧
    lv_obj_t *temp_arc = lv_arc_create(screen);
    lv_obj_set_size(temp_arc, 60, 60);
    lv_obj_align(temp_arc, LV_ALIGN_TOP_LEFT, 10, 80); // 定位到左上角
    lv_arc_set_range(temp_arc, 0, 100); // 设置圆弧范围为0到100
    lv_arc_set_value(temp_arc, 25); // 初始值为25
    lv_obj_set_style_arc_color(temp_arc, lv_color_make(0xFF, 0x00, 0x00), LV_PART_INDICATOR); // 设置前景弧颜色为蓝色
    lv_obj_set_style_arc_width(temp_arc, 6, LV_PART_INDICATOR); // 设置前景弧宽度为4
    lv_obj_set_style_arc_color(temp_arc, lv_color_make(0xE0, 0xE0, 0xE0), LV_PART_MAIN); // 设置背景弧颜色为浅灰色
    lv_obj_set_style_arc_width(temp_arc, 4, LV_PART_MAIN); // 设置背景弧宽度为4
    lv_obj_set_style_arc_rounded(temp_arc, true, LV_PART_INDICATOR); // 设置前景弧末端为圆形[^6^]

    // 创建温度标签
    lv_obj_t *temp_label = lv_label_create(screen);
    lv_label_set_text(temp_label, "25°C");
    lv_obj_align_to(temp_label, temp_arc, LV_ALIGN_CENTER, 0, 0); // 将标签居中对齐到圆弧
    lv_obj_set_style_text_color(temp_label, lv_color_make(0xFF, 0x00, 0x00), 0);

    // 创建温度英文标签
    lv_obj_t *temp_text_label = lv_label_create(screen);
    lv_label_set_text(temp_text_label, "Temper");
    lv_obj_align_to(temp_text_label, temp_arc, LV_ALIGN_OUT_BOTTOM_MID, 0, -10); // 定位到温度圆弧的下方
    lv_obj_set_style_text_color(temp_text_label, lv_color_make(0xFF, 0x00, 0x00), 0);

    // 创建湿度圆弧
    lv_obj_t *humi_arc = lv_arc_create(screen);
    lv_obj_set_size(humi_arc, 60, 60);
    lv_obj_align_to(humi_arc, temp_arc, LV_ALIGN_OUT_RIGHT_MID, 5, 0); // 定位到温度圆弧的右侧
    lv_arc_set_range(humi_arc, 0, 100); // 设置圆弧范围为0到100
    lv_arc_set_value(humi_arc, 50); // 初始值为50
    lv_obj_set_style_arc_color(humi_arc, lv_color_make(0x00, 0x00, 0xFF), LV_PART_INDICATOR); // 设置前景弧颜色为绿色
    lv_obj_set_style_arc_width(humi_arc, 6, LV_PART_INDICATOR); // 设置前景弧宽度为6
    lv_obj_set_style_arc_color(humi_arc, lv_color_make(0xE0, 0xE0, 0xE0), LV_PART_MAIN); // 设置背景弧颜色为浅灰色
    lv_obj_set_style_arc_width(humi_arc, 4, LV_PART_MAIN); // 设置背景弧宽度为4

    // 创建湿度标签
    lv_obj_t *humi_label = lv_label_create(screen);
    lv_label_set_text(humi_label, "50%");
    lv_obj_align_to(humi_label, humi_arc, LV_ALIGN_CENTER, 0, 0); // 将标签居中对齐到圆弧
    lv_obj_set_style_text_color(humi_label, lv_color_make(0x00, 0x00, 0xFF), 0);

    // 创建湿度英文标签
    lv_obj_t *humi_text_label = lv_label_create(screen);
    lv_label_set_text(humi_text_label, "Hum");
    lv_obj_align_to(humi_text_label, humi_arc, LV_ALIGN_OUT_BOTTOM_MID, 0, -10); // 定位到湿度圆弧的下方
    lv_obj_set_style_text_color(humi_text_label, lv_color_make(0x00, 0x00, 0xFF), 0);

    // 创建WiFi图标
    lv_obj_t *wifi_label = lv_label_create(screen);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI); // 使用 LVGL 自带的WiFi符号
    lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_22, 0);
    lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT, -70, 5); // 定位到右上角，偏移量为-40
    lv_obj_set_style_text_color(wifi_label, lv_color_make(0x29, 0x24, 0x21), 0);

    // 创建电量图标
    lv_obj_t *battery_label = lv_label_create(screen);
    lv_label_set_text(battery_label, LV_SYMBOL_BATTERY_3); // 使用 LVGL 自带的电池符号
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_22, 0);
    lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -5, 5); // 定位到右上角，偏移量为-5
    lv_obj_set_style_text_color(battery_label, lv_color_make(0x00, 0xFF, 0x00), 0); // 设置为绿色

    // 电池电量百分比文本
    lv_obj_t *battery_text = lv_label_create(screen);
    lv_label_set_text(battery_text, "73%");
    lv_obj_align_to(battery_text, battery_label, LV_ALIGN_OUT_LEFT_MID, -5, 0); // 对齐到电池图标的左侧
    lv_obj_set_style_text_color(battery_text, lv_color_make(0x38, 0x5E, 0x0F), 0); // 设置文本颜色为黑色

    // 创建声图标
    lv_obj_t *bell_label = lv_label_create(screen);
    lv_label_set_text(bell_label, LV_SYMBOL_BELL); // 使用 LVGL 自带的bell符号
    lv_obj_set_style_text_font(bell_label, &lv_font_montserrat_22, 0);
    lv_obj_align_to(bell_label, wifi_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10); // 定位到WiFi图标的下方
    lv_obj_set_style_text_color(bell_label, lv_color_make(0xA0, 0xA0, 0xA0), 0);

    // 创建光图标
    lv_obj_t *eye_label = lv_label_create(screen);
    lv_label_set_text(eye_label, LV_SYMBOL_EYE_OPEN); // 使用 LVGL 自带的eyeopen符号
    lv_obj_set_style_text_font(eye_label, &lv_font_montserrat_22, 0);
    lv_obj_align_to(eye_label, bell_label, LV_ALIGN_OUT_RIGHT_MID, 15, 0); // 定位到bell图标的右侧
    lv_obj_set_style_text_color(eye_label, lv_color_make(0xA0, 0xA0, 0xA0), 0);

    // 创建触图标
    lv_obj_t *charge_label = lv_label_create(screen);
    lv_label_set_text(charge_label, LV_SYMBOL_CHARGE); // 使用 LVGL 自带的charge符号
    lv_obj_set_style_text_font(charge_label, &lv_font_montserrat_22, 0);
    lv_obj_align_to(charge_label, eye_label, LV_ALIGN_OUT_RIGHT_MID, 15, 0); // 定位到eyeopen图标的右侧
    lv_obj_set_style_text_color(charge_label, lv_color_make(0xA0, 0xA0, 0xA0), 0);

    /* 添加时钟组件 */
    lvgl_clock_start();
    set_manual_time();

    // 创建基础样式和警告样式
    static lv_style_t style_base;
    lv_style_init(&style_base);
    lv_style_set_bg_color(&style_base, lv_palette_main(LV_PALETTE_GREEN)); // 背景颜色：浅绿色
    lv_style_set_border_color(&style_base, lv_palette_darken(LV_PALETTE_GREEN, 3)); // 边框颜色：深化的浅绿色
    lv_style_set_border_width(&style_base, 3);                                      // 边框宽度：3像素
    lv_style_set_radius(&style_base, 12);                                           // 圆角半径：12像素
    lv_style_set_shadow_width(&style_base, 10);                                     // 阴影宽度：10像素
    lv_style_set_shadow_opa(&style_base, LV_OPA_50);                                // 阴影透明度：50%
    lv_style_set_text_color(&style_base, lv_palette_darken(LV_PALETTE_GREEN, 4));
    lv_style_set_width(&style_base, 175);                                            // 对象宽度：175像素
    lv_style_set_height(&style_base, 65);                                            // 对象高度：65像素

    static lv_style_t style_warning;
    lv_style_init(&style_warning);
    lv_style_set_bg_color(&style_warning, lv_palette_main(LV_PALETTE_YELLOW));      // 背景颜色：黄色
    lv_style_set_border_color(&style_warning, lv_palette_darken(LV_PALETTE_YELLOW, 3)); // 边框颜色：深化的黄色
    lv_style_set_text_color(&style_warning, lv_palette_darken(LV_PALETTE_YELLOW, 4));    // 文本颜色：更深的黄色

    lv_obj_t *obj_warning = lv_obj_create(screen);                                  // 在屏幕上创建一个对象
    lv_obj_add_style(obj_warning, &style_base, 0);                                 // 应用基础样式（默认状态）
    // 禁用滚动条
    lv_obj_set_scrollbar_mode(obj_warning, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align_to(obj_warning, humi_arc, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_t *label_warning = lv_label_create(obj_warning);
    lv_label_set_text(label_warning, "Have taken");
    lv_obj_set_style_text_font(label_warning, &lv_font_montserrat_30, 0);
    lv_obj_center(label_warning); // 将标签居中对齐

    // 创建用户按键
    btn1 = lv_btn_create(screen);
    lv_obj_set_size(btn1, 80, 40);
    lv_obj_align(btn1, LV_ALIGN_RIGHT_MID, -5, 50);

    lv_obj_t *label_B1 = lv_label_create(btn1);
    lv_label_set_text(label_B1, "User");
    lv_obj_center(label_B1);
    lv_obj_set_style_text_font(label_B1, &lv_font_montserrat_22, 0);

    // 设置用户按钮样式
    lv_obj_set_style_bg_color(btn1, lv_palette_main(LV_PALETTE_ORANGE), 0);
    lv_obj_set_style_border_color(btn1, lv_palette_darken(LV_PALETTE_ORANGE, 3), 0);
    lv_obj_set_style_border_width(btn1, 3, 0);

    lv_obj_add_event_cb(btn1, event_cb, LV_EVENT_CLICKED, NULL);

    // 创建计划按键
    btn2 = lv_btn_create(screen);
    lv_obj_set_size(btn2, 80, 40);
    lv_obj_align(btn2, LV_ALIGN_RIGHT_MID, -5, 95);

    lv_obj_t *label_B2 = lv_label_create(btn2);
    lv_label_set_text(label_B2, "List");
    lv_obj_center(label_B2);
    lv_obj_set_style_text_font(label_B2, &lv_font_montserrat_22, 0);

    // 设置计划按钮样式
    lv_obj_set_style_bg_color(btn2, lv_palette_main(LV_PALETTE_INDIGO), 0);
    lv_obj_set_style_border_color(btn2, lv_palette_darken(LV_PALETTE_INDIGO, 3), 0);
    lv_obj_set_style_border_width(btn2, 3, 0);

    lv_obj_add_event_cb(btn2, event_cb, LV_EVENT_CLICKED, NULL);

    // 创建剩余时间英文标签
    lv_obj_t *countdown_label = lv_label_create(screen);
    lv_label_set_text(countdown_label, "Remaining:");
    lv_obj_align(countdown_label, LV_ALIGN_BOTTOM_LEFT, 10, -5);
    lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(countdown_label, lv_color_make(0x41, 0x69, 0xE1), LV_PART_MAIN); // 蓝色

    // 创建倒计时标签
    label1 = lv_label_create(screen);
    lv_label_set_text(label1, "Formalin:");
    lv_obj_align_to(label1, countdown_label, LV_ALIGN_OUT_TOP_LEFT, 0, -15);
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label1, lv_color_make(0xFF, 0x80, 0x00), LV_PART_MAIN); // 橙色

    label2 = lv_label_create(screen);
    lv_label_set_text(label2, "Today 15:00");
    lv_obj_align_to(label2, countdown_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(label2, lv_color_make(0x00, 0xC9, 0x57), LV_PART_MAIN); // 紫色

    // 创建#2号药仓标签
    lv_obj_t *medicine_label = lv_label_create(screen);
    lv_label_set_text(medicine_label, "#2 Medicine");
    lv_obj_align_to(medicine_label, label1, LV_ALIGN_OUT_TOP_LEFT, 0, -12);
    lv_obj_set_style_text_font(medicine_label, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(medicine_label, lv_color_make(0x00, 0x80, 0x00), LV_PART_MAIN); // 绿色

    // 创建需要吃多少粒的标签
    lv_obj_t *num_label = lv_label_create(screen);
    lv_label_set_text(num_label, "2 capsules");
    lv_obj_align_to(num_label, label1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_text_font(num_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(num_label, lv_color_make(0x00, 0x80, 0x00), LV_PART_MAIN); // 绿色


    rt_sem_release(&lvgl_sem); //释放信号量位置

    // 主循环
    while (1)
        {
            lv_task_handler();

            if (Yao_flag == 1)
            {
                lv_obj_add_style(obj_warning, &style_warning, 0);
                lv_obj_set_style_text_font(label_warning, &lv_font_montserrat_26, 0);
                lv_label_set_text(label_warning, "To be served");
                Yao_flag = 0;
            }
            else if (Yao_flag == 2)
            {
                lv_obj_add_style(obj_warning, &style_base, 0);
                lv_label_set_text(label_warning, "Have taken");
                lv_obj_set_style_text_font(label_warning, &lv_font_montserrat_30, 0);
                Yao_flag = 0;
            }

            if(atoi(Number) == 0)
            {
                Yao_State = 1;
            }
            else
            {
                Yao_State = 0;
            }

            /* 窗口状态处理 */
            switch (new_win_flag)
            {
                case 1:
                    create_list_window(lv_scr_act());
                    new_win_flag = 0;
                    break;
                case 2:
                    delete_list_windows(lv_scr_act());
                    new_win_flag = 0;
                    break;
                case 3:
                    create_user_window(lv_scr_act());
                    new_win_flag = 0;
                    break;
                case 4:
                    delete_user_windows();
                    new_win_flag = 0;
                    break;
            }

            static rt_uint32_t last_tick = 0;
            rt_uint32_t current_tick = rt_tick_get();
            // 每秒更新一次
            if (current_tick - last_tick >= 1000)
            {

                if(atoi(LED) == 1)
                {
                    lv_obj_set_style_text_color(eye_label, lv_color_make(0xFF, 0x99, 0x12), 0);
                }
                else
                {
                    lv_obj_set_style_text_color(eye_label, lv_color_make(0xA0, 0xA0, 0xA0), 0);
                }

                if(atoi(Shake) == 1)
                {
                    lv_obj_set_style_text_color(charge_label, lv_color_make(0x40, 0xE0, 0xD0), 0);
                }
                else
                {
                    lv_obj_set_style_text_color(charge_label, lv_color_make(0xA0, 0xA0, 0xA0), 0);
                }

                if(atoi(Sound) == 1)
                {
                    lv_obj_set_style_text_color(bell_label, lv_color_make(0xFF, 0xD7, 0x00), 0);
                }
                else
                {
                    lv_obj_set_style_text_color(bell_label, lv_color_make(0xA0, 0xA0, 0xA0), 0);
                }

                // 更新文本标签
                if (strlen(Name) > 0) {
                    char text[32];
                    snprintf(text, sizeof(text), "%s:", Name); // 拼接字符串
                    lv_obj_align_to(num_label, label1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
                    lv_label_set_text(label1, text);
                }

                if (strlen(Box) > 0) {
                    char text[32];
                    snprintf(text, sizeof(text), "#%s Medicine", Box); // 拼接字符串
                    lv_label_set_text(medicine_label, text);
                }
                if (strlen(Number) > 0) {
                    char text[32];
                    snprintf(text, sizeof(text), "%s capsules", Number); // 拼接字符串
                    lv_obj_align_to(num_label, label1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
                    lv_label_set_text(num_label, text);
                }

                // 更新温度圆弧
                lv_arc_set_value(temp_arc, T);
                char temp_str[10];
                snprintf(temp_str, sizeof(temp_str), "%d°C", T);
                lv_label_set_text(temp_label, temp_str);

                // 更新湿度圆弧
                lv_arc_set_value(humi_arc, H);
                char humi_str[10];
                snprintf(humi_str, sizeof(humi_str), "%d%%", H);
                lv_label_set_text(humi_label, humi_str);

                last_tick = current_tick;
                sys_time++;

                // 转换为本地时间结构体
                timeinfo = localtime(&sys_time);

                // 更新时间显示
                char time_str[20];
                strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
                lv_label_set_text(time_label, time_str);

                // 更新日期显示
                char date_str[20];
                strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);
                lv_label_set_text(date_label, date_str);

                // 计算距离目标的剩余时间
                struct tm target_tm = *timeinfo; // 复制当前时间结构体
                struct tm target_time = {0};

                // 使用 sscanf 解析 Time 字符串
                if (sscanf(Time, "%d:%d", &target_time.tm_hour, &target_time.tm_min) == 2) {
                    target_tm.tm_hour = target_time.tm_hour;
                    target_tm.tm_min = target_time.tm_min;
                    target_tm.tm_sec = 0;
                    target_tm.tm_isdst = -1; // 自动处理夏令时
                }

                // 转换为时间戳并自动处理跨天
                time_t target = mktime(&target_tm);

                // 如果当前时间已过今日目标时间，计算明日目标时间
                if (sys_time >= target) {
                    target += 24 * 3600; // 增加24小时
                    target_tm = *localtime(&target);
                    target_tm.tm_hour = 15;
                    target_tm.tm_min = 0;
                    target_tm.tm_sec = 0;
                    target_tm.tm_isdst = -1;
                    target = mktime(&target_tm);
                }

                // 计算剩余时间
                time_t remaining = target - sys_time;
                int hours = remaining / 3600;
                int minutes = (remaining % 3600) / 60;

                // 更新标签内容
                char countdown_str[30];
                if (sys_time < target) {
                    snprintf(countdown_str, sizeof(countdown_str), "%dh%02dm", hours, minutes);
                    lv_label_set_text(label2, countdown_str);
                } else {
                    lv_label_set_text(label2, "Time passed");
                }
            }
            rt_thread_mdelay(5); // 每 5ms 更新一次
        }
}

int main(void)
{
    rt_sem_init(&lvgl_sem, "lvgl_ready", 0, RT_IPC_FLAG_FIFO);

    if (rt_led_init_with_thread(&led1, GET_PIN(A, 4), PIN_HIGH, 512, 16, 10) != RT_EOK)
    {
        return -RT_ERROR;
    }

    // 初始化按键
    if (key_init() != RT_EOK)
    {
         rt_kprintf("Key init failed!\n");
         return -RT_ERROR;
    }

    // 创建 LVGL 线程
    rt_thread_t lvgl_tid = rt_thread_create("lvgl",
                                               lvgl_thread,
                                               RT_NULL,
                                               1024*4,
                                               5,
                                               5);

    if (lvgl_tid != RT_NULL)
    {
        rt_thread_startup(lvgl_tid);
    }
    else
    {
        rt_kprintf("Failed to create LVGL thread\n");
        return -RT_ERROR;
    }

    // 等待 LVGL 初始化完成（最多等待 8秒）
    if (rt_sem_take(&lvgl_sem, RT_TICK_PER_SECOND*8) == RT_EOK)
    {
        /* 创建 AHT30 线程 */
        aht30_thread_create();
        rt_thread_mdelay(1000);
        /* 启动蜂鸣器线程 */
        rt_thread_mdelay(500);
        //初始化震动电机
        vibrator_init();
        pwm_beep_start();
        rt_thread_mdelay(500);
        /* 创建 8266 线程 */
        CreatUart3TestEntry();

    }

    return RT_EOK;
}

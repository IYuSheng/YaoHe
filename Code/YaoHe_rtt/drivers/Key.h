#ifndef DRIVERS_KEY_H_
#define DRIVERS_KEY_H_
#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>

#define KEY2_PIN    GET_PIN(B, 3)   // PB3
#define KEY3_PIN    GET_PIN(B, 4)   // PB4
#define KEY1_PIN    GET_PIN(C, 12)  // PC12
#define KEY4_PIN    GET_PIN(A, 11)  // PA11

#define KEY_PIN    GET_PIN(C, 6)  // PC6

#define KEY_1_PIN    GET_PIN(A, 12)  // PA12
#define KEY_2_PIN    GET_PIN(C, 7)  // PC7
#define KEY_3_PIN    GET_PIN(B, 14)  // PB14

// 按键事件回调函数类型
typedef void (*key_event_cb)(rt_uint8_t key_id);

// 初始化按键
int key_init();
// 注册按键事件回调
void key_set_callback(key_event_cb cb);

#endif /* DRIVERS_KEY_H_ */

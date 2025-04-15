#ifndef DRIVERS_MOTOR_DRIVER_H_
#define DRIVERS_MOTOR_DRIVER_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>

#define VIBRATOR_PIN    GET_PIN(A, 2)

// 初始化震动电机GPIO
int vibrator_init(void);

// 直接控制电机状态
void vibrator_on(void);  // 高电平启动
void vibrator_off(void); // 低电平停止

#endif /* DRIVERS_MOTOR_DRIVER_H_ */

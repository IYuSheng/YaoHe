#ifndef DRIVERS_AHT30_DRIVER_H_
#define DRIVERS_AHT30_DRIVER_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>

volatile uint16_t T,H;

#define AHT30_I2C_ADDR        0x38  // 默认地址

rt_thread_t aht30_thread_create(void);

#endif /* DRIVERS_AHT30_DRIVER_H_ */

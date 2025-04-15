#ifndef DRIVERS_BEEP_H_
#define DRIVERS_BEEP_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_common.h>

extern volatile uint8_t Beep_Flag;

int pwm_beep_start(void);

#endif /* DRIVERS_BEEP_H_ */

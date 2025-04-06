#ifndef DRIVERS_UART3_8266_H_
#define DRIVERS_UART3_8266_H_

#include <rtthread.h>
#include <string.h>
#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern void CreatUart3TestEntry(void);
rt_size_t Uart3_GetReceivedData(char *buffer, rt_size_t size);
rt_err_t Uart3_WaitReceive(char *buffer, rt_size_t size, rt_int32_t timeout);
rt_err_t Uart3_SendData(const char *data);

#endif /* DRIVERS_UART3_8266_H_ */

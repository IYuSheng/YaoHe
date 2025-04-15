#ifndef DRIVERS_UART3_8266_H_
#define DRIVERS_UART3_8266_H_

#include <rtthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cJSON.h>

extern volatile uint16_t T,H;
extern volatile int Yao_State;
extern struct rt_messagequeue cmd_mq;

extern char Name[32];
extern char Time[32];
extern char Box[32];
extern char Number[32];
extern char LED[32];
extern char Shake[32];
extern char Sound[32];

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    int temp;
    int hum;
    int led;
    int shake;
    int sound;
    int bat;
    int Yao;
} SensorData;

typedef struct {
    char name[16];
    char time[16];
    char box[16];
    char number[16];
    char led[16];
    char shake[16];
    char sound[16];
} CommandData;


void CreatUart3TestEntry(void);
rt_size_t Uart3_GetReceivedData(char *buffer, rt_size_t size);
rt_err_t Uart3_WaitReceive(char *buffer, rt_size_t size, rt_int32_t timeout);
rt_err_t Uart3_SendData(const char *data);

#endif /* DRIVERS_UART3_8266_H_ */

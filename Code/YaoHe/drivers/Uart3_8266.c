#include "Uart3_8266.h"
#define SAMPLE_UART_NAME       "uart3"
#define MAX_RX_BUFFER_SIZE     256

/* 串口接收消息结构体*/
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
    char data[];
};

/* 全局变量 */
static rt_device_t serial;
static struct rt_messagequeue rx_mq;
static rt_sem_t rx_sem;  // 用于接收同步的信号量
static char received_data[MAX_RX_BUFFER_SIZE]; // 接收数据缓冲区
static rt_size_t received_len = 0; // 接收数据长度


static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
    if (result == -RT_EFULL) {
        rt_kprintf("message queue full！\n");
    }
    return result;
}

static void serial_thread_entry(void *parameter)
{
    char rx_buffer[MAX_RX_BUFFER_SIZE];
    while (1)
    {
        //从消息队列中获取接收事件
        struct rx_msg msg;
        rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        //从串口读取数据
        rt_size_t len = rt_device_read(msg.dev, 0, rx_buffer, sizeof(rx_buffer)-1);
        rx_buffer[len] = '\0'; // 添加字符串终止符
        rt_kprintf("%s\r\n", rx_buffer);
        //解析数据，保留有效数据
        char *payload = strchr(rx_buffer, ':');
        if (payload)
        {
            payload++;
            // 保存接收到的数据
            received_len = strlen(payload);
            strncpy(received_data, payload, sizeof(received_data)-1);
            received_data[received_len] = '\0';

            // 发送信号量通知数据到达
            rt_sem_release(rx_sem);

            //打印esp8266接收到的信息
            rt_kprintf("%s\r\n", payload);
        }
    }
}

void CreatUart3TestEntry(void)
{
    static char msg_pool[512];
    // 创建信号量
    rx_sem = rt_sem_create("rx_sem", 0, RT_IPC_FLAG_FIFO);

    /* 查找并配置串口设备 */
    serial = rt_device_find(SAMPLE_UART_NAME);
    if (!serial) {
        rt_kprintf("find %s failed!\n", SAMPLE_UART_NAME);
        return;
    }

    /* 初始化消息队列 */
    rt_mq_init(&rx_mq, "rx_mq",
               msg_pool,
               sizeof(struct rx_msg),
               sizeof(msg_pool),
               RT_IPC_FLAG_FIFO);

    /* 打开串口设备 */
    if (rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX) != RT_EOK) {
        rt_kprintf("open %s failed!\n", SAMPLE_UART_NAME);
        return;
    }

    /* 设置接收回调 */
    rt_device_set_rx_indicate(serial, uart_input);

    /* 创建并启动线程 */
    rt_thread_t thread = rt_thread_create("serial",
                                         serial_thread_entry,
                                         RT_NULL,
                                         1024,
                                         5,
                                         10);
    if (thread) {
        rt_thread_startup(thread);
    } else {
        rt_kprintf("Create thread failed!\n");
        return;
    }
    rt_thread_mdelay(1000);
    Uart3_SendData("AT+CWMODE?\r\n");
    rt_thread_mdelay(1000);
    Uart3_SendData("AT+CWMODE=2\r\n");
}

rt_err_t Uart3_SendData(const char *data) {
    rt_size_t len = strlen(data);
    if (len == 0) return -RT_ERROR;

    rt_err_t result = rt_device_write(serial, 0, data, len);
    if (result != len) {
        rt_kprintf("Send failed: %d/%d\r\n", result, len);
        return -RT_ERROR;
    }
    return RT_EOK;
}

/* 等待接收数据（带超时） */
rt_err_t Uart3_WaitReceive(char *buffer, rt_size_t size, rt_int32_t timeout) {
    // 等待信号量（数据到达）
    if (rt_sem_take(rx_sem, rt_tick_from_millisecond(timeout)) != RT_EOK) {
        return -RT_ETIMEOUT;
    }

    // 拷贝数据到用户缓冲区
    rt_size_t copy_len = received_len < size-1 ? received_len : size-1;
    strncpy(buffer, received_data, copy_len);
    buffer[copy_len] = '\0';

    return RT_EOK;
}

/* 获取最近接收的数据 ,非阻塞获取*/
rt_size_t Uart3_GetReceivedData(char *buffer, rt_size_t size) {
    if (received_len == 0) return 0;

    rt_size_t copy_len = received_len < size-1 ? received_len : size-1;
    strncpy(buffer, received_data, copy_len);
    buffer[copy_len] = '\0';

    return copy_len;
}

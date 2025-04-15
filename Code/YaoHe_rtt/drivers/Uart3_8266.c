#include "Uart3_8266.h"
#define SAMPLE_UART_NAME       "uart3"
#define MAX_RX_BUFFER_SIZE     1024

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
char rx_buffer[MAX_RX_BUFFER_SIZE];

static char json_buffer[MAX_RX_BUFFER_SIZE * 2];
static int json_buffer_len = 0;

CommandData Cmd_data = {0};

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));//发送给消息队列
    if (result == -RT_EFULL) {
        rt_kprintf("message queue full！\n");
    }
    return result;
}

static void parse_command(const char *json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        rt_kprintf("JSON Failed: %s\n", cJSON_GetErrorPtr());
        return;
    }

    // 解析各字段
    cJSON *item = cJSON_GetObjectItem(root, "name");
    if (item && cJSON_IsString(item)) strncpy(Cmd_data.name, item->valuestring, sizeof(Cmd_data.name)-1);

    item = cJSON_GetObjectItem(root, "time");
    if (item && cJSON_IsString(item)) strncpy(Cmd_data.time, item->valuestring, sizeof(Cmd_data.time)-1);

    item = cJSON_GetObjectItem(root, "box");
    if (item && cJSON_IsString(item)) strncpy(Cmd_data.box, item->valuestring, sizeof(Cmd_data.box)-1);

    item = cJSON_GetObjectItem(root, "number");
    if (item && cJSON_IsString(item)) strncpy(Cmd_data.number, item->valuestring, sizeof(Cmd_data.number)-1);

    item = cJSON_GetObjectItem(root, "led");
    if (item && cJSON_IsString(item)) strncpy(Cmd_data.led, item->valuestring, sizeof(Cmd_data.led)-1);

    item = cJSON_GetObjectItem(root, "shake");
    if (item && cJSON_IsString(item)) strncpy(Cmd_data.shake, item->valuestring, sizeof(Cmd_data.shake)-1);

    item = cJSON_GetObjectItem(root, "sound");
    if (item && cJSON_IsString(item)) strncpy(Cmd_data.sound, item->valuestring, sizeof(Cmd_data.sound)-1);

    // 将解析后的数据复制到外部变量
    strncpy(Name, Cmd_data.name, sizeof(Name)-1);
    strncpy(Time, Cmd_data.time, sizeof(Time)-1);
    strncpy(Box, Cmd_data.box, sizeof(Box)-1);
    strncpy(Number, Cmd_data.number, sizeof(Number)-1);
    strncpy(LED, Cmd_data.led, sizeof(LED)-1);
    strncpy(Shake, Cmd_data.shake, sizeof(Shake)-1);
    strncpy(Sound, Cmd_data.sound, sizeof(Sound)-1);

    cJSON_Delete(root);
}

static void serial_thread_entry(void *parameter)
{
    while (1)
    {
        struct rx_msg msg;
        rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        rt_size_t len = rt_device_read(msg.dev, 0, rx_buffer, sizeof(rx_buffer)-1);
        rx_buffer[len] = '\0';

        // 将接收到的所有数据追加到缓冲区（无论是否包含{}）
        strncat(json_buffer, rx_buffer, sizeof(json_buffer)-json_buffer_len-1);
        json_buffer_len += len;
        json_buffer[json_buffer_len] = '\0';
        rt_kprintf("AT: %s\n", rx_buffer);

        // 改进的JSON解析逻辑
        while (1) {
            char *start = strchr(json_buffer, '{');
            if (!start) break;  // 没有找到起始符

            // 括号匹配查找结束符
            int brace_depth = 0;
            char *end = NULL;
            for (char *p = start; *p != '\0'; p++) {
                if (*p == '{') brace_depth++;
                if (*p == '}') brace_depth--;

                if (brace_depth == 0) {
                    end = p;
                    break;
                }
            }

            if (!end) break;  // 没有完整JSON

            // 提取并处理JSON
            int json_len = end - start + 1;
            char temp_json[json_len + 1];
            strncpy(temp_json, start, json_len);
            temp_json[json_len] = '\0';

            parse_command(temp_json);

            // 移动缓冲区内容（保留未处理数据）
            int remaining = json_buffer_len - (end - json_buffer + 1);
            if (remaining > 0) {
                memmove(json_buffer, end + 1, remaining);
                json_buffer_len = remaining;
                json_buffer[json_buffer_len] = '\0';
            } else {
                json_buffer[0] = '\0';
                json_buffer_len = 0;
                break;
            }
        }

        // 处理缓冲区溢出
        if (json_buffer_len >= sizeof(json_buffer) - 100) {
            rt_kprintf("Buffer near full, resetting...\n");
            json_buffer[0] = '\0';
            json_buffer_len = 0;
        }
    }
}

// 发送传感器数据
void send_sensor_data(SensorData *data) {

    // 构建 JSON 数据
    char json_data[128];
    rt_snprintf(json_data, sizeof(json_data), "{\"Temp\": %d,\"Hum\": %d,\"Led\": %d,\"Shake\": %d,\"Sound\": %d,\"Bat\": %d,\"Yao\": %d}",
                data->temp, data->hum, data->led, data->shake, data->sound, data->bat, data->Yao);

    // 发送数据长度
    char send_len[32];
    rt_snprintf(send_len, sizeof(send_len), "AT+CIPSEND=%d\r\n", (int)strlen(json_data));
    Uart3_SendData(send_len);
    rt_thread_mdelay(100);

    // 发送实际数据
    Uart3_SendData(json_data);
    rt_thread_mdelay(6000);
    Uart3_SendData("AT+CIPSTART=\"TCP\",\"192.168.233.211\",12345\r\n");
    rt_thread_mdelay(1000);  // 等待连接
}

void CreatUart3TestEntry(void)
{
    static char msg_pool[512];

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
                                         2048,
                                         5,
                                         10);
    if (thread) {
        rt_thread_startup(thread);
    } else {
        rt_kprintf("Create thread failed!\n");
        return;
    }

    /******************** WiFi & TCP 配置 ********************/
    // 设置STA模式
    Uart3_SendData("AT+CWMODE=1\r\n");
    rt_thread_mdelay(2000);

    // 连接WiFi（SSID: wcnmd，密码：1145141919）
    Uart3_SendData("AT+CWJAP=\"1\",\"12345678\"\r\n");
    rt_thread_mdelay(4000);  // 等待连接

    // 建立TCP连接
    Uart3_SendData("AT+CIPSTART=\"TCP\",\"192.168.233.211\",12345\r\n");
    rt_thread_mdelay(4000);  // 等待连接
    Uart3_SendData("AT+CIPSTART=\"TCP\",\"192.168.233.211\",12345\r\n");
    rt_thread_mdelay(2000);  // 等待连接

    //发送数据
    while (1)
    {

        SensorData data = {0}; // 初始化数据
        data.led = atoi(LED);
        data.shake = atoi(Shake);
        data.sound = atoi(Sound);
        data.bat = 73;

        // 更新温湿度数据
        data.temp = T;
        data.hum = H;

        data.Yao = Yao_State;

        send_sensor_data(&data);
    }
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

#include "AHT30_driver.h"
#include <rtdbg.h>

#define AHT30_CMD_INIT        0xBE   // 初始化命令
#define AHT30_CMD_TRIGGER     0xAC   // 触发测量命令
#define AHT30_CMD_SOFTRESET   0xBA   // 软复位命令

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

/* 初始化 AHT30 */
rt_err_t aht30_init(void)
{
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find("i2c2");

    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("I2C bus not found!");
        return -RT_ERROR;
    }

    // 发送初始化命令
    rt_uint8_t init_cmd[3] = {AHT30_CMD_INIT, 0x08, 0x00};
    if (rt_i2c_master_send(i2c_bus, AHT30_I2C_ADDR, RT_I2C_WR, init_cmd, 3) != 3) {
        rt_kprintf("Init cmd send failed!");
        return -RT_ERROR;
    }

    rt_thread_mdelay(350);  // 校准需要更长时间（手册建议至少 300ms）

    // 检查校准状态
    rt_uint8_t status = 0;
    if (rt_i2c_master_recv(i2c_bus, AHT30_I2C_ADDR, RT_I2C_RD, &status, 1) != 1) {
        rt_kprintf("Read status failed!");
        return -RT_ERROR;
    }

    if ((status & 0x08) == 0) {  // Bit[3] = 1 表示校准完成
        rt_kprintf("Calibration failed!");
        return -RT_ERROR;
    }

    return RT_EOK;
}

// 读取温湿度传感器数据（整型）
rt_err_t aht30_read(int *temperature, int *humidity)
{
    rt_uint8_t data[6] = {0};
    rt_uint8_t trigger_cmd[3] = {0xAC, 0x33, 0x00};

    // 触发测量
    if (rt_i2c_master_send(i2c_bus, AHT30_I2C_ADDR, RT_I2C_WR, trigger_cmd, 3) != 3) {
        rt_kprintf("Trigger failed!");
        return -RT_ERROR;
    }

    // 等待测量完成（状态轮询）
    int data_ready = 0;
    for (int retry = 0; retry < 80; retry++) {  // 80*1ms = 80ms超时
        rt_thread_mdelay(1);
        rt_uint8_t status;
        // 读取单个状态字节
        if (rt_i2c_master_recv(i2c_bus, AHT30_I2C_ADDR, RT_I2C_RD, &status, 1) == 1) {
            if (status & 0x80) {  // Bit7=1表示数据就绪
                data_ready = 1;
                break;
            }
        }
    }
    if (!data_ready) {
        rt_kprintf("Data not ready!");
        return -RT_ERROR;
    }

    // 读取完整的6字节数据（状态+湿度+温度）
    if (rt_i2c_master_recv(i2c_bus, AHT30_I2C_ADDR, RT_I2C_RD, data, 6) != 6) {
        rt_kprintf("Read data failed!");
        return -RT_ERROR;
    }

    // 检查状态位（确保数据有效）
    if ((data[0] & 0x80) == 0) {
        rt_kprintf("Sensor busy!");
        return -RT_ERROR;
    }

    // 解析湿度（20位数据）
    rt_uint32_t hum_raw = ((rt_uint32_t)data[1] << 12) |
                         ((rt_uint32_t)data[2] << 4) |
                         (data[3] >> 4);
    *humidity = (int)((hum_raw * 100) / (1 << 20));  // 整型百分比

    // 解析温度（20位数据）
    rt_uint32_t temp_raw = ((rt_uint32_t)(data[3] & 0x0F) << 16) |
                          ((rt_uint32_t)data[4] << 8) |
                          data[5];
    *temperature = (int)((temp_raw * 200) / (1 << 20) - 50);  // 整型摄氏度

    return RT_EOK;
}

/* AHT30 线程入口函数 */
static void aht30_thread_entry(void *param)
{
    int temp, hum;

    if (aht30_init() != RT_EOK)
    {
        rt_kprintf("AHT30 init failed!");
        return;
    }

    while (1)
    {
        if (aht30_read(&temp, &hum) == RT_EOK)
        {
            //rt_kprintf("Temperature: %d C, Humidity: %d%%\n", temp, hum);
            T = temp;
            H = hum;
        }

        rt_thread_mdelay(8000);  // 每 8 秒读取一次
    }
}

/* 创建 AHT30 线程的函数 */
rt_thread_t aht30_thread_create(void)
{
    rt_thread_t tid = rt_thread_create(
            "aht30",
            aht30_thread_entry,
            RT_NULL,
            2048,
            10,
            20
        );

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return tid;
}

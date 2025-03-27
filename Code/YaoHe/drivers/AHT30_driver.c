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

    // 发送初始化命令（0xBE + 0x08 + 0x00）
    rt_uint8_t init_cmd[3] = {0xBE, 0x08, 0x00};
    if (rt_i2c_master_send(i2c_bus, AHT30_I2C_ADDR, RT_I2C_WR, init_cmd, 3) != 3)
    {
        rt_kprintf("AHT30 init failed!");
        return -RT_ERROR;
    }

    rt_thread_mdelay(10);  // 等待初始化完成
    return RT_EOK;
}

/* 读取温湿度数据 */
rt_err_t aht30_read(float *temperature, float *humidity)
{
    rt_uint8_t data[6] = {0};
    rt_uint8_t trigger_cmd[3] = {0xAC, 0x33, 0x00};

    // 触发测量（0xAC + 0x33 + 0x00）
    if (rt_i2c_master_send(i2c_bus, AHT30_I2C_ADDR, RT_I2C_WR, trigger_cmd, 3) != 3)
    {
        rt_kprintf("Trigger measurement failed!");
            return -RT_ERROR;
    }

    // 等待测量完成
    rt_thread_mdelay(80);

    // 读取 6 字节数据
    if (rt_i2c_master_recv(i2c_bus, AHT30_I2C_ADDR, RT_I2C_RD, data, 6) != 6)
    {
        rt_kprintf("Read data failed!");
        return -RT_ERROR;
    }

    // 检查状态位
    if ((data[0] & 0x80) == 0)
    {
        rt_kprintf("Sensor busy!");
        return -RT_ERROR;
    }

    // 计算湿度（单位：%RH）
    rt_uint32_t hum_raw = ((rt_uint32_t)data[1] << 12) |
                          ((rt_uint32_t)data[2] << 4) |
                          ((rt_uint32_t)data[3] >> 4);
    *humidity = (hum_raw * 100.0) / (1 << 20);

    // 计算温度（单位：°C）
    rt_uint32_t temp_raw = ((rt_uint32_t)(data[3] & 0x0F) << 16) |
                           ((rt_uint32_t)data[4] << 8) |
                           data[5];
    *temperature = (temp_raw * 200.0) / (1 << 20) - 50;

    return RT_EOK;
}

/* AHT30 线程入口函数 */
static void aht30_thread_entry(void *param)
{
    float temp, hum;

    if (aht30_init() != RT_EOK)
    {
        rt_kprintf("AHT30 init failed!");
        return;
    }

    while (1)
    {
        if (aht30_read(&temp, &hum) == RT_EOK)
        {
            rt_kprintf("Temperature: %.2f°C, Humidity: %.2f%%\n", temp, hum);
        }
        rt_thread_mdelay(2000);  // 每 2 秒读取一次
    }
}

/* 创建 AHT30 线程的函数 */
rt_thread_t aht30_thread_create(void)
{
    rt_thread_t tid = rt_thread_create(
            "aht30",
            aht30_thread_entry,
            RT_NULL,
            1024,
            10,
            20
        );

    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }

    return tid;
}

#include "drv_spi_lcd.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <drv_spi.h>
#include <stdio.h>
#include <rtdbg.h>

#define DBG_TAG "drv_spi_lcd"
#define DBG_LVL DBG_LOG

//static rt_uint8_t TFT_buffer[128][128];

/* Global Variables ------------------------------------------------------------------*/
volatile uint16_t LCD_HEIGHT = ILI9341_SCREEN_HEIGHT;
volatile uint16_t LCD_WIDTH  = ILI9341_SCREEN_WIDTH;
extern const unsigned char * gImage_image;
//spi
struct rt_spi_device spi10;
struct rt_spi_configuration spi10_configuration;
//rt_uint8_t display_buffer[ILI9341_SCREEN_HEIGHT*ILI9341_SCREEN_WIDTH] = {0};

void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Common configuration */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = GPIO_PIN_8; // CS
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_9; // DCX
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

  /* Configure PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  rt_pin_mode(CS, PIN_MODE_OUTPUT);
  rt_pin_mode(DCX, PIN_MODE_OUTPUT);
  rt_pin_mode(RES, PIN_MODE_OUTPUT);

}

void ILI9341_SPI_Init(void)
{
    rt_spi_bus_attach_device(&spi10, "spi10", "spi1", RT_NULL);
    spi10_configuration.data_width = 8;
    spi10_configuration.max_hz = 40*1000*1000;
    spi10_configuration.mode = RT_SPI_MSB | RT_SPI_MASTER | RT_SPI_MODE_0;
    rt_spi_configure(&spi10, &spi10_configuration);
    if (rt_spi_take_bus(&spi10) != RT_EOK) {
        rt_kprintf("SPI bus take failed!\n");
        return;
    }
    rt_kprintf("owner= %d\r\n", spi10.bus->owner);
    MX_GPIO_Init();
}



/*Send data (char) to LCD*/
void ILI9341_SPI_Send(unsigned char SPI_Data)
{
    rt_spi_send(&spi10, &SPI_Data, 1);
}

/* Send command (char) to LCD */
void ILI9341_Write_Command(uint8_t Command)
{
    rt_pin_write(CS, PIN_LOW);
    rt_pin_write(DCX, PIN_LOW);
    ILI9341_SPI_Send(Command);
    rt_pin_write(CS, PIN_HIGH);
}

/* Send Data (char) to LCD */
void ILI9341_Write_Data(uint8_t Data)
{
    rt_pin_write(DCX, PIN_HIGH);
    rt_pin_write(CS, PIN_LOW);
    ILI9341_SPI_Send(Data);
    rt_pin_write(CS, PIN_HIGH);
}

/* Set Address - Location block - to draw into */
void ILI9341_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
    ILI9341_Write_Command(0x2A);
    ILI9341_Write_Data(X1>>8);
    ILI9341_Write_Data(X1);
    ILI9341_Write_Data(X2>>8);
    ILI9341_Write_Data(X2);

    ILI9341_Write_Command(0x2B);
    ILI9341_Write_Data(Y1>>8);
    ILI9341_Write_Data(Y1);
    ILI9341_Write_Data(Y2>>8);
    ILI9341_Write_Data(Y2);

    ILI9341_Write_Command(0x2C);
}

/*HARDWARE RESET*/
void ILI9341_Reset(void)
{
    rt_pin_write(RES, PIN_HIGH);
    rt_thread_mdelay(200); // 使用 RT-Thread 的延时函数替代 HAL_Delay
    rt_pin_write(RES, PIN_LOW);
    rt_thread_mdelay(200);
    rt_pin_write(RES, PIN_HIGH);
    rt_thread_mdelay(120); // 确保复位完成
}


/*Ser rotation of the screen - changes x0 and y0*/
void ILI9341_Set_Rotation(uint8_t Rotation)
{

    uint8_t screen_rotation = Rotation;

    ILI9341_Write_Command(0x36);
    HAL_Delay(1);

    switch(screen_rotation)
        {
            case SCREEN_VERTICAL_1:
                ILI9341_Write_Data(0x40|0x08);
                LCD_WIDTH = 240;
                LCD_HEIGHT = 320;
                break;
            case SCREEN_HORIZONTAL_1:
                ILI9341_Write_Data(0x20|0x08);
                LCD_WIDTH  = 320;
                LCD_HEIGHT = 240;
                break;
            case SCREEN_VERTICAL_2:
                ILI9341_Write_Data(0x80|0x08);
                LCD_WIDTH  = 240;
                LCD_HEIGHT = 320;
                break;
            case SCREEN_HORIZONTAL_2:
                ILI9341_Write_Data(0x40|0x80|0x20|0x08);
                LCD_WIDTH  = 320;
                LCD_HEIGHT = 240;
                break;
            default:
                //EXIT IF SCREEN ROTATION NOT VALID!
                break;
        }
}

/*Enable LCD display*/
void ILI9341_Enable(void)
{
    rt_pin_write(RES, PIN_HIGH);
}

/*Initialize LCD display*/
void ILI9341_Init(void)
{
    ILI9341_Enable();
    rt_pin_write(CS, PIN_LOW);
    ILI9341_Reset();

    //SOFTWARE RESET
    ILI9341_Write_Command(0x01);
    HAL_Delay(1000);

    //POWER CONTROL A
    ILI9341_Write_Command(0xCB);
    ILI9341_Write_Data(0x39);
    ILI9341_Write_Data(0x2C);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x34);
    ILI9341_Write_Data(0x02);

    //POWER CONTROL B
    ILI9341_Write_Command(0xCF);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0xC1);
    ILI9341_Write_Data(0x30);

    //DRIVER TIMING CONTROL A
    ILI9341_Write_Command(0xE8);
    ILI9341_Write_Data(0x85);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x78);

    //DRIVER TIMING CONTROL B
    ILI9341_Write_Command(0xEA);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x00);

    //POWER ON SEQUENCE CONTROL
    ILI9341_Write_Command(0xED);
    ILI9341_Write_Data(0x64);
    ILI9341_Write_Data(0x03);
    ILI9341_Write_Data(0x12);
    ILI9341_Write_Data(0x81);

    //PUMP RATIO CONTROL
    ILI9341_Write_Command(0xF7);
    ILI9341_Write_Data(0x20);

    //POWER CONTROL,VRH[5:0]
    ILI9341_Write_Command(0xC0);
    ILI9341_Write_Data(0x23);

    //POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_Write_Command(0xC1);
    ILI9341_Write_Data(0x10);

    //VCM CONTROL
    ILI9341_Write_Command(0xC5);
    ILI9341_Write_Data(0x3E);
    ILI9341_Write_Data(0x28);

    //VCM CONTROL 2
    ILI9341_Write_Command(0xC7);
    ILI9341_Write_Data(0x86);

    //MEMORY ACCESS CONTROL
    ILI9341_Write_Command(0x36);
    ILI9341_Write_Data(0x48);

    //PIXEL FORMAT
    ILI9341_Write_Command(0x3A);
    ILI9341_Write_Data(0x55);

    //FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_Write_Command(0xB1);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x18);

    //DISPLAY FUNCTION CONTROL
    ILI9341_Write_Command(0xB6);
    ILI9341_Write_Data(0x08);
    ILI9341_Write_Data(0x82);
    ILI9341_Write_Data(0x27);

    //3GAMMA FUNCTION DISABLE
    ILI9341_Write_Command(0xF2);
    ILI9341_Write_Data(0x00);

    //GAMMA CURVE SELECTED
    ILI9341_Write_Command(0x26);
    ILI9341_Write_Data(0x01);

    //POSITIVE GAMMA CORRECTION
    ILI9341_Write_Command(0xE0);
    ILI9341_Write_Data(0x0F);
    ILI9341_Write_Data(0x31);
    ILI9341_Write_Data(0x2B);
    ILI9341_Write_Data(0x0C);
    ILI9341_Write_Data(0x0E);
    ILI9341_Write_Data(0x08);
    ILI9341_Write_Data(0x4E);
    ILI9341_Write_Data(0xF1);
    ILI9341_Write_Data(0x37);
    ILI9341_Write_Data(0x07);
    ILI9341_Write_Data(0x10);
    ILI9341_Write_Data(0x03);
    ILI9341_Write_Data(0x0E);
    ILI9341_Write_Data(0x09);
    ILI9341_Write_Data(0x00);

    //NEGATIVE GAMMA CORRECTION
    ILI9341_Write_Command(0xE1);
    ILI9341_Write_Data(0x00);
    ILI9341_Write_Data(0x0E);
    ILI9341_Write_Data(0x14);
    ILI9341_Write_Data(0x03);
    ILI9341_Write_Data(0x11);
    ILI9341_Write_Data(0x07);
    ILI9341_Write_Data(0x31);
    ILI9341_Write_Data(0xC1);
    ILI9341_Write_Data(0x48);
    ILI9341_Write_Data(0x08);
    ILI9341_Write_Data(0x0F);
    ILI9341_Write_Data(0x0C);
    ILI9341_Write_Data(0x31);
    ILI9341_Write_Data(0x36);
    ILI9341_Write_Data(0x0F);

    //EXIT SLEEP
    ILI9341_Write_Command(0x11);
    HAL_Delay(120);

    //TURN ON DISPLAY
    ILI9341_Write_Command(0x29);

    //STARTING ROTATION
    ILI9341_Set_Rotation(SCREEN_VERTICAL_1);
}

//INTERNAL FUNCTION OF LIBRARY
/*Sends block colour information to LCD*/
//void ILI9341_Draw_Colour_Burst(uint16_t Colour, uint32_t Size)
//{
//
//
//    //SENDS COLOUR
//
//    uint32_t Buffer_Size = 0;
//    if((Size*2) < BURST_MAX_SIZE)
//    {
//        Buffer_Size = Size;
//    }
//    else
//    {
//        Buffer_Size = BURST_MAX_SIZE;
//    }
//
//    rt_pin_write(DCX, PIN_HIGH);
//    rt_pin_write(CS, PIN_LOW);
//
//    unsigned char chifted =     Colour>>8;;
//    unsigned char burst_buffer[Buffer_Size];
//
//    for(uint32_t j = 0; j < Buffer_Size; j+=2)
//        {
//            burst_buffer[j] =   chifted;
//            burst_buffer[j+1] = Colour;
//        }
//
//    uint32_t Sending_Size = Size*2;
//    uint32_t Sending_in_Block = Sending_Size/Buffer_Size;
//    uint32_t Remainder_from_block = Sending_Size%Buffer_Size;
//
//    if(Sending_in_Block != 0)
//    {
//        for(uint32_t j = 0; j < (Sending_in_Block); j++)
//        {
//            HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char *)burst_buffer, Buffer_Size, 10);
//        }
//    }
//
//    //REMAINDER!
//    HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char *)burst_buffer, Remainder_from_block, 10);
//    rt_pin_write(CS, PIN_HIGH);
//}
void ILI9341_Write_DDRAM1(uint16_t Colour, uint32_t Size)
{
    rt_uint8_t data[2];
    data[0] = (Colour >> 8) & 0xff; // 高8位在前（RGB565的R和G高位）
    data[1] = Colour & 0xff;        // 低8位在后（G低位和B）
    rt_pin_write(DCX, PIN_HIGH);
    rt_pin_write(CS, PIN_LOW);
    for (int var = 0; var < Size; var++) {
        rt_spi_send(&spi10, data, 1);
        rt_spi_send(&spi10, data+1, 1);
    }
    rt_pin_write(CS, PIN_HIGH);
}

//FILL THE ENTIRE SCREEN WITH SELECTED COLOUR (either #define-d ones or custom 16bit)
/*Sets address (entire screen) and Sends Height*Width ammount of colour information to LCD*/
void ILI9341_Fill_Screen(uint16_t Colour)
{
    ILI9341_Set_Address(0,0,LCD_WIDTH,LCD_HEIGHT);
    ILI9341_Write_DDRAM1(Colour,LCD_WIDTH*LCD_HEIGHT);
}

// 增加圆角半径和边框厚度参数
// 改进后的圆角矩形绘制函数
void ILI9341_Draw_Rounded_Rectangle(uint16_t x0, uint16_t y0,
                                   uint16_t width, uint16_t height,
                                   uint16_t color,
                                   uint8_t radius,
                                   uint8_t thickness)
{
    /* 参数验证增强 */
    if (width < 2 || height < 2) return;
    radius = MINT(radius, MINT(width, height)/2);
    thickness = CLAMP(thickness, 1, 5);

    /* 计算有效绘制区域 */
    const uint16_t diameter = radius * 2;
    if (diameter > width || diameter > height) {
        radius = MINT(width, height)/2;
    }

    /* 优化后的圆弧绘制算法 */
    void draw_rounded_corner(uint16_t cx, uint16_t cy, uint8_t quadrant) {
        int16_t x = radius - 1;
        int16_t y = 0;
        int16_t dx = 1;
        int16_t dy = 1;
        int16_t err = dx - (int16_t)(radius << 1);

        // 分层绘制厚度
        for (uint8_t t = 0; t < thickness; t++) {
            int16_t current_radius = radius - t;
            if (current_radius <= 0) break;

            x = current_radius - 1;
            y = 0;
            dx = 1;
            dy = 1;
            err = dx - (current_radius << 1);

            while (x >= y) {
                // 根据象限绘制八个对称点
                switch (quadrant) {
                case 0: // 左上
                    ILI9341_DrawPixel(cx - x, cy - y, color); // 主弧
                    ILI9341_DrawPixel(cx - y, cy - x, color); // 对称弧
                    break;
                case 1: // 右上
                    ILI9341_DrawPixel(cx + y, cy - x, color);
                    ILI9341_DrawPixel(cx + x, cy - y, color);
                    break;
                case 2: // 右下
                    ILI9341_DrawPixel(cx + x, cy + y, color);
                    ILI9341_DrawPixel(cx + y, cy + x, color);
                    break;
                case 3: // 左下
                    ILI9341_DrawPixel(cx - y, cy + x, color);
                    ILI9341_DrawPixel(cx - x, cy + y, color);
                    break;
                }

                if (err <= 0) {
                    y++;
                    err += dy;
                    dy += 2;
                }
                if (err > 0) {
                    x--;
                    dx += 2;
                    err += dx - (current_radius << 1);
                }
            }
        }
    }

    /* 绘制四边带厚度（优化版本）*/
    // 上边
    ILI9341_FillRect(x0 + radius, y0, width - 2*radius, thickness, color);
    // 下边
    ILI9341_FillRect(x0 + radius, y0 + height - thickness, width - 2*radius, thickness, color);
    // 左边
    ILI9341_FillRect(x0, y0 + radius, thickness, height - 2*radius, color);
    // 右边
    ILI9341_FillRect(x0 + width - thickness, y0 + radius, thickness, height - 2*radius, color);

    /* 绘制四个圆角（优化坐标计算）*/
    draw_rounded_corner(x0 + radius,     y0 + radius,      0); // 左上
    draw_rounded_corner(x0 + width - radius - 1, y0 + radius,      1); // 右上
    draw_rounded_corner(x0 + width - radius - 1, y0 + height - radius - 1, 2); // 右下
    draw_rounded_corner(x0 + radius,     y0 + height - radius - 1, 3); // 左下
}

/* 新增的填充矩形函数（优化性能）*/
void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (w == 0 || h == 0) return;

    ILI9341_Set_Address(x, y, x + w - 1, y + h - 1);
    rt_pin_write(DCX, PIN_HIGH);
    rt_pin_write(CS, PIN_LOW);

    const uint32_t total = (uint32_t)w * h;
    uint8_t buffer[128]; // 128字节缓冲区（64像素）
    uint16_t *ptr = (uint16_t*)buffer;

    for(uint16_t i=0; i<64; i++) {
        ptr[i] = color;
    }

    uint32_t sent = 0;
    while(sent < total) {
        uint16_t chunk = MINT(total - sent, 64);
        rt_spi_send(&spi10, buffer, chunk * 2);
        sent += chunk;
    }

    rt_pin_write(CS, PIN_HIGH);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    /* 边界检查 */
    if (x >= ILI9341_SCREEN_WIDTH || y >= ILI9341_SCREEN_HEIGHT)
        return;

    /* 设置地址窗口 */
    ILI9341_Set_Address(x, y, x, y);

    /* 发送颜色数据 */
    uint8_t data[] = {color >> 8, color & 0xFF};
    rt_pin_write(DCX, PIN_HIGH);
    rt_pin_write(CS, PIN_LOW);
    rt_spi_send(&spi10, data, sizeof(data));
    rt_pin_write(CS, PIN_HIGH);
}


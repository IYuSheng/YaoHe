#ifndef DRIVERS_DRV_SPI_LCD_H_
#define DRIVERS_DRV_SPI_LCD_H_

#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#include <rtdef.h>
extern struct rt_spi_device spi10;

//ILI9341的SPI驱动
#define CS    GET_PIN(C, 8)  // CSX引脚
#define DCX   GET_PIN(C, 9)  // D/C引脚
#define RES   GET_PIN(D, 2)  // RST引脚
#define BEI   GET_PIN(B,0)   //背光

///* 触摸相关引脚定义 */
//#define TOUCH_I2C_NAME        "i2c1"
//#define TOUCH_RST_PIN         GET_PIN(A, 15)
//#define TOUCH_INT_PIN         GET_PIN(B, 5)
//#define FT6x36_ADDR           0x38

#define MINT(a,b) ((a)<(b)?(a):(b))
#define CLAMP(v,min,max) ((v)<(min)?(min):((v)>(max)?(max):(v)))

#define ILI9341_SCREEN_HEIGHT 240
#define ILI9341_SCREEN_WIDTH    320

#define BURST_MAX_SIZE  500

#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

#define SCREEN_VERTICAL_1           0
#define SCREEN_HORIZONTAL_1     1
#define SCREEN_VERTICAL_2           2
#define SCREEN_HORIZONTAL_2     3


//PIN define
//#define DCX 32  //DCX--PC0
//#define CS 33   //CS--PC1
//#define RES 34   //RES--PC2

//struct TFT_Message{
//    struct rt_spi_device  *device;
//    rt_uint16_t type;
//    rt_uint16_t * arg;
//    rt_base_t PIN;
//    rt_uint16_t send_lenth;
//};

void MX_GPIO_Init(void);
void ILI9341_SPI_Init(void);
void ILI9341_SPI_Send(unsigned char SPI_Data);
void ILI9341_Write_Command(uint8_t Command);
void ILI9341_Write_Data(uint8_t Data);
void ILI9341_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2);
void ILI9341_Reset(void);
void ILI9341_Set_Rotation(uint8_t Rotation);
void ILI9341_Enable(void);
void ILI9341_Init(void);
void ILI9341_Fill_Screen(uint16_t Colour);
void ILI9341_Write_DDRAM1(uint16_t Colour, uint32_t Size);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_Sleep(void);
void ILI9341_Wake(void);

#endif /* DRIVERS_DRV_SPI_LCD_H_ */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();

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

  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

  rt_pin_mode(CS, PIN_MODE_OUTPUT);
  rt_pin_mode(DCX, PIN_MODE_OUTPUT);
  rt_pin_mode(RES, PIN_MODE_OUTPUT);
  rt_pin_mode(BEI, PIN_MODE_OUTPUT);

//  // 复位引脚
//  rt_pin_mode(TOUCH_RST_PIN, PIN_MODE_OUTPUT);
//  rt_pin_write(TOUCH_RST_PIN, PIN_HIGH);
//
//  // 中断引脚
//  rt_pin_mode(TOUCH_INT_PIN, PIN_MODE_INPUT_PULLUP);

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
    rt_thread_mdelay(200);
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
    ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);

    rt_pin_write(BEI, PIN_HIGH);
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

/* 熄屏功能 */
void ILI9341_Sleep(void)
{
    rt_pin_write(BEI, PIN_LOW);
}

/* 唤醒屏幕 */
void ILI9341_Wake(void)
{
    rt_pin_write(BEI, PIN_HIGH);
}


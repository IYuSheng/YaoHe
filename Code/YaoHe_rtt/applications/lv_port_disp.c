/**
 * @file lv_port_disp_templ.c
 *
 */
#include "drv_spi_lcd.h"
#include <rtthread.h>
#include <drv_common.h>
#include <rtdevice.h>
#include <drv_spi.h>

#define MY_DISP_HOR_RES 320
 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "lvgl.h"
//#include "packages\ili9341-latest\lcd_ili9341.h"
#include "drv_spi_lcd.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf_1[MY_DISP_HOR_RES * 20] __attribute__((section(".noinit"))); // 防止被其他线程覆盖
    static lv_color_t buf_2[MY_DISP_HOR_RES * 20] __attribute__((section(".noinit")));
    lv_disp_draw_buf_init(&draw_buf, buf_1, buf_2, MY_DISP_HOR_RES * 20);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
    ILI9341_SPI_Init();
    ILI9341_Init();
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
        /* 设置一次地址窗口 */
        ILI9341_Set_Address(area->x1, area->y1, area->x2, area->y2);

        /* 计算总像素数 */
        uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);

        /* 批量发送数据 */
        rt_pin_write(DCX, PIN_HIGH); // 进入数据模式
        rt_pin_write(CS, PIN_LOW);
        rt_spi_send(&spi10, (uint8_t *)color_p, size * 2); // RGB565每像素2字节
        rt_pin_write(CS, PIN_HIGH);

        lv_disp_flush_ready(disp_drv);//告诉LVGL画图成功的返回函数，不能删，很重要
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif

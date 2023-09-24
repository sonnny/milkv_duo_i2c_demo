
// https://github.com/zijian-z/linux-ssd1306
// make
// scp ssd1306 root@192.168.42.1:/root
// ./ssd1306

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include "font.h"

#define I2C_SLAVE	0x0703	/* Use this slave address */

char* filename = "/dev/i2c-1";
int file;

uint8_t init_cmd_arr[25] = 
{
    0xae, //关闭显示
    0xd5, 0x80, //显示时钟频率
    0xa8, 0x3f, //复用率
    0xd3, 0x00, //偏移
    0x40, //开始线
    0x8d,0x14, //VCC电源
    0xa1, //段重新映射
    0xc8, //COM输出方式
    0xda, 0x12, //COM输出方式
    0x81, 0xff, //对比度最大
    0xd9, 0xf1, //充电周期
    0xdb, 0x30, //VCC电压输出
    0x20, 0x10, //页寻址
    0xa4, //0x4正常显示,0xa5整体点亮
    0xa6, //正常显示
    0xaf //开启显示
};

void write_cmd(uint8_t cmd)
{
    uint8_t buf[2];
    buf[0] = 0x00;
    buf[1] = cmd;
    if (write(file, buf, 2) != 2)
    {
        printf("write cmd %d error\n", cmd);
    }
}

void write_data(uint8_t data)
{
    uint8_t buf[2];
    buf[0] = 0x40;
    buf[1] = data;
    if (write(file, buf, 2) != 2)
    {
        printf("write data %d error\n", data);
    }
}

void oled_init()
{
    for (size_t i = 0; i < sizeof(init_cmd_arr); i++)
    {
        write_cmd(init_cmd_arr[i]);
    }
}

/**
 * x ~ 0 - 127，列数
 * y ~ 0 - 7，页数，并非行数
*/
void oled_set_pos(uint8_t x, uint8_t y)
{
    y = y % 8;
    x = x % 128;
    write_cmd(0xb0+y);
    //列地址分两次发送，一次是低4位，一次是高4位
    write_cmd(x & 0x0f);
    write_cmd((x & 0xf0) >> 4 | 0x10);
}

/**
 * 0x00为黑
 * 0xff为白
*/
void oled_clear_global(uint8_t light) {
    for (size_t i = 0; i < 8; i++)
    {
        write_cmd(0xb0 + i);
        for (size_t j = 0; j < 128; j++)
        {
            write_data(light);
        }
    }
}

/**
 * 一个ascii字符长度为6，宽度为8，宽度占据了一页
 * x ~ 0 - 127，列数
 * y ~ 0 - 7，页数，并非行数
*/
void oled_write_ascii(uint8_t x, uint8_t y, uint8_t c, uint8_t font_size)
{
    uint16_t font_index = (c - ' ') * font_size;
    oled_set_pos(x, y);
    if (font_size == 6)
    {
        for (size_t i = 0; i < font_size; i++)
        {
            write_data(font8x6[font_index + i]);
        }
    }
}

/**
 * 默认为水平直线，最长为128
 * is_vertical非0是为竖直线，最长为64
*/
void oled_draw_line(uint8_t x, uint8_t y, uint8_t length, uint8_t is_vertical, uint8_t bottom) 
{
    if (is_vertical > 0)
    {
        int page = 0;
        while (length >= 8 && y + page < 8)
        {
            oled_set_pos(x, y + page);
            write_data(0xff);
            length = length - 8;
            page++;
        }
        if (y + page < 8)
        {
            oled_set_pos(x, y + page);
            write_data(length);
        }
    } 
    else 
    {
        uint8_t data = 0x01;
        if (bottom > 0)
        {
            data = 0x80;
        }
        
        oled_set_pos(x, y);
        uint8_t max_length = length + x > 128 ? 128 : length + x;
        for (size_t i = x; i < max_length; i++)
        {
            write_data(data);
        }
    }
}

int main() 
{
    file = open(filename, O_RDWR);
    if (file < 0)
    {
        printf("open %s error\n", filename);
        exit(1);
    }
    
    int addr = 0x3c;
    if (ioctl(file, I2C_SLAVE, addr) < 0)
    {
        printf("ioctl error\n");
        exit(1);
    }
    
    oled_init();
    
    for(int i=0; i<10; i++){
    oled_clear_global(0x00);
    oled_write_ascii(0, 0, 'H', 6);
    oled_write_ascii(7, 0, 'E', 6);
    oled_write_ascii(13, 0, 'L', 6);
    oled_write_ascii(19, 0, 'L', 6);
    oled_write_ascii(25, 0, 'O', 6);
    oled_write_ascii(31, 0, ' ', 6);
    oled_write_ascii(37, 0, 'M', 6);
    oled_write_ascii(43, 0, 'I', 6);
    oled_write_ascii(49, 0, 'L', 6);
    oled_write_ascii(55, 0, 'K', 6);
    oled_write_ascii(61, 0, 'V', 6);
    oled_write_ascii(67, 0, ' ', 6);
    oled_write_ascii(73, 0, 'D', 6);
    oled_write_ascii(79, 0, 'U', 6);
    oled_write_ascii(85, 0, 'O', 6);
    sleep(1);
     oled_clear_global(0x00);
    oled_write_ascii(0, 0, '9', 6);
    oled_write_ascii(7, 0, 'D', 6);
    oled_write_ascii(13, 0, 'O', 6);
    oled_write_ascii(19, 0, 'L', 6);
    oled_write_ascii(25, 0, 'L', 6);
    oled_write_ascii(31, 0, 'A', 6);
    oled_write_ascii(37, 0, 'R', 6);
    oled_write_ascii(43, 0, ' ', 6);
    oled_write_ascii(49, 0, 'L', 6);
    oled_write_ascii(55, 0, 'I', 6);
    oled_write_ascii(61, 0, 'N', 6);
    oled_write_ascii(67, 0, 'U', 6);
    oled_write_ascii(73, 0, 'X', 6);
    sleep(1);
    }   
    
    
 
    close(file);
    return 0;
}

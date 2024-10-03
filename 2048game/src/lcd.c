/*
 * @Author: SingleBiu
 * @Date: 2021-05-21 12:01:15
 * @LastEditors: SingleBiu
 * @LastEditTime: 2021-07-09 16:17:29
 * @Description: file content
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


int lcd_fd = -1;
//帧缓冲首地址
int *plcd = NULL;

char w_digit[][58] = {
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xC0,0x1E,0xF0,
0x3C,0x78,0x38,0x38,0x78,0x3C,0x78,0x3C,0x70,0x1C,0x70,0x1C,0x70,0x1C,0x70,0x1C,
0x70,0x1C,0x70,0x1C,0x70,0x3C,0x78,0x3C,0x78,0x38,0x38,0x38,0x3C,0x70,0x1E,0xF0,
0x07,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x03,0x80,
0x1F,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,
0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0x80,0x03,0xC0,
0x1F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xE0,0x1C,0x70,
0x38,0x38,0x78,0x3C,0x78,0x3C,0x78,0x3C,0x38,0x38,0x00,0x78,0x00,0x70,0x00,0xE0,
0x01,0xC0,0x03,0x80,0x07,0x00,0x0E,0x00,0x1C,0x0C,0x38,0x1C,0x70,0x3C,0x7F,0xF8,
0x7F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xC0,0x38,0xF0,
0x38,0x78,0x78,0x78,0x38,0x78,0x00,0x78,0x00,0x70,0x00,0xE0,0x07,0xC0,0x00,0xF0,
0x00,0x78,0x00,0x38,0x00,0x3C,0x00,0x3C,0x78,0x3C,0x78,0x38,0x78,0x38,0x38,0xF0,
0x0F,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0xE0,0x01,0xE0,
0x03,0xE0,0x03,0xE0,0x07,0xE0,0x0E,0xE0,0x0C,0xE0,0x1C,0xE0,0x18,0xE0,0x30,0xE0,
0x70,0xE0,0x60,0xE0,0xFF,0xFC,0x00,0xE0,0x00,0xE0,0x00,0xE0,0x00,0xE0,0x01,0xF0,
0x07,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF8,0x3F,0xF8,
0x38,0x00,0x38,0x00,0x38,0x00,0x38,0x00,0x30,0x00,0x37,0xE0,0x3C,0x70,0x38,0x38,
0x10,0x3C,0x00,0x3C,0x00,0x3C,0x78,0x3C,0x78,0x3C,0x78,0x38,0x38,0x78,0x38,0xF0,
0x0F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xF0,0x0E,0x78,
0x1C,0x78,0x38,0x78,0x38,0x00,0x78,0x00,0x70,0x00,0x77,0xE0,0x7E,0x78,0x78,0x38,
0x78,0x3C,0x70,0x3C,0x70,0x1C,0x70,0x1C,0x78,0x3C,0x38,0x3C,0x3C,0x38,0x1E,0x70,
0x0F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFC,0x3F,0xF8,
0x78,0x38,0x70,0x30,0x60,0x60,0x00,0x60,0x00,0xC0,0x01,0xC0,0x01,0xC0,0x03,0x80,
0x03,0x80,0x03,0x80,0x07,0x80,0x07,0x00,0x07,0x00,0x07,0x00,0x0F,0x00,0x0F,0x00,
0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xE0,0x3C,0x70,
0x38,0x38,0x70,0x3C,0x70,0x1C,0x70,0x1C,0x38,0x38,0x3E,0x70,0x0F,0xE0,0x1F,0xE0,
0x38,0xF0,0x70,0x78,0x70,0x3C,0x70,0x1C,0x70,0x1C,0x70,0x1C,0x70,0x38,0x3C,0x70,
0x0F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xC0,0x3C,0x70,
0x38,0x38,0x78,0x38,0x70,0x3C,0x70,0x3C,0x70,0x1C,0x70,0x3C,0x70,0x3C,0x78,0x7C,
0x3C,0xFC,0x1F,0xFC,0x00,0x3C,0x00,0x38,0x00,0x38,0x38,0x70,0x38,0x70,0x3D,0xE0,
0x1F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};


//打开屏幕
void lcd_open(void)
{
	int fd;
	// fd = open("/dev/ubuntu_lcd",O_RDWR);
	fd = open("/dev/fb0",O_RDWR);
	if(fd == -1)
	{
		printf("sorry,open fail\n");
	}
	printf("open success\n");
	lcd_fd=fd;
	plcd=mmap(NULL, 480*800*4, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);   
	if(plcd==MAP_FAILED)
	{
		printf("sorry,mmap open fail\n");
	}
}
//关闭屏幕
void lcd_close(void)
{
	munmap(plcd,480*800*4);
	close(lcd_fd);
}
//描点
void lcd_draw_point(int x, int y, int color)
{
	if (x >= 0 &&  x < 800 &&  y >= 0 &&  y < 480)
	{
		*(plcd + 800*y + x) = color;
	}
}
//画矩形
void lcd_draw_rectanle(int x0, int y0, int w, int h, int color)
{
	int i,j;
	for(i=0;i<w;i++)
	{	
		for(j=0;j<h;j++)
		lcd_draw_point( x0+i,y0+j,color);
	}
}
//清屏
void lcd_clear_screen(int color)
{
	lcd_draw_rectanle(0, 0, 800, 480,  color);
}




void lcd_draw_word(char *ch, int len, int w, int color, int x0, int y0)
{
	int i,j;
	int x, y;
	int flag = w/8;
	//遍历整个点阵数组
	for (i = 0; i < len; i++)
	{
		for (j = 7; j >= 0; j--)
		{
			//if (ch[i] & (0x80/pow(2,7-j)))
			if ((ch[i] >> j) & 1)
			{
				x = 7-j+8*(i%flag) + x0;
				y = i/flag + y0;
				lcd_draw_point(x, y, color);
			}
		}
	}
}

void lcd_draw_digit(int value,int x0, int y0, int color)
{
	int i = 0;
	if (value == 0)
	{
		lcd_draw_word(w_digit[0],sizeof(w_digit[0]),16,color,x0+48-16*i,y0);
	}
	
	while (value)
	{
		int x;
		x = value%10;
		lcd_draw_word(w_digit[x],sizeof(w_digit[x]),16,color,x0+48-16*i,y0);
		value = value/10;
		i++;
	}
}
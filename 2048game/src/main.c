/*
 * @Author: SingleBiu
 * @Date: 2021-05-21 12:02:09
 * @LastEditors: SingleBiu
 * @LastEditTime: 2021-05-21 16:48:33
 * @Description: file content
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lcd.h"
#include "2048.h"
#include "bmp.h"
#include "ts.h"
struct button{
    int x;
    int y;
    int width;
    int hight;
};
int main()
{
	srandom ( time(NULL) );

	//1. 打开屏幕
	lcd_open();

	lcd_clear_screen(0x555555);
		 struct button bt= {700, 400, 50, 50};
  bmp_display("../2048game_img/exit.bmp", bt.x, bt.y);
	// 关闭屏幕
	game_2048();
	// int mv = get_user_input();
	// if (mv)
	// {	
	// 	goto restart;
	// }
	
	lcd_close();
	return 0;

}


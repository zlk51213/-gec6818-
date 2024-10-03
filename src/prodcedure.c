#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <framebuffer.h>
#include <touchscreen.h>
#include <font.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <linklist_file.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>
#include "login.h"
#include "music.h"
#include "crame.h"
int ts_fd;             // 触摸屏文件描述符
int fb_fd;             // 帧缓冲区文件描述符
unsigned int *fb_addr; // 帧缓冲区地址
bitmap bm;             // 位图结构体
font *f;               // 字体结构体
// 全局变量用于指示线程是否应该停止
volatile int should_stop = 0;

// 按键控件
struct button
{
    int x;
    int y;
    int width;
    int hight;
    unsigned int color;       // 没有点击的颜色
    unsigned int press_color; // 点击的颜色
    void (*handler)(void *);  // 按键的响应函数指针
};

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
struct button *bt[4]; // 按钮结构体数组

void clear_time_display(unsigned int *fb_addr, unsigned int bg_color)
{
    int x, y;
    for (y = 18; y < 18 + 30; y++)
    {
        for (x = 570; x < 570 + 220; x++)
        {
            display_point_to_framebuffer(x, y, bg_color, fb_addr);
        }
    }
}

void *display_time_thread()
{
    time_t t;
    struct tm *tmp;
    char buffer[64];
    font *f_t = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
    fontSetSize(f_t, 30);

    while (1)
    {

        sleep(1); // 每秒更新一次时间
        if (should_stop)
        {
            return NULL; // 退出循环
        }
        time(&t);
        tmp = localtime(&t);

        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tmp);

        clear_time_display(fb_addr, 0x005a9bd5);

        // 显示新时间
        fontPrint(f_t, &bm, 580, 20, buffer, getColor(0xff, 0xff, 0xff, 0xff), 0);
    }
    return NULL;
}

// 初始化桌面
void init_Desktop(unsigned int *fb_addr, struct button *bt[], char *url[4])
{
    int x, y;
    int reval;
    should_stop = 0;
    reval = display_jpeg_picture_to_framebuffer("../img/1.jpg", 0, 0, fb_addr, 1);
    if (reval == 0)
    {
        printf("display picture success\n");
        return;
    }
    reval = display_jpeg_picture_to_framebuffer("../img/2.jpg", 0, 415, fb_addr, 1);
    if (reval == 0)
    {
        printf("display picture success\n");
        return;
    }
    int i;
    for (i = 0; i < 4; i++)
        display_jpeg_picture_to_framebuffer(url[i], bt[i]->x, bt[i]->y, fb_addr, 1.2);
    //  //打开字体
    font *f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
    // //字体大小的设置
    fontSetSize(f, 30);

    bm.height = 480;
    bm.width = 800;
    bm.byteperpixel = 4;
    bm.map = (void *)fb_addr; // 指定画板是显存地址

    // //将字体写到点阵图上node.name    iller.name for (i=0; i<4; i++)  {fontPrint; pos =pos->next;}
    fontPrint(f, &bm, 15, 60, "2048",  getColor(0, 255, 255, 255), 0); // fontPrint(字体资源， 显存资源， x坐标，y坐标，显示的字符串, 字体颜色, 默认写0);
    fontPrint(f, &bm, 110, 60, "音乐", getColor(0, 255, 255, 255), 0);
    fontPrint(f, &bm, 175, 60, "相册", getColor(0, 255, 255, 255), 0);
    fontPrint(f, &bm, 245, 60, "扫雷", getColor(0, 255, 255, 255), 0);

    pthread_t time_pthread;
    if (pthread_create(&time_pthread, NULL, display_time_thread, (void *)fb_addr) != 0)
    {
        perror("创建时间线程失败");
        return;
    }
    fontUnload(f);
}

// 2048游戏的按键事件处理函数
void game1()
{
    int reval;
    reval = system("cd ../2048game/src && ./2048");

    pid_t pid = getpid(); // 调用 getpid() 获取当前进程的 PID
    printf("The Process ID (PID) is: %d\n", pid);
}
//  扫雷游戏的按键事件处理函数
void game2()
{
    int reval;
    reval = system("cd ../saolei && ./saolei");
    pid_t pid = getpid(); // 调用 getpid() 获取当前进程的 PID
    printf("The Process ID (PID) is: %d\n", pid);
}

// 显示按键
void display_button(struct button *bt, unsigned int *fb_addr)
{
    int x, y;
    // 显示一个按键
    for (y = bt->y; y < bt->y + bt->hight; y++)
    {
        for (x = bt->x; x < bt->x + bt->width; x++)
        {
            display_point_to_framebuffer(x, y, bt->color, fb_addr);
        }
    }
}
// 申请按键
struct button *request_button(int x, int y, int width, int hight, unsigned int color, unsigned int pcolor, void (*handler)(void *))
{
    struct button *bt;
    bt = calloc(1, sizeof(struct button)); // 申请1个按键结构体内存

    // 按键初始化
    bt[0].x = x;
    bt[0].y = y;
    bt[0].width = width;
    bt[0].hight = hight;
    bt[0].color = color;        // 记录按下的颜色
    bt[0].press_color = pcolor; // 记录松开的颜色
    bt[0].handler = handler;    // 记录以后按倒这个按键之后做什么动作

    return bt;
}
// 按键按下事件
void display_press_button(struct button *bt, unsigned int *fb_addr)
{
    int x, y;
    // 显示一个按键
    for (y = bt->y; y < bt->y + bt->hight; y++)
        for (x = bt->x; x < bt->x + bt->width; x++)
            display_point_to_framebuffer(x, y, bt->press_color, fb_addr);
}

int init_procedure(void)
{
    // system("mplayer -speed 2  ../vedio/2.mp4   -x  800  -y  480  -Zoom  -geometry 0:0   0  -quiet ");
    //login();
    bt[0] = request_button(10, 0, 60, 60, 0x00ff0000, 0x00ffffff, game1);
    bt[1] = request_button(100, 0, 60, 60, 0x0000ff00, 0x00ffffff, music);
    bt[2] = request_button(170, 0, 60, 60, 0x000000ff, 0x00ffffff, crame);
    bt[3] = request_button(240, 0, 60, 60, 0x00ffff00, 0x00ffffff, game2);
    char *url[4] = {"../img/2048.jpg", "../img/kg.jpg", "../img/crame.jpg", "../img/saolei.jpg"};
    fb_addr = init_framebuffer_device(&fb_fd); // 初始化显存设备
    init_Desktop(fb_addr, bt, url);            // 初始化桌面
    int status;
    int ts_x, ts_y;
    int press_x, press_y;
    int release_x, release_y;
    int i;
    ts_fd = init_touchscreen_device(); // 初始化触摸屏设备
    // 监控触摸屏
    while (1)
    {
        status = get_touchscreen_status(ts_fd, &ts_x, &ts_y);
        switch (status)
        {
        case TOUCH_PRESS_STATUS:
            // 记录按下的触摸屏坐标
            press_x = ts_x;
            press_y = ts_y;

            for (i = 0; i < 4; i++)
            {
                // 判断是否按倒了按键范围内
                if (press_x > bt[i]->x && press_x < bt[i]->x + bt[i]->width && press_y > bt[i]->y && press_y < bt[i]->y + bt[i]->hight) // 判断松开的坐标是否在范围内
                {
                    // 显示按下来按键内容
                    display_press_button(bt[i], fb_addr);
                }
            }
            break;
        case TOUCH_RELEASE_STATUS:
            // printf("触摸屏被松开x=%d, y=%d\n", ts_x, ts_x);
            release_x = ts_x;
            release_y = ts_y;

            // 循环显示4个按键
            for (i = 0; i < 4; i++)
            {

                if (press_x > bt[i]->x && press_x < bt[i]->x + bt[i]->width && press_y > bt[i]->y && press_y < bt[i]->y + bt[i]->hight) // 判断松开的位置在是否在我们的按键范围内
                {
                    display_jpeg_picture_to_framebuffer(url[i], bt[i]->x, bt[i]->y, fb_addr, 1.2); // 显示按键

                    // 判断松开来的位置是否在按键范围内容
                    if (release_x > bt[i]->x && release_x < bt[i]->x + bt[i]->width && release_y > bt[i]->y && release_y < bt[i]->y + bt[i]->hight) // 判断松开的坐标是否在范围内
                    {
                        should_stop = 1;                // 调用停止线程的函数
                        bt[i]->handler(bt[i]);          // 执行响应函数
                        init_Desktop(fb_addr, bt, url); // 重新初始化桌面
                    }
                }
            }

            break;
        default:
            break;
        }
    }
    return 0;
}
void destroy_procedure()
{

    // 销毁界面

    destroy_framebuffer_device(fb_fd, fb_addr); // 销毁显存设备

    // 释放资源
    int i;
    for (i = 0; i < 4; i++)
    {
        free(bt[i]);
    }
    printf("finish %p\n", fb_addr);

    destroy_framebuffer_device(fb_fd, fb_addr); // 销毁显存设备
}

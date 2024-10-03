#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <font.h>
#include <framebuffer.h>
#include <string.h>
#include "login.h"
#define BIG 800 * 480 * 3
#define LITTLE 76 * 56 * 3

#define X1 203 + 80 * 0
#define X2 203 + 80 * 1
#define X3 203 + 80 * 2
#define X4 203 + 80 * 3
#define X5 203 + 80 * 4
#define X6 203 + 80 * 5
#define ACCOUNT_Y 43
#define PASSWD_Y 123

#define K0 0
#define K1 1
#define K2 2
#define K3 3
#define K4 4
#define K5 5
#define K6 6
#define K7 7
#define K8 8
#define K9 9
#define K_DELETE 10
#define K_OK 11
struct Point
{
    int x;
    int y;
};
int fb_fd;
unsigned int *fb_addr;
font *f;
bitmap bm;    // 位图结构体
int flag = 0; // 0表示已注册，1表示未注册
int getPoint(int *x, int *y, int *key_status)
{
    // int x,y;

    // 1、打开触摸屏驱动
    int ts_fd;
    ts_fd = open("/dev/input/event0", O_RDWR);
    if (ts_fd == -1)
    {
        perror("打开触摸屏失败\n");
        return -1;
    }

    // 定义事件结构体
    struct input_event ts_event;

    while (1)
    {
        // 2、读取触摸坐标
        read(ts_fd, &ts_event, sizeof(ts_event));

        // 3、	处理坐标，判断信息 if
        if (ts_event.type == EV_ABS) // 判断当前是否为触摸
        {
            if (ts_event.code == ABS_X) // 判断当前触摸坐标是否是x轴
            {
                *x = ts_event.value; // 输出x轴坐标
                // printf("x轴的坐标为：%d\n",x);
            }
            if (ts_event.code == ABS_Y) // 判断当前触摸坐标是否是x轴
            {
                *y = ts_event.value; // 输出x轴坐标
                // printf("y轴的坐标为：%d\n",y);
            }
            printf("当前的坐标为：(%d,%d)\n", *x, *y);
        }

        // 判断是否按下   按下：1  松开0
        if (ts_event.type == EV_KEY && ts_event.code == BTN_TOUCH && ts_event.value == 0)
        {
            printf("当前的触摸值：%d\n", ts_event.value);
            printf("最后的坐标为：(%d,%d)\n", *x, *y);
            printf("you have leaving....\n");

            break;
        }
    }

    // 4、关闭触摸
    *key_status = 0;
    close(ts_fd);

    return 0;
}

int loadBigImg(char *path, int lcdbuf[])
{
    int bmpfd;
    int ret;
    int wet;
    int i;
    char bmpbuf[BIG];
    printf("%s", path);
    bmpfd = open(path, O_RDWR);
    if (bmpfd == -1)
    {
        perror("打开图片失败\n");
        return -1;
    }
    // 略过头字节
    lseek(bmpfd, 54, SEEK_SET);

    // 2、读取BMP图片的像素点
    ret = read(bmpfd, bmpbuf, BIG);
    if (ret == -1)
    {
        perror("读取图片失败\n");
        return -1;
    }
    printf("读取的图片字节数为%d\n", ret);

    // 3-4字节转换
    for (i = 0; i < 800 * 480; i++)
    {
        //       			B                  G                  R            A
        lcdbuf[i] = bmpbuf[0 + i * 3] << 0 | bmpbuf[1 + i * 3] << 8 | bmpbuf[2 + i * 3] << 16 | 0x00 << 24;
    }

    // 翻转
    int x, y;
    int temp[800 * 480];
    for (y = 0; y < 480; y++)
    {
        for (x = 0; x < 800; x++)
        {
            temp[800 * y + x] = lcdbuf[800 * (479 - y) + x];
        }
    }

    for (y = 0; y < 480; y++)
    {
        for (x = 0; x < 800; x++)
        {
            lcdbuf[800 * y + x] = temp[800 * y + x];
        }
    }
    close(bmpfd);
    return 0;
}

int loadLittleImg(char *path, int lcdbuf[])
{
    int bmpfd;
    int ret;
    int wet;
    int i;
    char bmpbuf[LITTLE];

    bmpfd = open(path, O_RDWR);
    if (bmpfd == -1)
    {
        perror("打开图片失败\n");
        return -1;
    }
    // 略过头字节
    lseek(bmpfd, 54, SEEK_SET);

    // 2、读取BMP图片的像素点
    ret = read(bmpfd, bmpbuf, LITTLE);
    if (ret == -1)
    {
        perror("读取图片失败\n");
        return -1;
    }
    printf("读取的图片字节数为%d\n", ret);

    // 3-4字节转换
    for (i = 0; i < 76 * 56; i++)
    {
        //       			B                  G                  R            A
        lcdbuf[i] = bmpbuf[0 + i * 3] << 0 | bmpbuf[1 + i * 3] << 8 | bmpbuf[2 + i * 3] << 16 | 0x00 << 24;
    }

    // 翻转
    int x, y;
    int temp[76 * 56];
    for (y = 0; y < 56; y++)
    {
        for (x = 0; x < 76; x++)
        {
            temp[76 * y + x] = lcdbuf[76 * (55 - y) + x];
        }
    }

    for (y = 0; y < 56; y++)
    {
        for (x = 0; x < 76; x++)
        {
            lcdbuf[76 * y + x] = temp[76 * y + x];
        }
    }
    close(bmpfd);

    return 0;
}

void changeBuf(int lcdbuf[], int data[], int type, int index)
{

    int i;
    int j;
    int x = -1;
    int y = -1;
    if (type == 0)
    {
        y = ACCOUNT_Y;
        switch (index)
        {
        case 1:
            x = X1;
            break;
        case 2:
            x = X2;
            break;
        case 3:
            x = X3;
            break;
        case 4:
            x = X4;
            break;
        case 5:
            x = X5;
            break;
        case 6:
            x = X6;
            break;
        }
    }
    else if (type == 1)
    {
        y = PASSWD_Y;
        switch (index)
        {
        case 1:
            x = X1;
            break;
        case 2:
            x = X2;
            break;
        case 3:
            x = X3;
            break;
        case 4:
            x = X4;
            break;
        case 5:
            x = X5;
            break;
        case 6:
            x = X6;
            break;
        }
    }

    for (i = y; i < 56 + y; i++)
    {
        for (j = x; j < 76 + x; j++)
        {
            lcdbuf[800 * i + j] = data[(i - y) * 76 + j - x];
        }
    }
}

int updateGui(int lcdbuf[])
{
    int *lcdmmap;
    int lcdfd;

    lcdfd = open("/dev/fb0", O_RDWR);
    if (lcdfd == -1)
    {
        perror("打开lcd失败\n");
        return -1;
    }
    lcdmmap = mmap(NULL, 800 * 480 * 4, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, lcdfd, 0);
    if (lcdmmap == NULL)
    {
        perror("映射失败\n");
        return -1; // 输出错误
    }
    // 释放内存
    int x, y;
    for (y = 0; y < 480; y++)
    {
        for (x = 0; x < 800; x++)
        {
            lcdmmap[800 * y + x] = lcdbuf[800 * y + x];
        }
    }

    munmap(lcdmmap, 800 * 480 * 4);
    if (flag == 1)
    {
        // //将字体写到点阵图上node.name    iller.name for (i=0; i<4; i++)  {fontPrint; pos =pos->next;}
        fontPrint(f, &bm, 380, 0, "注册", getColor(0, 30, 30, 30), 0); // fontPrint(字体资源， 显存资源， x坐标，y坐标，显示的字符串, 字体颜色, 默认写0);
    }
    else
        fontPrint(f, &bm, 380, 0, "登陆", getColor(0, 30, 30, 30), 0); // fontPrint(字体资源， 显存资源， x坐标，y坐标，显示的字符串, 字体颜色, 默认写0);

    // 4、关闭BMP和lcd

    close(lcdfd);
    return 0;
}

int getKey(int x, int y)
{
    if (x >= 261 && x <= 579 && y >= 241 && y <= 419)
    {
        if (x >= 501 && x <= 579 && y >= 301 && y <= 359)
        {
            return K0;
        }
        else if (x >= 261 && x <= 339 && y >= 241 && y <= 299)
        {
            return K1;
        }
        else if (x >= 341 && x <= 419 && y >= 241 && y <= 299)
        {
            return K2;
        }
        else if (x >= 421 && x <= 499 && y >= 241 && y <= 299)
        {
            return K3;
        }
        else if (x >= 261 && x <= 339 && y >= 301 && y <= 359)
        {
            return K4;
        }
        else if (x >= 341 && x <= 419 && y >= 301 && y <= 359)
        {
            return K5;
        }
        else if (x >= 421 && x <= 499 && y >= 301 && y <= 359)
        {
            return K6;
        }
        else if (x >= 261 && x <= 339 && y >= 361 && y <= 419)
        {
            return K7;
        }
        else if (x >= 341 && x <= 419 && y >= 361 && y <= 419)
        {
            return K8;
        }
        else if (x >= 421 && x <= 499 && y >= 361 && y <= 419)
        {
            return K9;
        }
        else if (x >= 501 && x <= 579 && y >= 241 && y <= 299)
        {
            return K_DELETE;
        }
        else if (x >= 501 && x <= 579 && y >= 361 && y <= 419)
        {
            return K_OK;
        }
    }
    return -1;
}

void delete(int lcdbuf[], int type, int index)
{
    int i;
    int j;
    int x = -1;
    int y = -1;
    if (type == 0)
    {
        y = ACCOUNT_Y;
        switch (index)
        {
        case 1:
            x = X1;
            break;
        case 2:
            x = X2;
            break;
        case 3:
            x = X3;
            break;
        case 4:
            x = X4;
            break;
        case 5:
            x = X5;
            break;
        case 6:
            x = X6;
            break;
        }
    }
    else if (type == 1)
    {
        y = PASSWD_Y;
        switch (index)
        {
        case 1:
            x = X1;
            break;
        case 2:
            x = X2;
            break;
        case 3:
            x = X3;
            break;
        case 4:
            x = X4;
            break;
        case 5:
            x = X5;
            break;
        case 6:
            x = X6;
            break;
        }
    }

    for (i = y; i < 56 + y; i++)
    {
        for (j = x; j < 76 + x; j++)
        {
            // lcdbuf[800 * i + j] = data[(i - y) * 76 + j - x];
            lcdbuf[800 * i + j] = 0xFFFFFF; // 白色
        }
    }
}

int login(void)
{

    FILE *login_fd;

    login_fd = fopen("login.txt", "r");
    if (login_fd == NULL)
    {
        perror("没有login.txt文件，用户未注册\n");
        flag = 1;
    }

    fb_addr = init_framebuffer_device(&fb_fd); // 初始化显存设备
    f = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");

    fontSetSize(f, 50);

    bm.height = 480;
    bm.width = 800;
    bm.byteperpixel = 4;
    bm.map = (void *)fb_addr; // 指定画板是显存地址

    int lcdbuf_main[800 * 480];
    int lcdbuf_dl[800 * 480];
    int lcdbuf_lo[800 * 480];
    int lcdbuf_sb[800 * 480];
    int lcdbuf0[76 * 56];
    int lcdbuf1[76 * 56];
    int lcdbuf2[76 * 56];
    int lcdbuf3[76 * 56];
    int lcdbuf4[76 * 56];
    int lcdbuf5[76 * 56];
    int lcdbuf6[76 * 56];
    int lcdbuf7[76 * 56];
    int lcdbuf8[76 * 56];
    int lcdbuf9[76 * 56];
    int lcdbufm[76 * 56];
    int key_status = 0;
    int x = -1;
    int y = -1;
    int accountNum = 0;
    int passwdNum = 0;
    char account[10];
    char passwd[10];
    char inputAccount[10];
    char inputPasswd[10];
    int input_status = 1;
    char login_Account[50] = {0};
    memset(account, 0, sizeof(account));
    memset(passwd, 0, sizeof(passwd));
    memset(inputAccount, 0, sizeof(inputAccount));
    memset(inputPasswd, 0, sizeof(inputPasswd));

    loadBigImg("../login_img/main.bmp", lcdbuf_main);
    loadBigImg("../login_img/dl.bmp", lcdbuf_dl);
    loadBigImg("../login_img/sb.bmp", lcdbuf_sb);
    loadLittleImg("../login_img/0.bmp", lcdbuf0);
    loadLittleImg("../login_img/1.bmp", lcdbuf1);
    loadLittleImg("../login_img/2.bmp", lcdbuf2);
    loadLittleImg("../login_img/3.bmp", lcdbuf3);
    loadLittleImg("../login_img/4.bmp", lcdbuf4);
    loadLittleImg("../login_img/5.bmp", lcdbuf5);
    loadLittleImg("../login_img/6.bmp", lcdbuf6);
    loadLittleImg("../login_img/7.bmp", lcdbuf7);
    loadLittleImg("../login_img/8.bmp", lcdbuf8);
    loadLittleImg("../login_img/9.bmp", lcdbuf9);
    loadLittleImg("../login_img/m.bmp", lcdbufm);
    loadBigImg("../login_img/lo.bmp", lcdbuf_lo);
login_begin:
    updateGui(lcdbuf_main);

    if (flag == 1)
    {
        // //将字体写到点阵图上node.name    iller.name for (i=0; i<4; i++)  {fontPrint; pos =pos->next;}
        fontPrint(f, &bm, 380, 0, "注册", getColor(0, 30, 30, 30), 0); // fontPrint(字体资源， 显存资源， x坐标，y坐标，显示的字符串, 字体颜色, 默认写0);
    }
    else
        fontPrint(f, &bm, 380, 0, "登陆", getColor(0, 30, 30, 30), 0); // fontPrint(字体资源， 显存资源， x坐标，y坐标，显示的字符串, 字体颜色, 默认写0);

    while (1)
    {
        getPoint(&x, &y, &key_status);
        if (key_status == 0)
        {
            printf("x = %d  y = %d\n", x, y);
            switch (getKey(x, y))
            {
            case K0:
                printf("您按下了0\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf0, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = K0;
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf0, 0, input_status);

                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '0';
                    input_status++;
                }
                break;
            case K1:
                printf("您按下了1\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf1, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '1';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf1, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '1';
                    input_status++;
                }
                break;
            case K2:
                printf("您按下了2\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf2, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '2';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf2, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '2';
                    input_status++;
                }
                break;
            case K3:
                printf("您按下了3\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf3, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '3';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf3, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '3';
                    input_status++;
                }
                break;
            case K4:
                printf("您按下了4\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf4, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '4';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf4, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '4';
                    input_status++;
                }
                break;
            case K5:
                printf("您按下了5\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf5, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '5';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf5, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '5';
                    input_status++;
                }
                break;
            case K6:
                printf("您按下了6\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf6, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '6';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf6, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '6';
                    input_status++;
                }
                break;
            case K7:
                printf("您按下了7\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf7, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '7';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf7, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '7';
                    input_status++;
                }
                break;
            case K8:
                printf("您按下了8\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf8, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '8';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf8, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '8';
                    input_status++;
                }
                break;
            case K9:
                printf("您按下了9\n");
                if (input_status > 12)
                {
                    break;
                    printf("输入已满\n");
                }

                if (input_status > 6 && input_status <= 12)
                {
                    changeBuf(lcdbuf_main, lcdbuf9, 1, input_status - 6);
                    updateGui(lcdbuf_main);
                    inputPasswd[input_status - 7] = '9';
                    input_status++;
                }
                if (input_status > 0 && input_status <= 6)
                {
                    changeBuf(lcdbuf_main, lcdbuf9, 0, input_status);
                    updateGui(lcdbuf_main);
                    inputAccount[input_status - 1] = '9';
                    input_status++;
                }
                break;
            case K_DELETE:
                printf("您按下了Delete\n");
                if (input_status > 1)
                {
                    if (input_status > 1 && input_status <= 7)
                    {

                        delete (lcdbuf_main, 0, input_status - 1);
                        updateGui(lcdbuf_main);
                        input_status--;
                        printf("input_status %d\n", input_status);
                    }
                    if (input_status > 7 && input_status <= 13)
                    {

                        delete (lcdbuf_main, 1, input_status - 7);
                        updateGui(lcdbuf_main);
                        input_status--;
                        printf("input_status %d\n", input_status);
                    }
                }
                break;
            case K_OK:
                printf("您按下了OK\n");
                    printf("inputAccount %s\n", inputAccount);
                    printf("inputPasswd %s\n", inputPasswd);
                if (input_status != 13)
                {
                    break;
                }
                if (flag == 0)
                {
                    printf("haha\n");

                    login_fd = fopen("login.txt", "r");
                    fgets(login_Account, 50, login_fd);
                    printf("%s\n", login_Account);
                    char *str1 = strstr(login_Account, "account");
                    char *str2 = strstr(login_Account, "password");
                    str1 = str1 + 8;
                    str2 = str2 + 9;
                    printf("str1 %s\n", str1);
                    printf("str2 %s\n", str2);
                    strncpy(account, str1, 6);
                    strncpy(passwd, str2, 6);
                    printf("account %s\n", account);
                    printf("password %s\n", passwd);
                    printf("inputAccount %s\n", inputAccount);
                    printf("inputPasswd %s\n", inputPasswd);
                    fclose(login_fd);
                    if (strcmp(account, inputAccount) == 0 && strcmp(passwd, inputPasswd) == 0)
                    {
                        printf("登录成功！！\n");
                        updateGui(lcdbuf_dl);
                        sleep(1);
                        return 0;
                    }
                    else
                    {
                        printf("登录失败！！\n");
                        updateGui(lcdbuf_sb);
                        sleep(5);
                        updateGui(lcdbuf_main);
                    }
                }
                else
                {
                    login_fd = fopen("login.txt", "w+");
                    if (login_fd == NULL)
                    {
                        perror("创建文件失败\n");
                        continue;
                    }
                    printf("account:%s password:%s\n", inputAccount, inputPasswd);
                    fprintf(login_fd, "account:%s password:%s\n", inputAccount, inputPasswd);
                    printf("注册成功！\n");
                    updateGui(lcdbuf_lo);
                    sleep(1);
                    updateGui(lcdbuf_main);
                    flag = 0;
                    fclose(login_fd);
                    goto login_begin;
                }

                break;
            default:
                break;
            }
        }
    }
    destroy_framebuffer_device(fb_fd, fb_addr); // 销毁显存设备
    return 0;
}
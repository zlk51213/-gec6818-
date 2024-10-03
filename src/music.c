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
extern int ts_fd;             // 触摸屏文件描述符
extern int fb_fd;             // 帧缓冲区文件描述符
extern unsigned int *fb_addr; // 帧缓冲区地址
int play_flag = 0, first_flag = 0;
Node *node_mp3;
void init_music()
{
    display_jpeg_picture_to_framebuffer("../music_img/music_bg.jpg", 0, 0, fb_addr, 1);
    display_jpeg_picture_to_framebuffer("../music_img/exit.jpg", 0, 0, fb_addr, 1);
    display_jpeg_picture_to_framebuffer("../music_img/pre.jpg", 150, 390, fb_addr, 1);
    display_jpeg_picture_to_framebuffer("../music_img/callback.jpg", 278, 390, fb_addr, 1);
    display_jpeg_picture_to_framebuffer("../music_img/stop.jpg", 390, 390, fb_addr, 1);
    display_jpeg_picture_to_framebuffer("../music_img/quick_next.jpg", 482, 390, fb_addr, 1);
    display_jpeg_picture_to_framebuffer("../music_img/next.jpg", 602, 390, fb_addr, 1);
}
int get_music_url(Linklist *linklist_mp3)
{
    DIR *dirp;
    struct dirent *direntp;
    // 添加寻找路径
    char *path = "../music";
    // 存储图片路径
    int i = 0;
    int music_number = 0;
    if ((dirp = opendir(path)) == NULL)
    {
        printf("打开目录 %s 失败: %s\n", path, strerror(errno));
        return -1;
    }
    while ((direntp = readdir(dirp)) != NULL)
    {
        /* 给文件或目录名添加路径:argv[1]+"/"+direntp->d_name */
        char dirbuf[512];
        memset(dirbuf, 0, sizeof(dirbuf));
        strcpy(dirbuf, path);
        strcat(dirbuf, "/");
        strcat(dirbuf, direntp->d_name);

        if (strstr(dirbuf, "mp3"))
        {
            printf("The file path is:%s\n", dirbuf);
            insert_linklist(linklist_mp3, dirbuf);
            i++;
        }
    }
    closedir(dirp);
    return i;
}

void stop_music()
{

    display_jpeg_picture_to_framebuffer("../music_img/play.jpg", 240 + 150, 390, fb_addr, 1);
    printf("暂停\n");
    play_flag = 1;
    system("killall -STOP madplay &"); // 利用system函数调用killall命令将madplay终止掉
    printf("音乐已被终止\n");
}

void play_music()
{
    char command[100] = {0};
    printf("播放\n");
    display_jpeg_picture_to_framebuffer("../music_img/stop.jpg", 240 + 150, 390, fb_addr, 1);

    play_flag = 0;
    if (first_flag == 0)
    {
        sprintf(command, "madplay %s  -a -30 &", node_mp3->data);
        printf("%s\n", command);
        system(command);
        // 把首次播放标志置1
        first_flag = 1;
        printf("开始播放\n");
    }
    // 否则就恢复播放
    else
    {
        system("killall -CONT madplay &");
        printf("恢复播放\n");
    }

    // system("killall -9 madplay");//利用system函数调用killall命令将madplay终止掉
    printf("音乐已播放\n");
}

void prev_music()
{
    system("killall -9 madplay");
    first_flag = 0;
    node_mp3 = node_mp3->prev;
    printf("%s\n", node_mp3->data);
    char command[100] = {0};
    sprintf(command, "madplay %s -a -30 &", node_mp3->data);
    printf("%s\n", command);
    system(command);
    // 把首次播放标志置1
    first_flag = 1;
    printf("开始播放\n");
    printf("上一首\n");
}

void next_music()
{
    system("killall -9 madplay");
    first_flag = 0;
    node_mp3 = node_mp3->next;
    printf("%s\n", node_mp3->data);
    char command[100] = {0};
    sprintf(command, "madplay %s -a -30 &", node_mp3->data);
    printf("%s\n", command);
    system(command);
    // 把首次播放标志置1
    first_flag = 1;
    printf("下一首\n");
}

void music()
{
    int len;
    init_music();
    Linklist *linklist_mp3 = create_linklist(); // 创建链表
    len = get_music_url(linklist_mp3);          // 获取音乐文件路径
    printf("music_num:%d\n", len);
    int ts_x, ts_y;
    int press_x, press_y;
    int release_x, release_y;
    ts_fd = init_touchscreen_device(); // 初始化触摸屏设备
    int status;
    node_mp3 = linklist_mp3->first;
    while (1)
    {
        status = get_touchscreen_status(ts_fd, &ts_x, &ts_y);
        switch (status)
        {
        case TOUCH_COORDINATE_STATUS:
            break;
        case TOUCH_PRESS_STATUS:
            // 记录按下的触摸屏坐标
            press_x = ts_x;
            press_y = ts_y;
            break;
        case TOUCH_RELEASE_STATUS:
            release_x = ts_x;
            release_y = ts_y;
            // 判断按键范围//
            // 暂停播放音乐按键
            if (press_x >= 390 && release_x <= 390 + 92 && press_y >= 380 && release_y <= 380 + 90)
            {
                // 暂停音乐

                if (play_flag == 0)
                {
                    stop_music();
                }
                // 播放音乐按键
                else
                {
                    play_music();
                }
            }
            // 回退
            if (press_x >= 278 && release_x <= 278 + 112 && press_y >= 380 && release_y <= 380 + 90)
            {
                printf("回退\n");
            }
            // 快进
            if (press_x >= 482 && release_x <= 482 + 120 && press_y >= 380 && release_y <= 380 + 90)
            {
                printf("快进\n");
            }
            // 退出音乐播放器界面
            if (press_x >= 0 && release_x <= 80 && press_y >= 0 && release_y <= 50)
            {
                return;
                printf("返回\n");
            }
            // 上一首播放
            if (press_x >= 150 && release_x <= 150 + 128 && press_y >= 380 && release_y <= 380 + 90)
            {
                 prev_music();
            }
            // 下一首播放
            if (press_x >= 602 && release_x <= 602 + 120 && press_y >= 380 && release_y <= 380 + 90)
            {
                 next_music();
            }
        }
    }
}

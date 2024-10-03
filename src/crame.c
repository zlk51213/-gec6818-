#include <dirent.h>
#include <framebuffer.h>
#include <touchscreen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <bmp.h>            // bmp图片操作头文件
#include <linklist_file.h>    // 链表文件操作头文件
extern int ts_fd;             // 触摸屏文件描述符
extern int fb_fd;             // 帧缓冲区文件描述符
extern unsigned int *fb_addr; // 帧缓冲区地址
void myclear()
{
    int x, y;
    for (y = 0; y < 480; y++)
        for (x = 0; x < 800; x++)
            display_point_to_framebuffer(x, y, 0, fb_addr); // 清屏
}
int get_picture_url(Linklist *jpg_pic)
{
    DIR *dirp;
    struct dirent *direntp;
    // 添加寻找路径
    char *path = "../crame";
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

        if (strstr(dirbuf, "bmp") || strstr(dirbuf, "jpg"))
        {
            printf("The file path is:%s\n", dirbuf);
            insert_linklist(jpg_pic, dirbuf);
            i++;
        }
    }
    closedir(dirp);
    return i;
}
void crame()
{
    myclear(); // 清屏
    int pic_len;
    Linklist *jpg_pic = create_linklist();     // 创建图片链表
    pic_len = get_picture_url(jpg_pic); // 获取图片文件路径
    Node *node_pic = jpg_pic->first;
    printf("The number of pic is:%d\n", pic_len); // 输出图片文件数量

    // 监控触摸屏
    int status;
    int ts_x, ts_y;
    int press_x, press_y;
    int release_x, release_y;
    ts_fd = init_touchscreen_device(); // 初始化触摸屏设备
    display_jpeg_picture_to_framebuffer((char *)node_pic->data, 0, 0, fb_addr, 1);
    while (1)
    {
        status = get_touchscreen_status(ts_fd, &ts_x, &ts_y);
        switch (status)
        {
            break;
        case TOUCH_PRESS_STATUS:
            // printf("触摸屏被按下x=%d, y=%d\n", x, y);
            // 记录按下的触摸屏坐标
            press_x = ts_x;
            press_y = ts_y;
            break;
        case TOUCH_RELEASE_STATUS:
            // printf("触摸屏被松开x=%d, y=%d\n", ts_x, ts_x);
            release_x = ts_x;
            release_y = ts_y;
            if (abs(release_x - press_x) <= 30 && abs(release_y - press_y) <= 30)
            {
                    if (strstr(node_pic->data, "jpg"))
                {
                    display_jpeg_picture_to_framebuffer((char *)node_pic->data, 0, 0, fb_addr, 1.5);
                }
            }

            // 判断滑动方向
            if (abs(release_x - press_x) > abs(release_y - press_y))
            {
                if (release_x - press_x > 30)
                {

                    printf("右滑动\n");
                    
                    node_pic = node_pic->prev;
                    if (strstr(node_pic->data, "bmp"))
                    {
                        pic_circular_spread(node_pic->data, 0, 0);
                    }
                    else
                    {
                        myclear();
                        display_jpeg_picture_to_framebuffer((char *)node_pic->data, 0, 0, fb_addr, 1);
                    }
                }
                else if (press_x - release_x > 30)
                {

                    printf("左滑动\n");
                    node_pic = node_pic->next;
                    if (strstr(node_pic->data, "bmp"))
                    {
                        pic_circular_spread(node_pic->data, 0, 0);
                    }
                    else
                    {
                        myclear();
                        display_jpeg_picture_to_framebuffer((char *)node_pic->data, 0, 0, fb_addr, 1);
                    }
                }
            }
            else
            {
                if (release_y - press_y > 30)
                {
                    printf("下滑动\n");
                    return;
                }
                else if (press_y - release_y > 30)
                {
                    printf("上滑动\n");
                    if (strstr(node_pic->data, "jpg"))
                    {
                        myclear();
                        display_jpeg_picture_to_framebuffer((char *)node_pic->data, 0, 0, fb_addr, 1);
                    }
                }
            }
            break;
        default:
            break;
        }
    }
}

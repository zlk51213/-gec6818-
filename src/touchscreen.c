#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

int init_touchscreen_device(void)
{
    int ts_fd;

    ts_fd = open("/dev/input/event0", O_RDONLY);
    if(ts_fd == -1)
    {
        perror("打开触摸屏失败");
        return -1;
    }

    return ts_fd;
}

int destroy_touchscreen_device(int ts_fd)
{
    return close(ts_fd);
}

#define TOUCH_COORDINATE_STATUS     0
#define TOUCH_PRESS_STATUS          1
#define TOUCH_RELEASE_STATUS        2
#define TOUCH_ERROR_STATUS          -1

int get_touchscreen_status(int ts_fd, int *x_addr, int *y_addr)
{
    struct input_event ts_data;
    ssize_t rd_size;
    static int x, y;
    int status;

    while(1)
    {
        rd_size = read(ts_fd, &ts_data, sizeof(ts_data));//阻塞功能
        if(rd_size == -1)
        {
            perror("获取触摸屏数据失败");
            status = TOUCH_ERROR_STATUS;
            break;
        }

        if(ts_data.type == EV_ABS)//判断他是否是坐标事件
        {
            if(ts_data.code == ABS_X)//判断数据是否是x坐标
            {
                x = ts_data.value;
            }
            else if(ts_data.code == ABS_Y)//判断数据是否是y坐标
            {
                y =  ts_data.value;
            }
            status = TOUCH_COORDINATE_STATUS;
        }
        else if(ts_data.type == EV_KEY)//按键事件
        {
            if(ts_data.value == 1)
            {
                status = TOUCH_PRESS_STATUS;
            }
            else
            {
                status = TOUCH_RELEASE_STATUS;
            }
        }
        else//结束包
        {
            *x_addr = x;
            *y_addr = y;
            //printf("获取结束\n");
            break;
        }

    }

    return status;
}

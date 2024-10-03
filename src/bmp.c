#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <framebuffer.h>
#include <touchscreen.h>

struct bmp_color
{
    char blue;
    char green;
    char red;
}; // 用来代表bmp图片当中的一个像素点大小

extern unsigned int *fb_addr;

int pic_circular_spread(char pathname[128], int startx, int starty)
{
    int pic_fd;
    ssize_t rd_size;

    pic_fd = open(pathname, O_RDONLY);
    if (pic_fd == -1)
    {
        perror("打开图片失败啦");
        return -1;
    }

    // lseek(pic_fd, 54, SEEK_SET);//跳过前面54字节头部信息
    char head_info[54];

    read(pic_fd, head_info, 54);

    int width, height;

    width = *(int *)&head_info[18];
    height = *(int *)&head_info[22];

    printf("宽度：%d, 高度：%d\n", width, height);

    struct bmp_color buffer[480][800];
    unsigned int color;
    int x, y;
    rd_size = read(pic_fd, buffer, sizeof(buffer));
    int k;
    for(k=0; k<467; k+=3)//8、特殊动画“圆形扩散”效果算法
    {
    	for(y=0; y<480; y++)
    	{
    		for(x=0; x<800; x++)
    		{
    			if((x-400)*(x-400)+(y-240)*(y-240) <= k*k)
    			{
            color = 0x00<<24 | buffer[y][x].red<<16 | buffer[y][x].green<<8 |buffer[y][x].blue;//A<<24 | R<<16 | G<<8 | B->ARGB

            display_point_to_framebuffer(x, 479-y, color, fb_addr);
    			}
    		}
    	}
    }
	// for(k=468; k>=0; k-=3)
	// {
	// 	for(y=0; y<480; y++)
	// 	{
	// 		for(x=0; x<800; x++)
	// 		{
	// 			if((x-400)*(x-400)+(y-240)*(y-240) >= k*k)
    //         color = 0x00<<24 | buffer[y][x].red<<16 | buffer[y][x].green<<8 |buffer[y][x].blue;//A<<24 | R<<16 | G<<8 | B->ARGB

    //         display_point_to_framebuffer(x, 479-y, color, fb_addr);
	// 		}
	// 	}
	// }	
        close(pic_fd);
        return 0;
}

    int bmpshow(char pathname[128], int startx, int starty)
    {
        int pic_fd;
        ssize_t rd_size;

        pic_fd = open(pathname, O_RDONLY);
        if (pic_fd == -1)
        {
            perror("打开图片失败啦");
            return -1;
        }

        // lseek(pic_fd, 54, SEEK_SET);//跳过前面54字节头部信息
        char head_info[54];

        read(pic_fd, head_info, 54);

        int width, height;

        width = *(int *)&head_info[18];
        height = *(int *)&head_info[22];

        printf("宽度：%d, 高度：%d\n", width, height);

        struct bmp_color buffer[480][800];
        unsigned int color;
        int x, y;

        rd_size = read(pic_fd, buffer, sizeof(buffer));

        for (y = 0; y < height && y < 480; y++)
        {
            for (x = 0; x < width && x < 800; x++)
            {
                color = 0x00 << 24 | buffer[y][x].red << 16 | buffer[y][x].green << 8 | buffer[y][x].blue; // A<<24 | R<<16 | G<<8 | B->ARGB

                display_point_to_framebuffer(x, 479 - y, color, fb_addr);
            }
        }

        close(pic_fd);
        return 0;
    }

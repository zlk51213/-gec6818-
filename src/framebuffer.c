#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
//初始化显存设备
unsigned int *init_framebuffer_device(int *fd_arg)
{
	int fb_fd;
	unsigned int *fb_addr;

	fb_fd = open("/dev/fb0", O_RDWR);//读写的方式打开显存文件
	if(fb_fd == -1)
	{
		perror("打开显存文件失败");
		return NULL;
	}
	
	fb_addr = mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);//获取显存映射到虚拟内存的地址
	if(fb_addr == MAP_FAILED)
	{
		perror("映射显存内存失败\n");
		close(fb_fd);
		return NULL;
	}

	*fd_arg = fb_fd;

	return fb_addr;
}
//销毁	显存设备
int destroy_framebuffer_device(int fb_fd, unsigned int *fb_addr)
{
	munmap(fb_addr, 800*480*4);
	
	return close(fb_fd);

}

//在显存中显示一个点
static inline void display_point_to_framebuffer(int x, int y, unsigned int color, unsigned int *fb_addr)
{
	*(fb_addr+800*y+x) = color;
}
//在显存中显示一个矩形
void display_frame_to_framebuffer(unsigned int *frame_addr, unsigned int *fb_addr)
{
	memcpy(fb_addr, frame_addr, 800*480*4);
}

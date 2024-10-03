#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

extern unsigned int *init_framebuffer_device(int *fd_arg);
extern int destroy_framebuffer_device(int fb_fd, unsigned int *fb_addr);
extern void display_frame_to_framebuffer(unsigned int *frame_addr, unsigned int *fb_addr);

extern int display_jpeg_picture_to_framebuffer(const char *filename, unsigned int x_s, unsigned int y_s, unsigned int *screen_fb_addr, float mul);
static inline void display_point_to_framebuffer(int x, int y, unsigned int color, unsigned int *fb_addr)
{
	*(fb_addr+800*y+x) = color;
}



#endif
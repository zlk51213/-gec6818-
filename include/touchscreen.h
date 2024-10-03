#ifndef _TOUCHSCREEN_H
#define _TOUCHSCREEN_H

#define TOUCH_COORDINATE_STATUS     0
#define TOUCH_PRESS_STATUS          1
#define TOUCH_RELEASE_STATUS        2
#define TOUCH_ERROR_STATUS          -1

extern int init_touchscreen_device(void);
extern int destroy_touchscreen_device(int ts_fd);
extern int get_touchscreen_status(int ts_fd, int *x_addr, int *y_addr);


#endif
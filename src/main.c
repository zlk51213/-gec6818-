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
#include "procedure.h"
int main(void)
{
init_procedure();//启动程序初始化
destroy_procedure();//程序退出时销毁
}
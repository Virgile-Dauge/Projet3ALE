
#include <linux/ioctl.h>
#define MAJOR_NUM 197
#define GET_DIST _IOR(MAJOR_NUM, 0, int)
#define SET_VAL _IOW(MAJOR_NUM, 1, int)

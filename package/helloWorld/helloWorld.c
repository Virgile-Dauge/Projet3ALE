#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sonar.h"

int main() {
	char message[100];
	printf("Hello World!\n");
	int dist =0,err =0;
	errno = 0;
	int sonar = open("/dev/sonar",O_RDWR);
	perror("plop");
	errno = 0;
	err = ioctl(sonar, GET_DIST, &dist);
	//ioctl(sonar, SET_VAL, 42);
	perror("plop");
	printf("dist : %d\nerr : %d\n",dist,err);
	close(sonar);
	return 0;
}

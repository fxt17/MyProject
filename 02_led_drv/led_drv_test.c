
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
 * ./hello_drv_test -w abc
 * ./hello_drv_test -r
 */
int main(int argc, char **argv)
{
	int fd;
	char buf[1024];
	int len;
	
	/* 1. 判断参数 */
	if (argc < 2) 
	{
		printf("Usage: %s -w <string>\r\n", argv[0]);
		printf("       <string>	1 : led off\r\n");
		printf("       <string>	0 : led on\r\n");
		printf("       %s -r\r\n", argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open("/dev/led_drv", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/led_drv\r\n");
		return -1;
	}

	/* 3. 写文件或读文件 */
	if ((0 == strcmp(argv[1], "-w")) && (argc == 3))
	{
		len = strlen(argv[2]) + 1;
		len = len < 1024 ? len : 1024;
		write(fd, argv[2], len);
	}
	else
	{
		len = read(fd, buf, 1024);		
		buf[1023] = '\0';
		printf("APP read : %s\r\n", buf);
		if(buf[0]==0)
		{
			printf("LED on!\r\n");
		}
		else
		{
			printf("LED off!\r\n");
		}
	}
	
	close(fd);
	
	return 0;
}



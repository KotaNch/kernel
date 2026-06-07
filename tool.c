#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h> 

#define MY_DEVICE_CLEAR _IO('k', 1)  

int main(void) {
    int fd = open("/dev/simpele_char_dev", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    ioctl(fd, MY_DEVICE_CLEAR);
    printf("Buffer cleared!\n");
                                   
    close(fd);
    return 0; 
                                                                                   
 
}

                                              

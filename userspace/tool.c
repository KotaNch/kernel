#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/cyclic_buffer"

#define CYCLIC_IOC_MAGIC 'k'
#define CYCLIC_IOC_CLEAR _IO(CYCLIC_IOC_MAGIC,1)
#define CYCLIC_IOC_AVAILABLE  _IOR(CYCLIC_IOC_MAGIC, 2, int)

static void usage(const char *prog){
    fprintf(stderr,
        "Usage: \n"
        " %s clear\n"
        " %s write <text\n"
        " %s read <bytes>\n",
        " %s avail\n",
        prog,prog,prog,prog
    );
}

static int open_device(void){
    int fd = open(DEVICE_PATH, O_RDWR);

    if (fd < 0){
        fprintf(stderr,"open %s: %s\n", DEVICE_PATH,strerror(errno));
    }
    return fd;
}

static int clear_buffer(int fd){
    if (ioctl(fd, CYCLIC_IOC_CLEAR) <0){
        fprintf(stderr,"ioctl clear: %s\n", strerror(errno));
        return 1;
    }
    puts("Buffer cleared");
    return 0;
}

static int available_buffer(int fd){
    int avail = 0;

    if (ioctl(fd, CYCLIC_IOC_AVAILABLE, &avail)<0){
        fprintf(stderr,"ioctl avail: %s\n", strerror(errno));
        return 1;
    }
    printf("avaliable: %d bytes\n",avail);
    return 0;
}

static int write_buffer(int fd,const char *text){
    ssize_t written = write(fd,text, strlen(text));

    if (written < 0){
        fprintf(stderr, "write: %s\n", strerror(errno));
        return 1;
    }
    printf("written: %zd bytes\n",written);
    return 0;
}

static int read_buffer(int fd, const char *count_arg){
    char *end = NULL;
    long requested = strtol(count_arg,&end,10);
    char *buf;
    ssize_t received;

    if (*count_arg == '\0' || *end != '\0' || requested <= 0){
        fprintf(stderr, "read bytes must be a positive integer\n");
        return 1;
    }
    buf = calloc((size_t)requested + 1,sizeof(*buf));
    if (!buf){
        fprintf(stderr,"calloc: %s\n",strerror(errno));
        return 1;
    }

    received = read(fd, buf, (size_t)requested);
    if (received < 0 ){
        fprintf(stderr, "read: %s\n", strerror(errno));
        free(buf);
        return 1;
    }
    printf("read: %zd bytes\n",received);
    if (received > 0){
        printf("%.*s\n",(int)received,buf);
    }

    free(buf);
    return 0;
}

int main(int argc,char **argv) {
    int fd;
    int ret;

    if (argc < 2){
        usage(argv[0]);
        return 1;
    }
    fd = open_device();
    if (fd < 0){
        return 1;
    }

    if (strcmp(argv[1],"clear") == 0 && argc == 2){
        ret = clear_buffer(fd);
    }else if (strcmp(argv[1], "write") == 0 && argc == 3){
        ret = write_buffer(fd, argv[2]);
    }else if (strcmp(argv[1], "read") == 0 && argc == 3) {
        ret = read_buffer(fd, argv[2]);
    }else if (strcmp(argv[1], "avail") == 0 && argc == 2) {
        ret = available_buffer(fd);
    }else {
        usage(argv[0]);
        ret = 1;
    }
    close(fd);
    return ret;
}
 
                                              

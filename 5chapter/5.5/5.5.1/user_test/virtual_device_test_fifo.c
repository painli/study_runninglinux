#include <stdio.h>
#include <fcntl.h>



#define DEMO_DEV_NAME "/dev/my_demo_dev"

int main(){
        char buffer[64];
        int fd;
        int ret;
        size_t len;
        char message[80] = "Testing the virtual FIFO device";
        char* read_buffer;



        len = sizeof(message);
        read_buffer = malloc(2*len);
        memset(read_buffer,0,2*len);

        fd = open(DEMO_DEV_NAME,O_RDWR | O_NONBLOCK);
        if(fd < 0){
            printf("open device: %s failed\n",DEMO_DEV_NAME);
            return -1;
        }

        ret = read(fd,read_buffer,2*len);
        printf("read %d bytes\n",ret);
        printf("read buffer=%s\n",read_buffer);

        /* 1. write the message to device*/
        ret = write(fd,(char *)message,len);
        if(ret <= 0){
                printf("can not write on device:%d, ret = %d\n",fd,ret);
        }

        ret = write(fd,(char *)message,len);
        if(ret <= 0){
                printf("can not write on device:%d, ret = %d\n",fd,ret);
        }


        ret = read(fd,read_buffer,2*len);
        printf("read %d bytes\n",ret);
        printf("read buffer=%s\n",read_buffer);


        close(fd);
    return 0;
}

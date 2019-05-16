CC := arm-linux-gnueabi-gcc
CFLAGS += --static


TARGET := virtual_device_test_fifo
DEPEND := virtual_device_test_fifo.c


all : $(DEPEND) 
	$(CC) -o $(TARGET) $(CFLAGS) $^ 
	cp $(TARGET) ~/code/kernel4.0/kmodules/
#%.o : %.c
#	$(CC) -o $@  $^

.PHONY : clean


clean:
	rm -rf $(TARGET)

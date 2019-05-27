CC := arm-linux-gnueabi-gcc
CFLAGS += --static


TARGET := 5.6_test
DEPEND := test.c


all : $(DEPEND) 
	$(CC) -o $(TARGET) $(CFLAGS) $^ 
	cp $(TARGET) ~/code/kernel4.0/kmodules/
#%.o : %.c
#	$(CC) -o $@  $^

.PHONY : clean


clean:
	rm -rf $(TARGET)

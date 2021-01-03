MODULE_DIR=example
MODULE_DIR2=
SRC_SOURCE=*.c $(MODULE_DIR)/*.c $(MODULE_DIR2)/*.c
CC=gcc
CFLAGS_WARN = -g

LIBPATH = -L. \
			-L/home/yswan/install-ffmpeg/lib
LIBVAR = -lavformat -lavcodec -lavutil -lswresample -pthread -lm -lz
DEFINE =
INCLUDE = -I. \
		-I/home/yswan/install-ffmpeg/include

SOURCE = $(wildcard $(SRC_SOURCE))
OBJS = $(patsubst %.c,%.o,$(SOURCE))

TARGET = a.out

ALL:$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBPATH) $(LIBVAR)
	rm -rf $(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS_WARN) $(DEFINE) $(INCLUDE) $< -o $@
clean:
	rm -rf $(OBJS)
	rm -rf $(TARGET)
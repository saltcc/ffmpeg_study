MODULE_DIR=
MODULE_DIR2=
SRC_SOURCE=*.c $(MODULE_DIR)/*.c $(MODULE_DIR2)/*.c
CC=gcc
CFLAGS_WARN = -g

LIBPATH = -L. \
			-L/usr/local/ffmpeg/lib
LIBVAR = -lavformat -lavcodec -lz -lswresample  -lavutil -pthread -lm
DEFINE =
INCLUDE = -I. \
		-I/usr/local/ffmpeg/include

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
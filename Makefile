

CC = g++

SRCS = dragonfly.cc \
	   setup.cc \


OBJS = $(SRCS:.cc=.o)

EXEC = dragonfly

CFLAGS = -g -Wall -O2 

LIBS = -levent -lpthread -lglog

INCS = -I /usr/local/include

LIBINCS = -L /usr/local/lib

ARGS = $(LIBS) $(INCS) $(LIBINCS)

start:$(OBJS)
	$(CC) -o  $(EXEC) $(CFLAGS) $(OBJS) $(ARGS)
.cc.o:
	$(CC) -o $@ -c $< $(ARGS) $(CFLAGS)

.c.o:
	$(CC) -o $@ -c $< $(ARGS) $(CFLAGS)

#创建库文件
#命名规则强制libXXX.so
#在.o的语句中加上-fPIC    位置无关代码
#在库的生成语句中添加  -shared

clean:
	rm -rf $(OBJS) $(EXEC)

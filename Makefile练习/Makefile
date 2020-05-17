# 阶段一
#app:add.c sub.c dive.c mul.c main.c
#	gcc add.c sub.c dive.c mul.c main.c -o app

# 阶段二
#app:add.o sub.o dive.o mul.o main.o
#	gcc add.o sub.o dive.o mul.o main.o -o app
#add.o:add.c
#	gcc -c add.c
#sub.o:sub.c
#	gcc -c sub.c
#dive.o:dive.c
#	gcc -c dive.c
#mul.o:mul.c
#	gcc -c mul.c
#main.o:main.c
#	gcc -c main.c

# 阶段三
# $@表示目标, $^表示所有依赖, $<表示依赖中的第一个

#src = $(wildcard *.c)
#obj = $(patsubst %.c, %.o, $(src))
#target = app
#
#$(target):$(obj)
#	gcc $^ -o $@
#%.o:%.c
#	gcc -c $< -o $@

# 阶段四

# CPPFLAGS 预处理时的参数 Compile pre-process 编译预处理
# CFLAGS 编译时的参数 Compile 编译
# LDFLAGS 加载时的参数(共享库) Load 装载,加载
# CC 用的编译器是什么

CPPFLAGS = -Iinclude
CFLAGS = -g -Wall
LDFLAGS = -lpthread
CC = gcc

src = $(wildcard *.c)
obj = $(patsubst %.c, %.o, $(src))
target = app

$(target):$(obj)
	$(CC) $^ $(LDFLAGS) -o $@
%.o:%.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@
.PHONY:clean
# 清除生成的过程文件
clean:
	-rm -f *.o
	-rm -f app
# 彻底清除过程文件和配置文件
distclean:
	rm /usr/bin/app
install:
	cp app /usr/bin/

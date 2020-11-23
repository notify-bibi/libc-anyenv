override CFLAGS+=-Wall -g3 -fPIC
CC=clang 
CXX=clang++ 
LLVM_CONFIG=llvm-config


OBJS=std2socket32.o std2socket64.o
PREFIX=/usr/local

all: std2socket32.so std2socket64.so

std2socket32.o: std2socket.c
	$(CC) $(CFLAGS) -m32 -c $^ -o $@
std2socket64.o: std2socket.c
	$(CC) $(CFLAGS) -m64 -c $^ -o $@

std2socket32.so: std2socket32.o
	$(LINK.c) -shared -m32 $^ -o $@

std2socket64.so: std2socket64.o
	$(LINK.c) -shared -m64 $^ -o $@


clean:
	rm -f std2socket64.so std2socket64.d std2socket32.so std2socket32.d $(OBJS)


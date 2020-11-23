
#define _GNU_SOURCE  
#include<stdio.h>
#include <unistd.h>
#include <fcntl.h>


int main(){
	int fdf = fcntl(STDOUT_FILENO, F_GETFD);
	int fdfl = fcntl(STDOUT_FILENO, F_GETFL);


	fcntl(STDOUT_FILENO, F_SETFL, 0x402);

	printf("%p\n",  O_RDWR|O_APPEND|O_LARGEFILE);
	int n = puts("hello\n");
	puts("word ");

	int fdfc = fcntl(STDOUT_FILENO, F_GETFD);
	int fdflc = fcntl(STDOUT_FILENO, F_GETFL);

	printf("%d [%p %p | %p %p]", n, fdf, fdfl, fdfc, fdflc);
	write(1, "gggggggg\n", 9);
	getchar();
}

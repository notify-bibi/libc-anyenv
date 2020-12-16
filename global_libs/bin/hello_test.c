#include <stdio.h>

int main(int argn, char** argc){

	void* p1 = malloc(0x20);
	void* p2 = malloc(0x20);
	void* p3 = malloc(0x20);
	free(p1);
	free(p2);
	free(p3);
    printf("well done!\n sizeof(size_t) = %d\n",sizeof(size_t));

}


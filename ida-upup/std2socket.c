#define _GNU_SOURCE  
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sched.h>
#include <signal.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>

static int gsfd = 0;
static int (*func_fork) () = NULL;
static size_t (*func_strspn)(const char *, const char *) = NULL;

// const char debuginfopath[] = "/usr/lib/debug/;";
const char debuginfopath[] = \
"/usr/lib/debug/;"
"/lib/x86_64-linux-gnu;"
"/lib;"
"/lib64;"
"/usr/lib;"
"/usr/lib64;"
"/usr/x86_64-linux-gnu/lib;"
"/usr/x86_64-linux-gnu/lib64;"
"/usr/local/lib;"
"/usr/local/lib64;"
"/usr/lib/x86_64-linux-gnu;"
"/usr/lib/x86_64-linux-gnu64;"
"/usr/local/lib/x86_64-linux-gnu";


const char DWARF_sign[2] = { ";" };

static pid_t no_cache_getpid()
{
    pid_t pid;

#if defined(__x86_64__) || defined(__arm__) || defined(__powerpc__)
    pid = syscall(__NR_getpid);
#elif defined(__i386__)
    __asm__(
        "int $0x80"
        :
            "=a"(pid)
        :
            "a"(__NR_getpid)
        );
#endif

    return pid;
}

int srv_listen()
{
    struct addrinfo hint, *result;
    int res, sfd;

    memset(&hint, 0, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = 0;
    hint.ai_flags    = AI_PASSIVE;

    res = getaddrinfo(NULL, "12345", &hint, &result);
    if (res != 0) {
        perror("error : cannot get socket address!\n");
        exit(1);
    }

    sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sfd == -1) {
        perror("error : cannot get socket file descriptor!\n");
        goto faild;
    }
    int option = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	fcntl(sfd, F_SETFD, fcntl(sfd, F_GETFD, 0) | FD_CLOEXEC);

    if (getenv("BLOCK") != NULL) {
		fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) & (~O_NONBLOCK));
		printf("[*] blocking mode\n");
	}else{
		fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK);
		printf("[*] non-blocking mode\n");
	}



	if (getenv("LD_PRELOAD") != NULL) {
		if (strstr(getenv("LD_PRELOAD"), "std2socket") > 0 ){
			unsetenv("LD_PRELOAD");
			printf("unsetenv(\"LD_PRELOAD\");\n");
		}
	}
    if (getenv("BLOCK") != NULL) {
        unsetenv("BLOCK");
    }



    res = bind(sfd, result->ai_addr, result->ai_addrlen);
    if (res == -1) {
        perror("error : cannot bind the socket with the given address!\n");
        goto faild;
    }

    res = listen(sfd, SOMAXCONN);
    if (res == -1) {
        perror("error : cannot listen at the given socket!\n");
        goto faild;
    }
    printf("[*] Listening on 0.0.0.0:12345...\n");
   
    return sfd;
faild:
    close(sfd);
    shutdown(sfd, SHUT_RDWR);
    exit(1);
    return -1;
}


int srv_accept(int sfd){

    struct sockaddr remote;
    socklen_t len = sizeof(struct sockaddr);
    int fd = accept(sfd, &remote, &len);
    return fd;
}


pid_t fork(){
	pid_t rval, tid;

    if(!gsfd) gsfd = srv_listen();
    if (!func_fork) func_fork =  (int (*) ()) dlsym (RTLD_NEXT, "fork");

	int fd = srv_accept(gsfd);
	printf("[*] accept: %d fd:%x fl:%x\n", fd, fcntl(fd, F_GETFD), fcntl(fd, F_GETFL));


    fcntl(fd, F_SETFD, 0);
	fcntl(fd, F_SETFL, 0x8402 | FD_CLOEXEC);


	tid = no_cache_getpid();
#if 0
	// rval = syscall(__NR_clone, CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, 0, 0, &tid);
    // CLONE_VM|CLONE_VFORK
    rval = syscall(__NR_clone, CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, 0, 0, &tid);
#else
	rval = func_fork();
#endif
	
	if (fd<0) return rval;

	if (rval < 0){
		close(fd);
		perror("error : cannot fork! \n");
        exit(rval);
	}else if (rval == 0){
		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
	}else{
		printf("[*] p:%d fork cid:%d\n", no_cache_getpid(), rval);
	}
    close(fd);
	return rval;
}

void handler(char *caller) {
  void *array[10];
  size_t size;
  printf("Stack Trace Start for %s\n",caller);
  size = backtrace(array, 10);
  backtrace_symbols_fd(array, size, 2);
  printf("Stack Trace End\n");
}

static int count = 0;
size_t strspn(const char *s, const char *accept){
    // handler("strspn");
    if (!func_strspn) func_strspn =  (size_t (*)(const char *, const char *)) dlsym (RTLD_NEXT, "strspn");
    
    if (*(short*)accept == *(short*)DWARF_sign){
        if(!*s && !count){
            printf("[*] %d attach strspn orig debuginfo:[ %s ]  changed!\n", count, s);
            strcpy((char *)(size_t)s, debuginfopath);
            count += 1;
        }else if(!*s && count){
            count = 0;
        }else{
            count += 1;
        }
        
        // printf("[*] %d %p :[ %s ]\n",count, s, s);
    }
    size_t ret = func_strspn(s, accept);
    return ret;
}




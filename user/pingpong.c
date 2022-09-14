#include "kernel/types.h"
#include "user.h"

int main(int argc,char* argv[]){
    int fpid;
    int pipe1[2], pipe2[2];
    pipe(pipe1); //默认0下标为读端口，1为写端口
    pipe(pipe2);
    //管道1用于父亲写，儿子读；管道2用于儿子写，父亲读。

    fpid = fork();//草草，注意别在奇怪的地方fork，之前先fork然后再定义pipe，检查了一年

    if(fpid != 0) { // 这玩意是父亲进程
        char buf[123];
        close(pipe2[1]);
        close(pipe1[0]);

        write(pipe1[1], "1", 1);//传进去
        read(pipe2[0], buf, 1);
        printf("%d: received pong\n", getpid());
        wait(0);

    }
    else { // 这玩意是儿子进程 
        char buf[123];
        close(pipe1[1]);
        close(pipe2[0]);

        read(pipe1[0], buf, 1);
        printf("%d: received ping\n", getpid());
        write(pipe2[1], buf, 1);
        exit(0);
    }
    exit(0);
}
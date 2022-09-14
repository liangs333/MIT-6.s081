#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void solve(int pipeIn[2]) { 
    int pipeOut[2];
    int baseValue;

    read(pipeIn[0], &baseValue, (int)sizeof(baseValue));
//    printf("PipeValue %d %d\n", pipeIn[0], pipeIn[1]);
    
	pipe(pipeOut);//因为没加这句话坐牢30min，我是傻逼
    

//    printf("READ value = %d\n", baseValue);
    if(baseValue == -233) { 
//        printf("EXIT");
        exit(0);
    } 
    
    printf("prime %d\n", baseValue);

    if(fork() != 0) { //father
        close(pipeOut[0]);
        int nowValue;
        while(read(pipeIn[0], &nowValue, (int)sizeof(nowValue))) {
//            printf("value = %d\n", nowValue);
//            printf("ReadValue %d\n", nowValue);
            if(nowValue == -233)
                break;
            if(nowValue % baseValue != 0) {
//                printf("pushdown %d %d\n", baseValue, nowValue);
                write(pipeOut[1], &nowValue, (int)sizeof(nowValue));
            }
        }
        nowValue = -233;
        write(pipeOut[1], &nowValue, (int)sizeof(nowValue));

        wait(0);
        exit(0);
    }
    else { //child
//        sleep(20);
        close(pipeIn[0]);
        close(pipeOut[1]);
        solve(pipeOut);
    }
} 

int main(int argc,char* argv[]){
    int originPipe[2];
    pipe(originPipe);
    if(fork() != 0) {
        close(originPipe[0]);

        for(int i=2;i<=35;++i) {
            write(originPipe[1], &i, (int)sizeof(i));
        }
        int end = -233;
        write(originPipe[1], &end, (int)sizeof(end));        

        wait(0);
        exit(0);
    }
    else {
        close(originPipe[1]);
        solve(originPipe);
        wait(0);
        exit(0);
    }
}

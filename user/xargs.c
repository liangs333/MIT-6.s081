#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char* argv[]){
    char *command = argv[1];
    char buf;
    char params[MAXARG][100]; //从零开始的
    char *to_exec[MAXARG];

    while(1) { 
        int argCount = argc - 1;
        memset(params, 0, sizeof(params));
        for(int i = 1; i <= argCount; ++ i) {
            strcpy(params[i - 1], argv[i]);
        }
        //params [0, argcount)
//        printf("Param[0] = %s \n", params[0]);
        int readTag;
        int inputed = 0, wordCount = argCount, length = 0;
        while((readTag = read(0, &buf, 1)) > 0 && buf != '\n') { 
//            printf("read %c     wordcount = %d\n", buf, wordCount);
            if(buf != ' ') {
                params[wordCount][length++] = buf;
                inputed = 1;
            }
            else if(buf == ' ' && inputed == 1) {
                params[wordCount][length++] = 0;
                ++wordCount;
                inputed = 0, length = 0;
            }
            else { 
                //一大车空格，do nothing
            }
        } 
        
//        printf("END of While");

        // if(buf == '\n') {
        //     printf("Got end of line\n");
        // }

        if(readTag <= 0) { 
        // end of file
            // printf("EOF, out");
            break;
        } 
        
        for(int i = 0; i < MAXARG; ++i) { 
            to_exec[i] = params[i];
        } 
//        printf("Param[0] = %s \n", params[0]);
//        printf("command = %s\n", command);
        to_exec[MAXARG - 1] = 0;
        if(fork() == 0) {
            // printf("intofork\n");
//            printf("try command %s\n", command);
//             for(int i = 0; i < MAXARG; ++i) {
//                 printf("%s\n", params[i]);
// //                printf("Param[0] = %s \n", params[0]);
//             }
            exec(command, to_exec);
        }
        else {
            wait(0);
        }
    } 
    exit(0);
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    memset(buf, 0, sizeof(buf));
    memmove(buf, p, strlen(p) + 1);
    //原版直接贴过来会出问题，不知道为啥，小修了一下（
    return buf;
}

void find(char *path, char *fileName) //不管这个东西是个文件还是个目录，都能调用
{
//    printf("Into find %s %s\n", path, fileName);
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) //fd是文件描述符
    {
        // fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) //st现在装有该文件的信息了
    {
        // fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    strcpy(buf, path); //当前文件的path装进buf里
    p = buf + strlen(buf);
    *p++ = '/';//加上斜杠，为之后做准备

    switch (st.type)
    {
    case T_FILE:
        //如果path对应的是个文件
//        printf("This is file %s\n", path);
//        printf("%s  %s  %d   %d  %d\n", fmtname(path), fileName, strcmp(fmtname(path), fileName), strlen(fileName), strlen(fmtname(path)));
        if (strcmp(fmtname(path), fileName) == 0)
        {
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        //如果path对应的是个目录
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) //看起来，目录里装的是文件名的信息，一行一个，用read读进de里头。
        {
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) //de.inum是个啥不知道，抄的ls，后面用来防止转圈递归
                continue;
            memmove(p, de.name, DIRSIZ);//拷贝一下路径
            p[DIRSIZ] = 0;//终止符
            find(buf, fileName);
        }
        break;
    }

    close(fd);
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("find ERROR, 2 params needed.\n");
        exit(0);
    }
    char *path = argv[1], *fileName = argv[2];
    find(path, fileName);
    exit(0);
}

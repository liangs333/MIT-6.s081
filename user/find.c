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
    //ԭ��ֱ��������������⣬��֪��Ϊɶ��С����һ�£�
    return buf;
}

void find(char *path, char *fileName) //������������Ǹ��ļ����Ǹ�Ŀ¼�����ܵ���
{
//    printf("Into find %s %s\n", path, fileName);
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) //fd���ļ�������
    {
        // fprintf(2, "ls: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) //st����װ�и��ļ�����Ϣ��
    {
        // fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }

    strcpy(buf, path); //��ǰ�ļ���pathװ��buf��
    p = buf + strlen(buf);
    *p++ = '/';//����б�ܣ�Ϊ֮����׼��

    switch (st.type)
    {
    case T_FILE:
        //���path��Ӧ���Ǹ��ļ�
//        printf("This is file %s\n", path);
//        printf("%s  %s  %d   %d  %d\n", fmtname(path), fileName, strcmp(fmtname(path), fileName), strlen(fileName), strlen(fmtname(path)));
        if (strcmp(fmtname(path), fileName) == 0)
        {
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        //���path��Ӧ���Ǹ�Ŀ¼
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) //��������Ŀ¼��װ�����ļ�������Ϣ��һ��һ������read����de��ͷ��
        {
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) //de.inum�Ǹ�ɶ��֪��������ls������������ֹתȦ�ݹ�
                continue;
            memmove(p, de.name, DIRSIZ);//����һ��·��
            p[DIRSIZ] = 0;//��ֹ��
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

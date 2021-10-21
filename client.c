#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h> // for dirname()/basename()
#include <time.h>

#define MAX 4096
#define PORT 35535

char line[MAX], ans[MAX];
int n;

struct sockaddr_in saddr;
int sfd;

int lcat(char *fileName);
int lls(char *dirName);
int lcd(char *dirName);
int lpwd(char *dirName);
int lmkdir(char *dirName);
int lrmdir(char *dirName);
int lrm(char *fileName);

char *cmd[] = {"lcat", "lls", "lcd", "lpwd", "lmkdir", "lrmdir", "lrm", NULL};
int (*fptr[])(char *) = {(int (*)())lcat, lls, lcd, lpwd, lmkdir, lrmdir, lrm};

int find_cmd(char *command)
{
    int i = 0;
    while (cmd[i])
    {
        if (strcmp(command, cmd[i]) == 0)
            return i;
        i++;
    }
    return -1;
}

int lcat(char *fileName)
{
    int fd;
    int i, n;
    char buf[4096];
    if (fileName[0] == '\0')
        return -1;

    fd = open(fileName, O_RDONLY);
    if (fd < 0)
        return -2;

    while (n = read(fd, buf, 4096))
        write(1, buf, n);

    return 0;
}

int ls_file(char *fname)
{
    struct stat fstat, *sp;
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";
    int r, i;
    char ftime[64];
    sp = &fstat;

    if ((r = lstat(fname, &fstat)) < 0)
    {
        printf("can't stat %s\n", fname);
        return -1;
    }

    if ((sp->st_mode & 0xF000) == 0x8000)
        printf("%c", '-');
    if ((sp->st_mode & 0xF000) == 0x4000)
        printf("%c", 'd');
    if ((sp->st_mode & 0xF00) == 0xA000)
        printf("%c", 'l');
    for (i = 8; i >= 0; i--)
    {
        if (sp->st_mode & (1 << i))
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]);
    }

    printf("%4ld ", sp->st_nlink);
    printf("%4d ", sp->st_gid);
    printf("%4d ", sp->st_uid);
    printf("%8ld ", sp->st_size);

    strcpy(ftime, ctime(&sp->st_ctime));
    ftime[strlen(ftime) - 1] = 0;
    printf("%s  ", ftime);

    printf("%s", basename(fname));
    if ((sp->st_mode & 0xF000) == 0XA000)
    {
        char buf[512];
        readlink(fname, buf, 512);
        printf(" -> %s", buf);
    }
    printf("\n");

    return 0;
}

int ls_dir(char *dname)
{
    struct dirent *de;
    DIR *dr = opendir(dname);
    if (!dr)
    {
        printf("Could not open %s\n", dname);
        return -1;
    }

    while ((de = readdir(dr)))
        ls_file(de->d_name);

    closedir(dr);
    return 0;
}

int lls(char *dirName)
{
    struct stat mystat, *sp = &mystat;
    int r;
    char path[1024], cwd[256];
    char *dfn = "./";
    if (dirName[0] != '\0')
        dfn = dirName;
    if (r = lstat(dfn, sp) < 0)
    {
        printf("no such file %s\n", dfn);
        return -1;
    }

    strcpy(path, dfn);
    if (path[0] != '/')
    {
        getcwd(cwd, 256);
        strcpy(path, cwd);
        strcat(path, "/");
        strcat(path, dfn);
    }

    if (S_ISDIR(sp->st_mode))
        ls_dir(path);
    else
        ls_file(path);

    return 0;
}

int lcd(char *dirName)
{
    int r = chdir(dirName);
    return 0;
}

int lpwd(char *dirName)
{
    char cwd[256];
    getcwd(cwd, 256);
    printf("%s\n", cwd);
    return 0;
}
int lmkdir(char *dirName)
{
    return (mkdir(dirName, 0744));
}

int lrmdir(char *dirName)
{
    return (rmdir(dirName));
}

int lrm(char *fileName)
{
    return (remove(fileName));
}

int main(int argc, char *argv[], char *env[])
{
    int n;
    char how[64];
    int i;

    printf("1. create a socket\n");
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        printf("socket creation failed\n");
        exit(0);
    }

    printf("2. fill in server IP and port number\n");
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(PORT);

    printf("3. connect to server\n");
    if (connect(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }

    printf("********  processing loop  *********\n");
    while (1)
    {
        printf("input a line : ");
        bzero(line, MAX);        // zero out line[ ]
        fgets(line, MAX, stdin); // get a line (end with \n) from stdin

        line[strlen(line) - 1] = 0; // kill \n at end
        if (line[0] == 0)           // exit if NULL line
            continue;

        char command[64], pathname[64];
        command[0] = pathname[0] = 0;
        sscanf(line, "%s %s", command, pathname);
        if (command[0] == '\0')
            continue;

        int index = find_cmd(command);
        if (index < 0)
        {
            if (!strcmp(command, "get"))
            {
                // Send ENTIRE line to server
                send(sfd, line, MAX, 0);
                printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

                // read answer and printf to file
                int t = 0;
                int fd = open(pathname, O_WRONLY | O_CREAT, 0744);
                read(sfd, ans, MAX);
                read(sfd, ans, MAX);
                int s = atoi(ans);
                printf("s = %d\n", s);
                while (n < s)
                {
                    t = read(sfd, ans, MAX);
                    write(fd, ans, t);
                    n += t;
                }

                printf("server sent back %d bytes\n", n);
                close(fd);
            }
            else if (!strcmp(command, "put"))
            {
                /* code */
            }
            //just echo non-command inputs
            else
            {
                // Send ENTIRE line to server
                send(sfd, line, MAX, 0);
                printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

                // Read a line from sock and show it
                n = read(sfd, ans, MAX);
                puts(ans);
                printf("server sent back %d bytes\n", n);
            }
        }
        else
        {
            int r = fptr[index](pathname);
        }
    }

    return 0;
}

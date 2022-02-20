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
#include <libgen.h>
#include <time.h>

#include <sys/wait.h>

#define MAX 8192
#define PORT 35535

int n;

char ans[MAX];
char line[MAX];

int get(char *fileName, int sockfd);
int put(char *fileName, int sockfd);
int ls(char *dirName);
int cd(char *dirName);
int pwd(char *dirName);
int rm(char *fileName);

char *cmd[] = {"ls", "cd", "pwd", "rm", NULL};
int (*fptr[])(char *) = {(int (*)())ls, cd, pwd, rm};

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

int get(char *fileName, int sockfd)
{
    int n;
    int fd = open(fileName, O_RDONLY);
    if (fd < 0)
        return -1;

    struct stat mystat;
    stat(fileName, &mystat);
    n = read(fd, ans, MAX);
    write(sockfd, ans, n);

    close(fd);

    return 0;
}

int put(char *fileName, int sockfd)
{

    return 0;
}

int ls_file(char *fname)
{
    char t[512];
    struct stat fstat, *sp;
    char *t1 = "xwrxwrxwr-------";
    char *t2 = "----------------";
    int r, i;
    char ftime[64];
    sp = &fstat;

    if ((r = lstat(fname, &fstat)) < 0)
    {
        sprintf(ans, "can't stat %s\n", fname);
        return -1;
    }

    if ((sp->st_mode & 0xF000) == 0x8000)
    {
        sprintf(t, "%c", '-');
        strcat(ans, t);
    }
    if ((sp->st_mode & 0xF000) == 0x4000)
    {
        sprintf(t, "%c", 'd');
        strcat(ans, t);
    }
    if ((sp->st_mode & 0xF00) == 0xA000)
    {
        sprintf(t, "%c", 'l');
        strcat(ans, t);
    }
    for (i = 8; i >= 0; i--)
    {
        if (sp->st_mode & (1 << i))
        {
            sprintf(t, "%c", t1[i]);
            strcat(ans, t);
        }
        else
        {
            sprintf(t, "%c", t2[i]);
            strcat(ans, t);
        }
    }

    sprintf(t, "%4ld ", sp->st_nlink);
    strcat(ans, t);
    sprintf(t, "%4d ", sp->st_gid);
    strcat(ans, t);
    sprintf(t, "%4d ", sp->st_uid);
    strcat(ans, t);
    sprintf(t, "%8ld ", sp->st_size);
    strcat(ans, t);

    strcpy(ftime, ctime(&sp->st_ctime));
    ftime[strlen(ftime) - 1] = 0;
    sprintf(t, "%s  ", ftime);
    strcat(ans, t);

    sprintf(t, "%s", basename(fname));
    strcat(ans, t);
    if ((sp->st_mode & 0xF000) == 0XA000)
    {
        char buf[128];
        readlink(fname, buf, 512);
        sprintf(t, " -> %s", buf);
        strcat(ans, t);
    }
    strcat(ans, "\n");

    return 0;
}

int ls_dir(char *dname)
{
    struct dirent *de;
    DIR *dr = opendir(dname);
    if (!dr)
    {
        sprintf(ans, "Could not open %s\n", dname);
        return -1;
    }

    while ((de = readdir(dr)))
        ls_file(de->d_name);

    closedir(dr);
    return 0;
}

int ls(char *dirName)
{
    struct stat mystat, *sp = &mystat;
    int r;
    char path[1024], cwd[256];
    char *dfn = "./";
    if (dirName[0] != '\0')
        dfn = dirName;
    if (r = lstat(dfn, sp) < 0)
    {
        sprintf(ans, "no such file %s\n", dfn);
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

int cd(char *dirName)
{
    int r = chdir(dirName);
    return r;
}

int pwd(char *dirName)
{
    char cwd[256];
    getcwd(cwd, 256);
    sprintf(ans, "cwd=%s\n", cwd);

    return 0;
}

int rm(char *fileName)
{
    int r = remove(fileName);
    return r;
}

int main()
{
    char cwd[256];
    getcwd(cwd, 256);
    if (chroot(cwd) != 0)
    {
        return -1;
    }

    int sfd, cfd, len;
    struct sockaddr_in saddr, caddr;
    int i, length;

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
    //saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(PORT);

    printf("3. bind socket to server\n");
    if ((bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr))) != 0)
    {
        printf("socket bind failed\n");
        exit(0);
    }

    // Now server is ready to listen and verification
    if ((listen(sfd, 5)) != 0)
    {
        printf("Listen failed\n");
        exit(0);
    }

    while (1)
    {
        // Try to accept a client connection as descriptor newsock
        length = sizeof(caddr);
        cfd = accept(sfd, (struct sockaddr *)&caddr, &length);
        if (cfd < 0)
        {
            printf("server: accept error\n");
            exit(1);
        }

        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        printf("    IP=%s  port=%d\n", "127.0.0.1", ntohs(caddr.sin_port));
        printf("-----------------------------------------------\n");

        // Processing loop
        while (1)
        {
            bzero(ans, MAX);
            printf("server ready for next request ....\n");
            n = read(cfd, line, MAX);
            if (n == 0)
            {
                printf("server: client died, server loops\n");
                close(cfd);
                break;
            }

            char command[1024], pathname[3072];
            command[0] = pathname[0] = 0;
            sscanf(line, "%s %s", command, pathname);

            if (!strcmp(command, "mkdir"))
            {
                int r = mkdir(pathname, 0744);
                if (!r)
                    sprintf(ans, "server: mkdir %s ok\n", pathname);
                else
                    sprintf(ans, "server: mkdir %s failed\n", pathname);

                n = write(cfd, ans, MAX);
                printf("server: wrote n=%d bytes\n", n);
            }
            else if (!strcmp(command, "rmdir"))
            {
                int r = rmdir(pathname);
                if (!r)
                    sprintf(ans, "server: rmdir %s ok\n", pathname);
                else
                    sprintf(ans, "server: rmdir %s failed\n", pathname);

                n = write(cfd, ans, MAX);
                printf("server: wrote n=%d bytes\n", n);
            }
            else if (!strcmp(command, "get"))
            {
                int r = get(pathname, cfd);
            }
            else if (!strcmp(command, "put"))
            {
                int fd = open(pathname, O_WRONLY | O_CREAT, 0755);
                n = read(sfd, ans, MAX);
                write(fd, ans, n);

                close(fd);
            }
            else
            {
                int index = find_cmd(command);

                //if not a command, repeat message back
                if (index < 0)
                {
                    // show the line string
                    printf("server: read  n=%d bytes; line=[%s]\n", n, line);
                    strcat(line, " ECHO");

                    // send the echo line to client
                    n = write(cfd, line, MAX);

                    printf("server: wrote n=%d bytes", n);
                }
                else
                {
                    printf("server: read  n=%d bytes; line=[%s]\n", n, line);
                    printf("server: executing %s\n", cmd[index]);
                    int r = fptr[index](pathname);
                    n = write(cfd, ans, MAX);
                }
            }
        }
    }
}
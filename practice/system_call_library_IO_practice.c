#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

char *t1 = "xwrxwrxwr-------"; 
char *t2 = "----------------"; 

int ls_file(char *fname)
{
    struct stat fstat, *sp;
    int r, i;
    char ftime[64];
    sp = &fstat;

    if (r = lstat(fname, &fstat) < 0)
    {
        printf("can't stat %s\n", fname);
        return 1;
    }

    if ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
        printf("%c",'-'); 
    if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR()) 
        printf("%c", 'd');
    if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK()) 
        printf("%c", 'l');
    
    for (i = 8; i >= 0; i--)
    {
        if (sp->st_mode & (1 << i)) // print r|w|x 
            printf("%c", t1[i]);
        else
            printf("%c", t2[i]); // or print -
    }

    printf("%4d ",sp->st_nlink); // link count 
    printf("%4d ",sp->st_gid); // gid 
    printf("%4d ",sp->st_uid); // uid 
    printf("%8d ",sp->st_size); // file size 
    // print time 
    strcpy(ftime, ctime(&sp->st_ctime)); // print time in calendar form 
    ftime[strlen(ftime)-1] = 0; // kill \n at end 
    printf("%s ",ftime); 
    // print name 
    printf("%s", basename(fname)); // print file basename 
    // print -> linkname if symbolic file 
    if ((sp->st_mode & 0xF000)== 0xA000)
    {
        char buf[512];
        readlink(fname, buf, 512);
        printf(" -> %s", buf);
    }
    printf("\n");
}

int ls_dir(char *dname)
{
    DIR *d;
    struct dirent *dp;
    d = opendir(dname);
    char *fileName;
    while ((dp = readdir(d)) != NULL)
    {
        ls_file(dp->d_name);
    }
    return 0;
}

int my_ls(char *name)
{
    struct stat mystat, *sp = &mystat;
    int r;
    char *filename, path[1024], cwd[256];
    if (strlen(name) != 0)
        filename = name;
    else
        filename = "./";
    printf("filename = %s\n", filename);
    if (r = lstat(filename, sp) < 0)
    {
        printf("no such file %s\n", filename);
        return 1;
    }
    strcpy(path, filename);
    if (path[0] != '/')
    {
        getcwd(cwd, 256);
        strcpy(path, cwd); strcat(path, "/"); strcat(path, filename);
    }
    if (S_ISDIR(sp->st_mode))
    {
        ls_dir(path);
    }
    else
        ls_file(path);
}

int my_cat(char *filename)
{
    int fd;
    int i, n;
    char buf[4096];
    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        printf("cat failed\n");
        return 1;
    }
    while (n = read(fd, buf, 4096))
    {
        write(1, buf, n);
    }
}

int my_cat2(char *filename)
{
    FILE *fp;
    char buf[4096];
    fp = fopen(filename, "r");
    if (fp == 0)
    {
        printf("cat2 failed\n");
        return 1;
    }
    while (fgets(buf, 4096, fp))
        fputs(buf, stdout);
}

int main(int argc, char *argv[])
{
    my_ls("lab3");

    my_cat("stuff.txt");
    my_cat2("stuff.txt");

    return 0;
}
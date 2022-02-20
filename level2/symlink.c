/************* symlink.c file **************/
#include "type.h"

extern int my_creat();
extern int getino();
extern MINODE *iget();
extern void iput(); 

/**** globals defined in main.c file ****/
extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC proc[NPROC], *running;

extern char gpath[128];
extern char *name[64];
extern int n;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;

extern char line[128], cmd[32], pathname[128];

int my_symlink(char *old, char *new)
{
    int oino = getino(old);
    if (!oino)
    {
        printf("error - my_symlink: %s does not exist.\n", old);
        return -1;
    }

    MINODE *omip = iget(dev, oino);
    if (S_ISLNK(omip->INODE.i_mode))
    {
        printf("error - my_symlink: file1 can not be link type.\n");
        return -2;
    }

    char temp[128];
    bzero(temp, 128);
    strncpy(temp, new, strlen(new));
    int r = my_creat(temp);
    if (r)
        return r;
    
    bzero(temp,128);
    strncpy(temp, new, strlen(new));
    MINODE *mip, *pmip;
    int ino = getino(temp);
    mip = iget(dev, ino);

    char *parent = dirname(new);
    int pino = getino(parent);
    pmip = iget(dev, pino);
    pmip->refCount++;
    pmip->INODE.i_atime = time(0L);
    iput(pmip);

    mip->INODE.i_size = strlen(old);
    mip->refCount++;
    mip->INODE.i_mode = 0xA1A4;
    mip->dirty = 1;

    if (strlen(old) > 60)
    {
        printf("error - my_symlink: fil1 name too long.\n");
        return -3;
    }

    memcpy(mip->INODE.i_block, old, strlen(old));
    iput(mip);
    printf("symlink OK.\n");
    return 0;
}

int my_readlink(char *pName)
{
    int ino = getino(pName);
    if (!ino)
    {
        printf("error - my_readlink: %s does not exist.\n", pName);
        return -1;
    }
    MINODE *mip = iget(dev, ino);
    if (!S_ISLNK(mip->INODE.i_mode))
    {
        printf("error - my_readlink: %s is not link type.\n", pName);
        return -2;
    }

    char *n = (char*)mip->INODE.i_block;
    char temp[128]; bzero(temp,128);
    strncpy(temp, n, strlen(n));
    printf("%s -> %s\n", pName, temp);
    return 0;
}
/************* link_unlink.c file **************/
#include "type.h"

extern int getino();
extern MINODE *iget();
extern int enter_name();
extern void iput();
extern int bdalloc();
extern int idalloc();
extern int rm_child();

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


int my_link(char *old, char *new)
{
    printf("old=%s, new=%s\n", old, new);
    int oino = getino(old);
    if (!oino)
    {
        printf("error - my_link: %s does not exits.\n", old);
        return -1;
    }

    MINODE *omip = iget(dev, oino);

    if (S_ISDIR(omip->INODE.i_mode))
    {
        printf("error - my_link: file1 can not be a directory.\n");
        return -2;
    }
    
    char temp[128];
    bzero(temp, 128);
    strncpy(temp, new, strlen(new));

    char *parent = dirname(new);
    char *child = basename(temp);
    printf("parent=%s, child=%s\n", parent, child);

    int pino;
    if ((pino = getino(parent)) == 0)
    {
        printf("error - my_link: file2 has bad path.\n");
        return -3;
    }
    
    MINODE *pmip = iget(dev, pino);

    enter_name(pmip, oino, child);

    omip->INODE.i_links_count++;
    omip->dirty = 1;
    iput(omip);
    iput(pmip);
    printf("link OK.\n");
    return 0;
}

int my_unlink(char *fName)
{
    int ino = getino(fName);
    MINODE *mip = iget(dev, ino);

    if (S_ISDIR(mip->INODE.i_mode))
    {
        printf("error - my_unlink: can not unlink dir type.\n");
        return -1;
    }

    char temp[128];
    bzero(temp, 128);
    strncpy(temp, fName, strlen(fName));
    char *parent = dirname(fName);
    char *child = basename(temp);

    printf("parent=%s, child=%s\n", parent, child);

    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);

    rm_child(pmip, child);

    pmip->dirty = 1;
    iput(pmip);

    mip->INODE.i_links_count--;
    
    if (mip->INODE.i_links_count > 0)
        mip->dirty = 1;
    //if links == 0 actually remove file, i.e. delete its datablock
    else
    {
        for (int i = 0; i < 12; i++)
        {
            if (mip->INODE.i_block[i])
                bdalloc(dev, mip->INODE.i_block[i]);
        }
        
        idalloc(dev, ino); 
    }

    iput(mip);
    printf("unlink OK.\n");
    return 0;
}
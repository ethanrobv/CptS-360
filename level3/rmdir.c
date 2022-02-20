/************* rmdir.c file **************/
#include "type.h"

extern int getino();
extern MINODE *iget();
extern int get_block();
extern int search();
extern char *find_name();
extern int bdalloc();
extern int idalloc();
extern int ideal_len();
extern int put_block();
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

int rm_child(MINODE *pmip, char *n)
{
    char buf[BLKSIZE];
    DIR *dp, *lastDir;
    char *cp, *last;
    int length = 0;

    for (int i = 0; i < 12; i++)
    {
        if (!pmip->INODE.i_block[i])
            return -1;
        
        get_block(pmip->dev, pmip->INODE.i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        while (cp < buf + BLKSIZE)
        {
            char t[128];
            bzero(t, 128);
            strncpy(t, dp->name, dp->name_len);
        
            if (!strcmp(t, n))
            {
                printf("located %s...\n", n);

                //CASE 1: last entry
                if (cp + dp->rec_len == buf + BLKSIZE)
                {
                    int r = dp->rec_len;
                    memset(cp, 0, BLKSIZE - length);
                    lastDir->rec_len += r;
                    put_block(pmip->dev, pmip->INODE.i_block[i], buf);
                    return 0;
                }
                //CASE 2: only entry
                else if (dp->rec_len == BLKSIZE)
                {
                    memset(buf, 0, BLKSIZE);
                    put_block(pmip->dev, pmip->INODE.i_block[i], buf);
                    bdalloc(pmip->dev, pmip->INODE.i_block[i]);
                    pmip->INODE.i_size -= BLKSIZE;

                    //move all subseq. blocks to the left
                    for (int j = i; j < 11; j++)
                        pmip->INODE.i_block[j] = pmip->INODE.i_block[j+1];
                    pmip->INODE.i_block[11] = 0;
                    return 0;
                }
                //CASE 3: middle entry
                else
                {
                    int r = dp->rec_len;
                    int segment = length + dp->rec_len;

                    //overwrite entry
                    memcpy(cp, cp + dp->rec_len, BLKSIZE - segment);

                    while (cp < buf + BLKSIZE - segment)
                    {
                        dp = (DIR*)cp;
                        cp += dp->rec_len;
                    }

                    //increase remaining block size by size of deleted entry
                    dp->rec_len += r;

                    put_block(pmip->dev, pmip->INODE.i_block[i], buf);
                    return 0;
                }
            }

            length += dp->rec_len;
            last = cp;
            lastDir = dp;
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
}

int my_rmdir(char *dName)
{
    int ino = getino(dName);
    MINODE *mip = iget(dev, ino);

    //check if directory
    if (!S_ISDIR(mip->INODE.i_mode))
    {
        printf("error - my_rmdir: %s is not a directory.\n", dName);
        return -1;
    }

    char buf[BLKSIZE];
    DIR *dp; char *cp;
    for (int i = 0; i < 12; i++)
    {
        if (!mip->INODE.i_block[i])
            break;
        
        get_block(mip->dev, mip->INODE.i_block[i], buf);
        dp = (DIR*)buf;
        cp = buf;

        //if the last dir entry is "..", if so, the dir is empty
        while(cp + dp->rec_len < buf + BLKSIZE)
        {
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    //dp points at last entry
    if (strcmp(dp->name, ".."))
    {
        printf("error - my_rmdir: %s is not empty.\n", dName);
        return -2;
    }

    char temp[128];
    bzero(temp, 128);
    strcpy(temp, dName);
    temp[strlen(temp)] = '\0';

    char *parent = dirname(dName);
    char *child = basename(temp);

    printf("parent=%s, child = %s\n", parent, child);

    int pino = getino(parent);
    MINODE *pmip = iget(mip->dev, pino);

    //deallocate all datablocks in iblock
    for (int i = 0; i < 12; i++)
    {
        if (mip->INODE.i_block[i])
            bdalloc(mip->dev, mip->INODE.i_block[i]);
    }

    idalloc(mip->dev, mip->ino);
    iput(mip);

    rm_child(pmip, child);
    pmip->refCount--;
    pmip->INODE.i_atime = pmip->INODE.i_mtime = time(0L);
    pmip->dirty = 1;
    iput(pmip);

    printf("rmdir OK.\n");
    return 0;
}
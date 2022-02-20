/************* mkdir_creat.c file **************/
#include "type.h"

extern int getino();
extern MINODE *iget();
extern int get_block();
extern int put_block();
extern void iput();
extern int search();
extern int ialloc();
extern int balloc();
extern int ideal_len();

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

int enter_name(MINODE *pmip, int ino, char *n)
{
    char buf[BLKSIZE];
    for (int i = 0; i < 12; i++)
    {
        if (!pmip->INODE.i_block[i])
            break;
        get_block(pmip->dev, pmip->INODE.i_block[i], buf);
        DIR *dp = (DIR *)buf;
        char *cp = buf;

        //go to last entry in data block
        while (cp + dp->rec_len < buf + BLKSIZE)
        {
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        DIR *lastDir = dp;
        int remain = lastDir->rec_len - ideal_len(lastDir->name);

        //trim last dir entry and add new one to end
        if (remain >= ideal_len(n))
        {
            int t = ideal_len(lastDir->name);
            lastDir->rec_len = t;
            cp += lastDir->rec_len;
            lastDir = (DIR *)cp;
            strncpy(lastDir->name, n, strlen(n));
            lastDir->name_len = strlen(n);
            lastDir->inode = ino;
            lastDir->rec_len = remain;
            put_block(pmip->dev, pmip->INODE.i_block[i], buf);
        }
        //no space in data block
        else
        {
            printf("must allocate new data block...\n");
            bzero(buf, BLKSIZE);
            int bno = balloc(dev);
            get_block(pmip->dev, bno, buf);
            dp = (DIR *)buf;
            strncpy(dp->name, n, strlen(n));
            dp->name_len = strlen(n);
            dp->inode = ino;
            dp->rec_len = BLKSIZE;
            put_block(pmip->dev, bno, buf);
        }
    }
    return 0;
}

//make new dir "n" in pmip with 2 links . and ..
int kmkdir(MINODE *pmip, char *n)
{
    int ino = ialloc(pmip->dev);
    int bno = balloc(pmip->dev);

    char buf[BLKSIZE];

    MINODE *mip = iget(pmip->dev, ino);
    INODE *ip = &mip->INODE;
    DIR *dp;
    char *cp;

    ip->i_mode = 0x41ED;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = BLKSIZE;
    ip->i_links_count = 2; //. and ..
    ip->i_mtime = ip->i_ctime = ip->i_atime = time(0L);
    ip->i_blocks = 2;            //512 byte chunks; 1024 byte blocksize
    ip->i_block[0] = bno;        //new datablock from balloc()
    for (int i = 1; i < 15; i++) //zero out blocks 1-14
        ip->i_block[i] = 0;

    mip->dirty = 1; //we changed data
    iput(mip);

    get_block(mip->dev, ip->i_block[0], buf);

    dp = (DIR *)buf;
    cp = buf;

    //create .
    strcpy(dp->name, ".");
    dp->inode = mip->ino;
    dp->name_len = 1;
    dp->rec_len = 12;

    cp += dp->rec_len;
    dp = (DIR *)cp;

    //create ..
    strcpy(dp->name, "..");
    dp->inode = pmip->ino;
    dp->name_len = 2;
    dp->rec_len = BLKSIZE - 12;

    put_block(dev, ip->i_block[0], buf);

    enter_name(pmip, ino, n);
}

int my_mkdir(char *dName)
{
    MINODE *mip;

    if (dName[0] == '/')
    {
        mip = root;
        dev = root->dev;
    }
    else
    {
        mip = running->cwd;
        dev = mip->dev;
    }

    char temp[128];
    bzero(temp, 128);
    strcpy(temp, dName);
    temp[strlen(temp)] = '\0';

    char *parent = dirname(dName);
    char *child = basename(temp);

    int pino = getino(parent);

    if (pino == -1)
    {
        return -1;
    }

    MINODE *pmip = iget(dev, pino);

    if (S_ISDIR(pmip->INODE.i_mode)) // INODE is a directory
    {
        if (search(pmip, child) == 0)
        {
            kmkdir(pmip, child);

            pmip->refCount++;
            pmip->INODE.i_atime = pmip->INODE.i_ctime = pmip->INODE.i_mtime = time(0L);

            iput(pmip); //update disk
            printf("mkdir OK.\n");
            return 0;
        }
        else
        {
            printf("error - my_mkdir: directory already exists.\n");
            return -1;
        }
    }
    else
    {
        printf("error - my_mkdir: %s is not a directory.\n", parent);
        return -1;
    }
}

int my_creat(char *fName)
{
    if (fName[0] == '\0')
    {
        printf("error - my_creat: must specify a file name\n");
        return -1;
    }
    char temp[128];
    bzero(temp, 128);
    strcpy(temp, fName);
    temp[strlen(temp)] = '\0';

    char *parent = dirname(fName);
    char *child = basename(temp);

    int pino = getino(parent);
    MINODE *pmip = iget(dev, pino);

    if (!S_ISDIR(pmip->INODE.i_mode))
    {
        printf("error - my_creat: must specify a directory.\n");
        return -1;
    }
    
    if (search(pmip, child))
    {
        printf("error - my_creat: file already exists.\n");
        return -2;
    }

    int ino = ialloc(dev);
    char buf[BLKSIZE];
    MINODE *mip = iget(pmip->dev, ino);
    INODE *ip = &mip->INODE;
    DIR *dp;
    char *cp;

    ip->i_mode = 0x81A4;
    ip->i_uid = running->uid;
    ip->i_gid = running->gid;
    ip->i_size = 0;
    ip->i_links_count = 1;
    ip->i_mtime = ip->i_ctime = ip->i_atime = time(0L);
    ip->i_blocks = 2;
    ip->i_block[0] = 0;
    for (int i = 1; i < 15; i++)
        ip->i_block[i] = 0;

    mip->dirty = 1;
    iput(mip);
    enter_name(pmip, ino, child);
    pmip->INODE.i_atime = pmip->INODE.i_ctime = pmip->INODE.i_mtime = time(0L);
    pmip->dirty = 1;
    iput(pmip);
    printf("creat OK.\n");
    return ino;
}
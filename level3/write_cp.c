/************* write_cp.c file **************/
#include "type.h"

extern int get_block();
extern int balloc();
extern int my_truncate();
extern int get_block();
extern int put_block();
extern int my_open();
extern int my_read();
extern int my_write();
extern int my_close();
extern int my_creat();


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

int write_file()
{
    char ans[128];
    int fd; char fname[128];
    do
    {
        printf("Enter a [file descriptor] and [text string]: ");
        fgets(ans, 128, stdin);
    } while (ans[0] == '\0');

    sscanf(ans, "%d %s", &fd, fname);


}

int my_write(int fd, char *buf, int nbytes)
{
    int count = 0;
    int blk, lblk, startByte;
    if (nbytes < 0)
    {
        printf("error - my_write: cannot write negative bytes.\n");
        return -1;
    }

    while (nbytes)
    {
        lblk = running->fd[fd]->offset / BLKSIZE;
        startByte = running->fd[fd]->offset % BLKSIZE;

        //if direct block
        if (lblk < 12)
        {
            //physical block may not exist, must be allocated and recorded
            //in the INODE
            if (!running->fd[fd]->minodePtr->INODE.i_block[lblk])
                running->fd[fd]->minodePtr->INODE.i_block[lblk] = balloc(running->fd[fd]->minodePtr->dev);

            blk = running->fd[fd]->minodePtr->INODE.i_block[lblk];
        }
        
        //if indirect block (points to 256 direct bnos, each bno 4 bytes)
        else if (lblk >= 12 && lblk < 256 + 12)
        {   
            //indirect block [12] may not exist, if not, must be allocated
            //and recorded in the INODE, it should be initialized to 0
            if (!running->fd[fd]->minodePtr->INODE.i_block[12])
            {   
                running->fd[fd]->minodePtr->INODE.i_block[12] = balloc(running->fd[fd]->minodePtr->dev);
                int tbuf[256];
                bzero(tbuf, sizeof(tbuf));
            }

            //now convert logical block to physical block, allocate new block if 
            //physical block doesn't exist
            int tbuf[256];
            get_block(running->fd[fd]->minodePtr->dev, running->fd[fd]->minodePtr->INODE.i_block[12], tbuf);
            blk = tbuf[lblk - 12];
            if (!blk)
            {
                tbuf[lblk - 12] = balloc(running->fd[fd]->minodePtr->dev);
                blk = tbuf[lblk - 12];
                put_block(running->fd[fd]->minodePtr->dev, running->fd[fd]->minodePtr->INODE.i_block[12], tbuf);
            }
        }

        //if double indirect block (points to 256 indirect bnos, etc.)
        else
        {
            //double indirect block [13] may not exist, if not, must be allocated
            //and recorded in the INODE, should be initialized to 0
            //if an entry in the double indirect block doesn't exist, it should too be
            //intialized to 0.
            if (!running->fd[fd]->minodePtr->INODE.i_block[13])
            {
                int buf13[256];
                //allocate block
                running->fd[fd]->minodePtr->INODE.i_block[13] = balloc(running->fd[fd]->minodePtr->dev);

                //initialize block to zero
                bzero(buf13, sizeof(buf13));
                put_block(running->fd[fd]->minodePtr->dev, running->fd[fd]->minodePtr->INODE.i_block[13], (char*)buf13);
            }

            //use mailman alg to get target indirect block and direct block
            lblk -= (256 + 12);
            int buf13[256];
            bzero(buf13, sizeof(buf13));
            int dbuf[256];
            bzero(dbuf, sizeof(dbuf));
            //get double indirect block into buf
            get_block(running->fd[fd]->minodePtr->dev, running->fd[fd]->minodePtr->INODE.i_block[13], buf13);
            int dblk = buf13[lblk / 256];
            //if dblk does not exist, must be allocated
            //and recorded in the INODE, should be initialized to 0
            if (!dblk)
            {
                buf13[lblk / 256] = balloc(running->fd[fd]->minodePtr->dev);
                dblk = buf13[lblk / 256];
                int kbuf[256];
                bzero(kbuf, sizeof(kbuf));
                put_block(running->fd[fd]->minodePtr->dev, dblk, (char*)kbuf);
                put_block(running->fd[fd]->minodePtr->dev, running->fd[fd]->minodePtr->INODE.i_block[13], (char*)buf13);
            }

            //if physical block doesn't exist, must be allocated
            get_block(running->fd[fd]->minodePtr->dev, dblk, dbuf);
            blk = dbuf[lblk % 256];
            if (!blk)
            {
                dbuf[lblk % 256] = balloc(running->fd[fd]->minodePtr->dev);
                blk = dbuf[lblk % 256];
                int kbuf[256];
                bzero(kbuf, sizeof(kbuf));
                put_block(running->fd[fd]->minodePtr->dev, blk, (char*)kbuf);
                put_block(running->fd[fd]->minodePtr->dev, dblk, (char*)dbuf);
                //put_block(running->fd[fd]->minodePtr->dev, running->fd[fd]->minodePtr->INODE.i_block[13], )
            }
        }

        //write data to the block
        char writebuf[BLKSIZE];
        bzero(writebuf, sizeof(writebuf));
        char *cp = writebuf + startByte;
        int remain = BLKSIZE - startByte;
        char *cq = buf;

        get_block(running->fd[fd]->minodePtr->dev, blk, writebuf);

        //write 1 byte at a time
        /*
        while (remain > 0)
        {
            *cp++ = *cq++;
            count++;
            nbytes--; remain--;
            running->fd[fd]->offset++;
            if (running->fd[fd]->offset > running->fd[fd]->minodePtr->INODE.i_size)
                running->fd[fd]->minodePtr->INODE.i_size++;
            
            if (nbytes <= 0)
                break;
        }
        */
        
        //optimize to write maximal number of bytes -->

        //number of bytes to write is LE remaining bytes
        //in logical block; write everything in one go
        if(nbytes <= remain)
        {
            memcpy(cp, cq, nbytes);
            count += nbytes;
            remain -= nbytes;
            running->fd[fd]->offset += nbytes;
            if (running->fd[fd]->offset > running->fd[fd]->minodePtr->INODE.i_size)
                running->fd[fd]->minodePtr->INODE.i_size += nbytes;
            nbytes -= nbytes;
        }

        //number of bytes to write will not fit in the logical block,
        //write to end of block then find next one
        else
        {
            memcpy(cp, cq, remain);
            count += remain;
            nbytes -= remain;
            running->fd[fd]->offset += remain;
            if (running->fd[fd]->offset > running->fd[fd]->minodePtr->INODE.i_size)
                running->fd[fd]->minodePtr->INODE.i_size += remain;
            remain -= remain;
        }

        put_block(running->fd[fd]->minodePtr->dev, blk, writebuf);
    }

    running->fd[fd]->minodePtr->dirty = 1;
    printf("write OK: wrote %d char into file descriptor fd=%d\n", count, fd);
    return count;
}

int my_cp(char *src, char *dest)
{
    if (!strcmp(src, dest))
    {
        printf("error - my_cp: can not copy a file to itself.\n");
        return -1;
    }
    int fd = my_open(src, 0);
    if (fd < 0)
        return -1;
    int gd = my_open(dest, 1);
    if (gd == -1)
    {
        char t[512] = {0};
        strncpy(t, dest, strlen(dest));
        my_creat(dest);
        gd = my_open(t, 1);
    }
    int n; char buf[BLKSIZE];

    while (n = my_read(fd, buf, BLKSIZE))
    {
        my_write(gd, buf, n);
        bzero(buf, sizeof(buf));
    }

    my_close(fd);
    my_close(gd);

    printf("cp OK.\n");
    return 0;
}
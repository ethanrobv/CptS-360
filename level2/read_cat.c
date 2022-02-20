/************* read_cat.c file **************/
#include "type.h"

extern int get_block();
extern int my_open();
extern int my_read();
extern int my_close();

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

int read_file()
{
    //ask for FD and nbytes
    int fdToRead; int bytesToRead; char ans[64];
    do
    {
    printf("Enter a [file descriptor] and [num bytes to read]:");
    fgets(ans, 64, stdin);
    } while (ans[0] != '\0');

    sscanf(ans, "%d %d", &fdToRead, &bytesToRead);
    

    char buf[BLKSIZE];

    //verify that FD is open for RD or RW
    if (!running->fd[fdToRead])
    {
        printf("error - read_file: fd %d is not open.\n", fdToRead);
        return -1;
    }

    if (running->fd[fdToRead]->mode == 0 || running->fd[fdToRead]->mode == 2)
        return my_read(fdToRead, buf, bytesToRead);
    
    printf("error - read_file: fd %d is not opened in RD or RW mode.\n", fdToRead);
    return -2;
}

int my_read(int fd, char *buf, int nbytes)
{
    int count = 0;
    int offset = running->fd[fd]->offset;
    int avail = running->fd[fd]->minodePtr->INODE.i_size - offset;
    char *cq = buf;
    int lblk, blk, startByte;

    //we call read with the intention of reading 1024 bytes,
    //however if the contents we must read isn't actually that 
    //large we instead read only the byte equiv of file size.
    if (nbytes > avail)
        nbytes = avail;

    while (nbytes)
    {
        lblk = running->fd[fd]->offset / BLKSIZE;
        startByte = running->fd[fd]->offset % BLKSIZE;

        //if direct block
        if (lblk < 12)
            blk = running->fd[fd]->minodePtr->INODE.i_block[lblk];
        
        //if indirect block (points to 256 direct bnos, each bno 4 bytes)
        else if (lblk >= 12 && lblk < 256 + 12)
        {
            int tbuf[256];

            //get all 256 bnos into tbuf, navigate to target bno
            get_block(dev, running->fd[fd]->minodePtr->INODE.i_block[12], tbuf);
            blk = tbuf[lblk - 12];
        }
        //if double indirect block (points to 256 indirect bnos, etc.)
        else
        {
            //use mailman alg to get target indirect block and direct block
            lblk -= (256 + 12);
            int tbuf[256];
            //get double indirect block into buf
            get_block(dev, running->fd[fd]->minodePtr->INODE.i_block[13], tbuf);
            int dblk = tbuf[lblk / 256];
            //get dblk into buf
            int ttbuf[256];
            get_block(dev, dblk, ttbuf);
            blk = ttbuf[lblk % 256];
        }

        //get target direct block into readbuf
        char readbuf[BLKSIZE];
        bzero(readbuf, sizeof(readbuf));
        get_block(running->fd[fd]->minodePtr->dev, blk, readbuf);

        //navigate to correct position via offset
        char *cp = readbuf + startByte;
        //maximum bytes left in buf
        int remain = BLKSIZE - startByte;

        //read one byte at a time
        /*
        while (remain > 0)
        {
            //copy one byte from cp to cq
            *cq++ = *cp++;
            running->fd[fd]->offset++;
            count++;
            avail--; 
            remain--;
            nbytes--; 
            if (nbytes <= 0 || avail <= 0)
                break;
        }
        */

        //optimize to read maximal number of bytes -->

        //number of bytes to read is LE number of bytes remaining
        //in logical block, just copy nbytes from readbuf to buf
        //loop will terminate
        if (nbytes <= remain)
        {
            //strncpy() won't handle 0-byte(?); slower than memcpy()
            //strncpy(cq, cp, nbytes);
            //copy n bytes from cp to cq
            memcpy(cq, cp, nbytes);
            cq += nbytes; cp += nbytes;
            running->fd[fd]->offset += nbytes;
            count += nbytes;
            avail -= nbytes; 
            remain -= nbytes; 
            nbytes -= nbytes;
        }
        
        //number of bytes to read exceeds number of bytes remaining
        //in logical block, just copy everything remaining in block
        //then will get new logical block and etc. 
        else
        {
            //copy remaining bytes from readbuf to buf
            memcpy(cq, cp, remain);
            cq += remain; cp += remain;
            running->fd[fd]->offset += remain;
            count += remain;
            avail -= remain;
            nbytes -= remain;
            remain -= remain;
        }
    }

    //printf("my_read OK: read %d char from file descriptor %d.\n", count, fd);
    return count;
}

int my_cat(char *fName)
{
    if (fName[0] == '\0')
    {
        printf("error - my_cat: enter a file name.\n");
        return -1;
    }
    char mybuf[BLKSIZE], dummy = 0;
    bzero(mybuf, sizeof(mybuf));
    int fd = my_open(fName, 0);
    if (fd < 0)
        return -1;
    int n = 0;
    //while (n < running->fd[fd]->minodePtr->INODE.i_size)
    while (n = my_read(fd, mybuf, BLKSIZE))
    {
        mybuf[n] = 0;
        printf("%s", mybuf);
    
        bzero(mybuf, sizeof(mybuf));
    }

    putchar('\n');
    my_close(fd);
    return 0;
}
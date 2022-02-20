/************* mount_umount.c file **************/
#include "type.h"

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

MOUNT mountTable[8];

int my_mount()
{
    char ans[128];
    char filesys[64];
    char mount_point[64];
    bzero(mount_point, sizeof(mount_point));
    do
    {
        fgets(ans, 128, stdin);
        ans[strlen(ans)] = '\0';
        sscanf(ans, "%s %s", filesys, mount_point);

    } while (mount_point[0] == '\0' || filesys[0] == '\0');

    for (int i = 0; i < 8; i++)
    {
        if (!strcmp(filesys, mountTable[i].name))
        {
            printf("error - my_mount: filesys is already mounted.\n");
            return -1;
        }
    }

    int freeMountIndex = -1;

    for (int i = 0; i < 8; i++)
    {
        if (!mountTable[i].dev)
        {
            freeMountIndex = i;
            break;
        }
    }

    if (freeMountIndex < 0)
    {
        printf("error - my_mount: no more free mtable entries.\n");
        return -2;
    }

    //initial checks passed, now open new vdisk UNDER LINUX for RW
    //use linux file descriptor as new dev
    int linFD = open(filesys, O_RDWR);
    if (!linFD)
    {
        printf("error - my_mount: could not open %s for RDRW.\n", filesys);
        return -2;
    }

    //verify vdisk is ext2fs
    char buf[BLKSIZE];
    get_block(linFD, 1, buf);
    sp = (SUPER*)buf;
    if (sp->s_magic != 0xEF53)
    {
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        return -3;
    }

    int mtIno = getino(mount_point);
    MINODE *mtMip = iget()




    
}
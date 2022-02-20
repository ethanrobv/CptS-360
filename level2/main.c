/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include "type.h"

extern MINODE *iget();
extern void iput();
extern int get_block();
extern int rpwd();

extern int my_cd();
extern int my_ls();
extern int my_pwd();
extern int my_mkdir();
extern int my_creat();
extern int my_rmdir();
extern int my_link();
extern int my_unlink();
extern int my_symlink();
extern int my_readlink();
extern int my_cat();
extern int my_cp();

MINODE minode[NMINODE];
MINODE *root;
PROC proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int n;           // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[128], pathname2[128];

int init()
{
  int i, j;
  MINODE *mip;
  PROC *p;

  printf("init()\n");

  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i = 0; i < NPROC; i++)
  {
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    //set all file descriptors to NULL
    for (j = 0; j < NFD; j++)
      proc[i].fd[j] = 0;
    p->cwd = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{
  printf("mount_root()\n");
  root = iget(dev, 2);
}

int quit()
{
  int i;
  MINODE *mip;
  for (i = 0; i < NMINODE; i++)
  {
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

char *disk = "diskimage";

int main(int argc, char *argv[])
{
  //clear terminal window with regex
	printf("\e[1;1H\e[2J");
  
  int ino;
  char buf[BLKSIZE];

  if (argc > 1)
  {
    disk = argv[1];
  }

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0)
  {
    printf("open %s failed\n", disk);
    exit(1);
  }

  dev = fd; // global dev same as this fd

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53)
  {
    printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
    exit(1);
  }
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  init();
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  // WRTIE code here to create P1 as a USER process
  printf("creating P1 as user process\n");
  running = &proc[1];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);
  printf("supported command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink|quit]\n");
  while (1)
  {
    bzero(pathname, 128); bzero(pathname2, 128);
    bzero(line, 128);
    bzero(cmd, 32);
    //fun colored terminal output
    printf("\033[1;32m");
    printf("root@erv-CptS360-DEMO:");
    printf("\033[0;34m");
    rpwd(running->cwd);
    //reset terminal output color
		printf("\033[0m");
    printf("$ ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;

    if (line[0] == 0)
      continue;
    pathname[0] = 0;

    sscanf(line, "%s %s %s", cmd, pathname, pathname2);
    printf("cmd=%s pathname=%s pathname2=%s\n", cmd, pathname, pathname2);

    if (strcmp(cmd, "ls") == 0)
      my_ls(pathname);
    else if (strcmp(cmd, "cd") == 0)
      my_cd(pathname);
    else if (strcmp(cmd, "pwd") == 0)
      my_pwd(running->cwd);
    else if (strcmp(cmd, "mkdir") == 0)
      my_mkdir(pathname);
    else if (strcmp(cmd, "creat")==0)
      my_creat(pathname);
    else if (strcmp(cmd, "rmdir")==0)
      my_rmdir(pathname);
    else if (strcmp(cmd, "link")==0)
      my_link(pathname, pathname2);
    else if (strcmp(cmd, "unlink")==0)
      my_unlink(pathname);
    else if (strcmp(cmd, "symlink")==0)
      my_symlink(pathname, pathname2);
    else if (strcmp(cmd, "readlink")==0)
      my_readlink(pathname);
    else if (strcmp(cmd, "cat")==0)
      my_cat(pathname);
    else if (strcmp(cmd, "cp")==0)
      my_cp(pathname, pathname2);
    else if (strcmp(cmd, "quit") == 0)
          quit();

    bzero(gpath, 128);
    bzero(name, 64);
  }
}

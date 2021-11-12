/************* cd_ls_pwd.c file **************/

#include "type.h"

extern int get_block();
extern int getino();
extern MINODE *iget();
extern int search();
extern void iput();
extern PROC *running;
extern MINODE *root;

extern int my_readlink();

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

int my_cd(char dName[128])
{
  //printf("cd: under construction READ textbook!!!!\n");

  // READ Chapter 11.7.3 HOW TO chdir

  int ino = getino(dName);
  MINODE *mip = iget(dev, ino);
  if (mip == 0)
    return -1;
  if (!S_ISDIR(mip->INODE.i_mode))
    return -2;
  iput(running->cwd);
  running->cwd = mip;

  bzero(dName, 128);
  return 0;
}

int ls_file(MINODE *mip, char *fName)
{
  // printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  if (!mip)
    return 1;
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";

  if ((mip->INODE.i_mode & 0xF000) == 0x8000)
    printf("%c", '-');
  else if ((mip->INODE.i_mode & 0xF000) == 0x4000)
    printf("%c", 'd');
  else if ((mip->INODE.i_mode & 0xF000) == 0xA000)
    printf("%c", 'l');

  for (int i = 8; i >= 0; i--)
  {
    if (mip->INODE.i_mode & (1 << i))
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]);
  }

  printf("%4d ", mip->INODE.i_links_count);
  printf("%4d ", mip->INODE.i_gid);
  printf("%4d ", mip->INODE.i_uid);
  printf("%8d ", mip->INODE.i_size);

  //time stuff
  char *time = ctime((time_t*)&mip->INODE.i_mtime);
  time[strlen(time)-1] = '\0';
  printf("%s ", time);
  
  char *t = basename(fName);

  if (S_ISLNK(mip->INODE.i_mode))
    printf("%s -> %s\n", t, (char*)mip->INODE.i_block);
  else
    printf("%s\n", t);

  return 0;
}

int ls_dir(MINODE *mip)
{
  //printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  for (int i = 0; i < 12; i++)
  {
    if (mip->INODE.i_block[i] == 0)
      break;

    get_block(dev, mip->INODE.i_block[i], buf);
    dp = (DIR *)buf;
    cp = buf;

    while (cp < buf + BLKSIZE)
    {
      bzero(temp, 256);
      strncpy(temp, dp->name, dp->name_len);
      MINODE *mip = iget(dev, dp->inode);
      ls_file(mip, temp);
      iput(mip);
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }

  return 0;
}

int my_ls(char *dName)
{
  //printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
  //ls_dir(running->cwd);
  char path[256];
  char *t = "./";
  if (dName[0] != '\0')
    t = dName;
  printf("dir=%s\n", t);

  int ino = getino(t);
  if (ino == 0)
    return 1;
  MINODE *mip = iget(dev, ino);

  if (!t)
  {
    ls_dir(running->cwd);
    iput(mip);
    return 0;
  }

  strcat(path, t);
  if (S_ISDIR(mip->INODE.i_mode))
    ls_dir(mip);
  else
    ls_file(mip, path);
  
  iput(mip);

  printf("ls OK.\n");
  return 0;
}

int rpwd(MINODE *wd)
{
  char buf[BLKSIZE], dName[256];
  bzero(buf, BLKSIZE);
  int ino, pino;
  DIR *dp;
  char *cp;
  MINODE *pip;

  //base case
  if (wd == root)
    return 0;

  ino = search(wd, ".");
  pino = search(wd, "..");

  pip = iget(dev, pino);
  get_block(dev, pip->INODE.i_block[0], buf);

  dp = (DIR *)buf;
  cp = buf;

  while (cp < buf + BLKSIZE)
  {
    bzero(dName, 256);
    strncpy(dName, dp->name, dp->name_len);

    if (dp->inode == ino)
      break;

    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  rpwd(pip);
  iput(pip);

  printf("/%s", dName);

  return 0;
}

int my_pwd(MINODE *wd)
{
  //printf("pwd: READ HOW TO pwd in textbook!!!!\n");
  if (wd == root)
    printf("/");
  else
    rpwd(wd);
  printf("\n");
  return 0;
}

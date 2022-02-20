/************* cd_ls_pwd.c file **************/
#include <unistd.h>

extern int get_block();
extern int getino();
extern MINODE *iget();
extern int search();

int cd(char *dName)
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
  char ftime[64];

  if ((mip->INODE.i_mode & 0xF000) == 0x8000)
    printf("%c", '-');
  else if ((mip->INODE.i_mode & 0xF000) == 0x4000)
    printf("%c",'d');
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

  //time stuf
  time_t ts = mip->INODE.i_ctime;
  strncpy(ftime, asctime(gmtime(&ts)), 64);
  ftime[strlen(ftime) - 1] = 0;
  printf("%s ", ftime);

  printf("%s", basename(fName));
  
  printf("\n");
  
  return 0;
}

int ls_dir(MINODE *mip)
{
  //printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;


  for (int i = 0; i < 12; i++)
  {
    if (mip->INODE.i_block[i] == 0)
      break;
    
    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR*)buf;
    cp = buf;

    while (cp < buf + BLKSIZE)
    {
      bzero(temp, 256);
      strncpy(temp, dp->name, dp->name_len);
      ls_file(iget(dev, dp->inode), temp);
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }

  /*
  while (cp < buf + BLKSIZE)
  {
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;

    printf("%s  ", temp);

    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  */

  printf("\n");

  return 0;
}

int ls(char *dName)
{
  //printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
  //ls_dir(running->cwd);
  char path[1024];
  bzero(path, 1024);
  char *t = "./";
  if (dName[0] != '\0')
    t = dName;

  int ino = getino(t);
  if (ino == 0)
    return 1;
  MINODE *mip = iget(dev, ino);

  if (!t)
  {
    printf("Here\n");
    ls_dir(running->cwd);
    return 0;
  }

  strcat(path, t);

  if (S_ISDIR(mip->INODE.i_mode))
    ls_dir(mip);
  else
    ls_file(mip, path);

  return 0;
}

int rpwd(MINODE *wd)
{
  char buf[BLKSIZE], dName[BLKSIZE];
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

  dp = (DIR*)buf;
  cp = buf;

  while (cp < buf + BLKSIZE)
  {
    bzero(dName, sizeof(dName));
    strncpy(dName, dp->name, dp->name_len);

    if (dp->inode == ino)
      break;
    
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  
  rpwd(pip);
  iput(pip);

  printf("/%s", dName);

  return 0;
}

int pwd(MINODE *wd)
{
  printf("pwd: READ HOW TO pwd in textbook!!!!\n");
  if (wd == root)
    printf("/");
  else
    rpwd(wd);
  printf("\n");
  return 0;
}

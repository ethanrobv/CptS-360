/*************** type.h file for LEVEL-2 ****************/
#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc GD;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD *gp;
INODE *ip;
DIR *dp;

#define FREE 0
#define READY 1

#define BLKSIZE 1024
#define NMINODE 128
#define NPROC 2
#define NFD 10
#define NOFT 64

typedef struct minode
{
  INODE INODE;          // INODE structure on disk
  int dev, ino;                     // (dev, ino) of INODE
  int refCount;                     // in use count
  int dirty;                        // 0 for clean, 1 for modified

  int mounted;              // for level-3
  struct mntable *mptr;     // for level-3
} MINODE;

typedef struct oft
{
  int mode;
  int refCount;
  MINODE *minodePtr;
  int offset;
} OFT;
OFT oft[NOFT];

typedef struct proc
{
  struct proc *next;
  int pid;              // process ID
  int uid;              // user ID
  int gid;
  int status;           // READY or FREE
  MINODE *cwd;          // CWD directory pointer

  OFT *fd[NFD];
} PROC;

typedef struct Mount
{
  int dev;
  int ninodes;
  int nblocks;
  int bmap;
  int imap;
  int iblk;
  struct Minode *mounted_inode;
  char name[64];
  char mount_name[64];
} MOUNT;


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

#endif
/***** LAB3 base code *****/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

char gpath[128]; // hold token strings
char *arg[64];   // token string pointers
int n;           // number of token strings

char dpath[128]; // hold dir strings in PATH
char *dir[64];   // dir string pointers
int ndir;        // number of dirs

char *head[64], *tail[64];
int pd[2];


int exec(char line[128], char **env, char **args);

int tokenize(char *pathname) // YOU have done this in LAB2
{                            // YOU better know how to apply it from now on
  char *s;
  strcpy(gpath, pathname); // copy into global gpath[]
  s = strtok(gpath, " ");
  n = 0;
  while (s)
  {
    arg[n++] = s; // token string pointers
    s = strtok(0, " ");
  }
  arg[n] = 0; // arg[n] = NULL pointer
}

int read_path(char **env)
{
  ndir = 0;
  for (int i = 0; env[i] != NULL; i++)
  {
    //parse *env[] for PATH
    char *temp1 = strtok(env[i], "=");
    if (!strcmp(temp1, "PATH"))
    {
      char *temp2 = strtok(NULL, ":");
      for (int j = 0; temp2 != NULL; j++)
      {
        dir[j] = temp2;
        ndir++;
        temp2 = strtok(NULL, ":");
      }
    }
  }
  //add cwd to dir list
  char *t = "./";
  dir[ndir] = t;
  ndir++;

  return 0;
}

int check_IO_redirect(char line[128], char **env)
{
  for (int i = 0; arg[i] != NULL; i++)
  {
    if (!strcmp(arg[i], ">"))
    {
      if (arg[i + 1] == NULL)
      {
        perror("expected a target");
        break;
      }
      else
      {
        arg[i] = NULL;
        close(1);
        open(arg[i + 1], O_WRONLY | O_CREAT, 0644);
      }
    }
    else if (!strcmp(arg[i], ">>"))
    {
      if (arg[i + 1] == NULL)
      {
        perror("expected a target");
        break;
      }
      else
      {
        arg[i] = NULL;
        close(1);
        open(arg[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0644);
      }
      char *head = arg[i - 1];
      char *tail = arg[i + 1];
    }
  }

  return 0;
}

int clean_pipes(char line[128], char **env)
{
  int i;
  int flag = 0;

  for (i = 0; arg[i] != NULL && !flag; i++)
  {
    if (!strcmp(arg[i], "|"))
      flag = 1;
  }
  //no pipe
  if (!flag)
    return -1;
  
  //encountered a pipe
  printf("cleaning pipe...\n");
  int h = 0, t = 0;
  for(int j = 0; j < n; j++)
  {
    if (j < i-1)
    {
      head[h] = arg[j];
      h++;
    }
    else if (j > i-1)
    {
      tail[t] = arg[j];
      t++;
    }
  }
  head[h] = 0;
  tail[t] = 0;

  pipe(pd);
  int pid = fork();
  /*PARENT AS READER CHILD AS WRITER*/
  if (pid)
  {
    close(pd[1]);
    //THE COMMENTED IMPLEMENTATION IS UNDEFINED
    //close(0);
    //dup (pd[0]);
    dup2(pd[0], 0);
    close(pd[0]);
    exec(line, env, tail);
  }
  else
  {
    close(pd[0]);
    //THE COMMENTED IMPLEMENTATION IS UNDEFINED
    //close(1);
    //dup(pd[1]);
    dup2(pd[1], 1);
    close(pd[1]);
    exec(line, env, head);
  }
  /*PARENT AS WRITER CHILD AS READER
  THIS DOES NOT WORK BECAUSE THE CHILD CAN DIE BEFORE 
  COMMAND HAS FINISHED OUTPUTTING TO TERMINAL
  LEADING TO UNDEFINED BEHAVIOR/CRASHES
  if(pid)
  {
    close(pd[0]);
    close(1);
    dup(pd[1]);
    close(pd[1]);
    exec(line, env, head);
  }
  else
  {
    close(pd[1]);
    close(0);
    dup(pd[0]);
    close(pd[0]);
    exec(line, env, tail);
  }
  */
}

int exec(char line[128], char **env, char **args)
{
  //do IO redirect checking
  //MUST DO HERE SO THAT PARENT'S I/O IS PRESERVED
  check_IO_redirect(line, env);
  
  //check each dir for command, if not found tell the user execve failed
  for (int i = 0; i < ndir; i++)
  {
    strcpy(line, dir[i]);
    strcat(line, "/");
    strcat(line, args[0]);
    execve(line, args, env);
  }

  perror("execve failed");
  exit(1);
}

int main(int argc, char *argv[], char *env[])
{
  int i;
  int pid, status;
  char *cmd;
  char line[128];

  //read path names from *env[]
  read_path(env);

  //show dirs
  for (i = 0; i < ndir; i++)
    printf("dir[%d] = %s\n", i, dir[i]);

  while (1)
  {
    printf("sh %d running\n", getpid());
    printf("enter a command line : ");
    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;
    if (line[0] == 0)
      continue;
    if (line[0] == ' ')
      continue;

    tokenize(line);

    for (i = 0; i < n; i++)
    { // show token strings
      printf("arg[%d] = %s\n", i, arg[i]);
    }
    getchar();

    cmd = arg[0]; // line = arg0 arg1 arg2 ...

    if (strcmp(cmd, "cd") == 0)
    {
      chdir(arg[1]);
      continue;
    }
    if (strcmp(cmd, "exit") == 0)
      exit(0);

    pid = fork();

    if (pid)
    {
      printf("sh %d forked a child sh %d\n", getpid(), pid);
      printf("sh %d wait for child sh %d to terminate\n", getpid(), pid);
      pid = wait(&status);
      printf("ZOMBIE child=%d exitStatus=%x\n", pid, status);
      printf("main sh %d repeat loop\n", getpid());
    }
    else
    {
      printf("child sh %d running\n", getpid());

      //do pipe checking
      if(clean_pipes(line, env) == -1)
        exec(line, env, arg);
    }
  }
}

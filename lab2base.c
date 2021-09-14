/*
	BASE CODE: CPTS 360 lab2base.c
	modified by Ethan Vincent
*/

#include <stdio.h>             	// for I/O
#include <stdlib.h>            	// for I/O
#include <libgen.h>       	// for dirname()/basename()
#include <string.h>		// for c string functions

#define LINE_SIZE 128
#define PATHNAME_SIZE 64
#define COMMAND_SIZE 16
#define BUFFER_SIZE 512

typedef struct node
{
         char  name[PATHNAME_SIZE];       // node's name string
         char  type;           // 'D' for DIR; 'F' for file
   struct node *child, *sibling, *parent;
}NODE;


int mkdir(char *pathname);
int rmdir(char *pathname);
int ls(char *pathname);
int cd(char *pathname);
int pwd();
int creat(char *name);
int rm(char *pathname);
int reload(char *filename);
int save(char *filename);
int menu();
int quit();

int initialize();

void save_helper(FILE *outfile, NODE *p);
void pwd_helper(NODE *cwd);
NODE* check_existence(char *dirname, NODE *p);
NODE* split_pathname(char *dirname, char *basename, char file_type);
int delete_child(NODE *parent, NODE *p);
int delete_tree();

NODE *root, *cwd, *start;
char line[LINE_SIZE];
char command[COMMAND_SIZE], pathname[PATHNAME_SIZE];
void get_absolute_pathname(NODE *p, char *abs_path);
char* return_abs_path(NODE *p);

//               0        1        2     3     4      5        6     7         8       9       10   
char *cmd[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm", "reload", "save", "menu", "quit", NULL};

int (*fptr[]) (char*) = { (int (*)()) mkdir, rmdir, ls, cd, pwd, creat, rm, reload, save, menu, quit };

int findCmd(char *command)
{
	int i = 0;
  	while(cmd[i])
  	{
    		if (strcmp(command, cmd[i]) == 0)
      			return i;
    		i++;
  	}
	return -1;
}

NODE *search_child(NODE *parent, char *name)
{
	NODE *p;
  	printf("search for %s in parent DIR\n", name);
  	p = parent->child;
  	if (p == 0)
    		return 0;
  	
	while(p)
	{
    		if (strcmp(p->name, name)==0)
      			return p;
    		p = p->sibling;
  	}
	
	return 0;
}

int insert_child(NODE *parent, NODE *q)
{
	NODE *p;
  	printf("insert NODE %s into END of parent child list\n", q->name);
  	p = parent->child;
  	if (p==0)
    		parent->child = q;
  	
	else
	{
    		while(p->sibling)
      			p = p->sibling;
    		p->sibling = q;
  	}
  	
	q->parent = parent;
  	q->child = 0;
  	q->sibling = 0;
}

int delete_child(NODE *parent, NODE *p)
{
	NODE* q;
	printf("delete NODE %s from parent child list\n", q->name);
	q = parent->child;
	
	//p is the first item in list
	if (q == p)
	{
		//CASE 1: p is the only item in list
		if (q->sibling == NULL)
			q->parent->child = NULL;
		
		//CASE 2: p has a sibling
		else
			q->parent->child = p->sibling;
	}
	
	//must look for p in list
	else
	{
		while (q->sibling != p)
			q = q->sibling;
		
		//CASE 3: p is last item in list
		if (q->sibling->sibling == NULL)
			q->sibling = NULL;
		
		//CASE 4: p is in middle of list
		else
			q->sibling = q->sibling->sibling;
	
	}
	
	free(p);
	return 0;
}

NODE* check_existence(char *dirname, NODE *p)
{
	if (dirname[0] != '\0')
        {
        	//check for existence of dirname
               	printf("check whether %s exists\n", dirname);
                char *temp = strtok(dirname, "/");
               	p = search_child(start, temp);
                while (temp != NULL)
                {
                	if (!p)
                        {
                                return NULL;
                        }
                        
			temp = strtok(NULL, "/");
                        if (temp != NULL)
                        	p = search_child(p, temp);
                }
	
		return p;
	}
	
	else
		return start;
}

NODE* split_pathname(char dirname[PATHNAME_SIZE], char basename[PATHNAME_SIZE], char file_type)
{
	NODE* p;
	char pathnameA[PATHNAME_SIZE];
        if (pathname[0] != '/')
	{
		char temp_pathname[PATHNAME_SIZE] = "";
		get_absolute_pathname(cwd, temp_pathname);
		strcpy(pathnameA, temp_pathname);
		strcat(pathnameA, pathname);
	}
	
	else
		strcpy(pathnameA, pathname);
      
      	printf("pathnameA = %s\n", pathnameA);
	char *temp = strtok(pathnameA, "/");

	if (pathnameA[0] == '/')
        {
                start = root;
                while (temp != NULL)
                {	
			char* temp1 = temp;
                        temp = strtok(NULL, "/");
                        if (temp == NULL)
				strcpy(basename, temp1);
                       
			else
                       	{		
				strcpy(dirname + strlen(dirname), "/");
                               	strcpy(dirname + strlen(dirname), temp1);
                       	}
                }
		
		printf("dirname = %s, basename = %s\n", dirname, basename);
                p = check_existence(dirname, p);
                if(!p)
                	return NULL;
		
		if (p->type != file_type)
			return NULL;
        }
		
	return p;
}

void get_absolute_pathname(NODE *p, char *abs_path)
{		
	if (p->parent != p)
		get_absolute_pathname(p->parent, abs_path);
	
	strcat(abs_path, p->name);
	if (strcmp(p->name, "/"))
		strcat(abs_path, "/");
}

void pwd_helper(NODE* cwd)
{
        if (cwd->parent != cwd)
                pwd_helper(cwd->parent);
        if (strcmp(cwd->name, "/") == 0)
                printf("%s", cwd->name);
        else
                printf("%s/", cwd->name);
}

void save_helper(FILE *outfile, NODE* p)
{
	if (p == NULL)
		return;
	
	char temp[PATHNAME_SIZE] = "";
	get_absolute_pathname(p, temp);
	
	fprintf(outfile, "%c\t%s\n", p->type, temp);
	save_helper(outfile, p->child);
	save_helper(outfile, p->sibling);

}

int delete_tree(NODE *r)
{
	if (!r)
		return 0;
	
	delete_tree(r->child);
	
	while (r->child)
		delete_child(r, r->child);
	
	delete_tree(r->sibling);

}

/***************************************************************
 This mkdir(char *name) makes a DIR in the current directory
 You MUST improve it to mkdir(char *pathname) for ANY pathname

 Completed. mkdir(char *pathname) creates a new directory for a
 given pathname.
****************************************************************/

int mkdir(char *pathname)
{
	if (pathname[0] == '\0')
	{
		printf("ERROR: mkdir - enter a pathname.\n");
		return -1;
	}
  	
	NODE *p, *q;

  	if (!strcmp(pathname, "/") || !strcmp(pathname, ".") || !strcmp(pathname, ".."))
  	{
    		printf("ERROR: mkdir - can't mkdir with name %s.\n", pathname);
    		return -1;
  	}	

  	
  	//containers needed to split pathname into dirname and basename
  	char pathnameA[PATHNAME_SIZE], dirname[PATHNAME_SIZE] = "", basename[PATHNAME_SIZE] = "";
  	strcpy(pathnameA, pathname);
  	char *temp = strtok(pathnameA, "/");

	//check if pathname only consists of forward slashes
	int flag1 = 1;
	if (pathname[1] == '/')
	{
		for (int i = 2; pathnameA[i] != '\0'; i++)
		{
			if (pathnameA[i] != '/')
			{
				flag1 = 0;
				break;
			}
		}
		
		if (flag1)
		{
			printf("ERROR: mkdir - %s is not a valid pathname.\n", pathname);
			return -1;
		}
	}

	//tokenize pathname
	p = (split_pathname(dirname, basename,'D'));
	if (!p)
		return -1;
	
	//check if basename already exists
	printf("check whether %s already exists\n", basename);
  	if (search_child(p, basename))
  	{
    		printf("ERROR: mkdir - name %s already exists.\n", pathname);
    		return -1;
  	}
		
  	printf("--------------------------------------\n");
  	printf("ready to mkdir %s\n", basename);
  	q = (NODE *)malloc(sizeof(NODE));
  	q->type = 'D';
  	strcpy(q->name, basename);
  	insert_child(p, q);
  	printf("mkdir %s OK\n", pathname);
  	printf("--------------------------------------\n");
   
  	return 0;
}

int rmdir(char *pathname)
{	
	if (pathname[0] == '\0')
	{
		printf("ERROR: rmdir - enter a pathname.\n");
		return -1;
	}

	NODE* p;
	char pathnameA[PATHNAME_SIZE];
	strcpy(pathnameA, pathname);
	//start from root
  	if (pathname[0] == '/')
	{
		start = root;
		//dont let user delete root directory
		if (pathname [1] == '\0')
		{
			printf("ERROR: rmdir - can not remove root directory.\n");
			return -1;
		}
		
		//check if pathname consists of only forward slashes
		int flag1 = 1;
		if (pathnameA[1] == '/')
		{
			for (int i = 2; pathnameA[i] != '\0'; i++)
			{
				if (pathnameA[i] != '/')
				{
					flag1 = 0;
					break;
				}
			}
			
			if (flag1)
			{
				printf("%s is an invalid pathname.\n", pathname);
				return -1;
			}
		}
	}
	
	//start from cwd
	else
		start = cwd;

	//if pathname is not empty, tokenize pathname using "/" as delimeter
	if (pathnameA[0] != '\0')
	{
		//confirm existence of pathname
		printf("Check whether %s exists\n", pathname);
		char *temp = strtok(pathnameA, "/");
		p = search_child(start, temp);
		while (temp != NULL)
		{
			//if p == NULL, directory does not exist
			if (!p)
			{
				printf("Error: rmdir - %s does not exist in specified directory.\n", temp);
				return -1;
			}
			temp = strtok(NULL, "/");
			//on the last iteration of loop, we want p to keep the address of the node to be deleted
			if (temp != NULL)
				p = search_child(p, temp);
		}
	}
	
	//if pathname is empty then call was made incorrectly from main
	else
	{
		printf("Error: rmdir - pathname must be specified.\n");
		return -1;
	}

	//check if p is a directory
	if (p->type == 'D')
	{
		//ensure p is empty
		if (p->child == NULL)
		{
			if (p == cwd)
			{
				printf("ERROR: rmdir - %s is the cwd. Undefined behavior. Call returned here, no deletion performed.\n", p->name);
				return -1;
			}
			
			char temp_name[PATHNAME_SIZE];
			strcpy(temp_name, p->name);
			printf("--------------------------------------\n");
			printf("ready to rmdir %s\n", temp_name);
		
			//remove p from p->parent's child list
			int c = delete_child(p->parent, p);
			
			if (c == 0)
    			{
				printf("SUCCESS: rmdir - %s deleted.\n", temp_name);
				printf("--------------------------------------\n");
			}
			
			else
				printf("ERROR: rmdir - delete_child(p->parent, p) failed.\n");
		}
		
		else
		{
			printf("Error: rmdir - %s is not empty.\n", p->name);
			return -1;
		}
			
	}	
	
	else
	{
		printf("Error: rmdir - %s is not a directory.\n", p->name);
		return -1;
	}

	
	return 0;
}

// This ls() list CWD. You MUST improve it to ls(char *pathname)
//Done.
int ls(char *pathname)
{	
	if (!strcmp(pathname, "/"))
	{
		NODE* q = root->child;
		printf("--------------------------------------\n");
		
		while (q != NULL)
		{
			printf("%s contents:\n", pathname);
			printf("[%c %s] ", q->type, q->name);
			q = q->sibling;
		}
		
		printf("\n--------------------------------------\n");
		return 0;
	}
	
	if (!strcmp(pathname, "."))
	{
		NODE* q = cwd->child;
		if (!q)
			return 0;
		
		printf("--------------------------------------\n");
		printf("%s contents:\n", cwd->parent->name);
		
		while (q != NULL)
		{
			printf("[%c %s] ", q->type, q->name);
			q = q->sibling;
		}
		
		printf("\n--------------------------------------\n");
		return 0;
	}

	if (!strcmp(pathname, ".."))
	{
		NODE* q;
		if (cwd->parent != cwd)
			q = cwd;
		
		else
			q = cwd->child;
		
		printf("--------------------------------------\n");
		printf("%s contents:\n", q->parent->name);
		
		while (q != NULL)
		{
			printf("[%c %s] ", q->type, q->name);
			q = q->sibling;
		}
		
		printf("\n--------------------------------------\n");
		return 0;
	}

	if (pathname[0] == '\0')
	{
		NODE *q = cwd->child;
		printf("--------------------------------------\n");
                printf("%s contents:\n", cwd->name);
		
		while (q != NULL)
                {       
                        printf("[%c %s] ", q->type, q->name);
                        q = q->sibling;
                
		}
               	printf("\n--------------------------------------\n");
                return 0;
	}

	NODE* p;
	char dirname[PATHNAME_SIZE] = "", basename[PATHNAME_SIZE] = "";
	
	p = split_pathname(dirname, basename, 'D');
	if (!p)
		return -1;
	
	p = search_child(p, basename);
	if (!p)
		return -1;
	
	else
	{
		if (p->type != 'D')
		{
			printf("ERROR: ls - %s is not a directory.\n", basename);
			return -1;
		}

		printf("--------------------------------------\n");
		printf("%s contents:\n", pathname);
		p = p->child;
		
		while (p != NULL)
		{
			printf("[%c %s] ", p->type, p->name);
			p = p->sibling;
		}
		
		printf("\n--------------------------------------\n");
	}	
	
	return 0;
}

int cd(char *pathname)
{
	if (pathname[0] == '\0')
	{
		printf("ERROR: cd - enter a pathname.\n");
		return -1;
	}

	NODE *p;
	//split pathname into dirname and basename
        char dirname[PATHNAME_SIZE] = "", basename[PATHNAME_SIZE] = "";	
	if (!strcmp(pathname, "/"))
	{
		cwd = root;
		return 0;
	}
	
	
	if (!strcmp(pathname, "."))
	{
		return 0;
	}
	
	if (!strcmp(pathname, ".."))
	{
		if (cwd->parent != NULL)
			cwd = cwd->parent;
		return 0;
	}
	

  	//split pathname
	p = (split_pathname(dirname, basename, 'D'));
	if (!p)
		return -1;
	
	p = search_child(p, basename);

	if (p)
	{
		if (p->type == 'D')
		{	
			cwd = p;
			return 0;
		}
		
		printf("ERROR: cd - %s is not a directory.\n", basename);
		return -1;
	}
	
	else
	{
		printf("ERROR: cd - %s does not exist.\n", basename);
		return -1;
	}
}

int pwd()
{
	pwd_helper(cwd);  		
	printf("\n");
}

int creat(char *name)
{
	if (pathname[0] == '\0')
        {
                printf("ERROR: creat - enter a pathname.\n");
                return -1;
        }
        NODE *p, *q;

        if (!strcmp(pathname, "/") || !strcmp(pathname, ".") || !strcmp(pathname, ".."))
        {
                printf("ERROR: creat - can't creat with name %s.\n", pathname);
                return -1;
        }

	//containers needed to split pathname into dirname and basename
        char pathnameA[PATHNAME_SIZE], dirname[PATHNAME_SIZE] = "", basename[PATHNAME_SIZE] = "";
        strcpy(pathnameA, pathname);
        char *temp = strtok(pathnameA, "/");

        //check if pathname only consists of forward slashes
        int flag1 = 1;
        if (pathname[1] == '/')
        {
                for (int i = 2; pathnameA[i] != '\0'; i++)
                {
                        if (pathnameA[i] != '/')
                        {
                                flag1 = 0;
                                break;
                        }
                }
                if (flag1)
                {
                        printf("ERROR: creat - %s is not a valid name.\n", pathname);
                        return -1;
                }
        }

        //tokenize pathname
        p = (split_pathname(dirname, basename, 'D'));
        if (!p)
                return -1;



        //check if basename already exists
        printf("check whether %s already exists\n", basename);
        if (search_child(p, basename))
        {
                printf("ERROR: creat - %s already exists.\n", pathname);
                return -1;
        }

	printf("--------------------------------------\n");
        printf("ready to creat %s\n", basename);
        q = (NODE *)malloc(sizeof(NODE));
        q->type = 'F';
        strcpy(q->name, basename);
        insert_child(p, q);
        printf("creat %s OK\n", pathname);
        printf("--------------------------------------\n");
  	
	return 0;
}

int rm(char *pathname)
{
	if (pathname[0] == '\0')
        {
                printf("ERROR: rm - enter a name.\n");
                return -1;
        }
	
	 if (!strcmp(pathname, "/"))
        {
                printf("ERROR: rm - %s is a directory.\n", pathname);
                return -1;
        }

	//check if pathname only consists of forward slashes
        int flag1 = 1;
        if (pathname[0] == '/' && pathname[1] == '/')
        {
                for (int i = 2; pathname[i] != '\0'; i++)
                {
                        if (pathname[i] != '/')
                        {
                                flag1 = 0;
                                break;
                        }
                }
                if (flag1)
                {
                        printf("ERROR: creat - %s is not a valid name.\n", pathname);
                        return -1;
                }
        }


        NODE* p;
	char dirname[PATHNAME_SIZE] = "", basename[PATHNAME_SIZE] = "";
        
	p = split_pathname(dirname, basename, 'D');
	if (!p)
		return -1;

	p = search_child(p, basename);
	if(!p)
	{	
		printf("ERROR: rm - %s does not exist.\n", basename);
		return -1;
	}

	//check if p is a directory
        if (p->type == 'F')
        {      
		char temp_name[PATHNAME_SIZE];
                strcpy(temp_name, p->name);
                printf("--------------------------------------\n");
                printf("ready to rm %s\n", temp_name);

                //remove p from p->parent's child list
                int c = delete_child(p->parent, p);

                if (c == 0)
                {
                    	printf("SUCCESS: rm - %s deleted.\n", temp_name);
                       	printf("--------------------------------------\n");
                }
               	
		else
               	{
			printf("ERROR: rm - delete_child(p->parent, p) failed.\n");
                	return -1;
		}   
        }
        
	else
        {
                printf("Error: rm - %s is a directory.\n", p->name);
                return -1;
        }

  	return 0;
}

int reload(char *filename)
{
	delete_tree(root);
	
	initialize();
	FILE *save_file;
	printf("test1\n");
	if (!strcmp(filename, ""))
		save_file = fopen("save_file_LAB2.dat", "r");

	else
		save_file = fopen(filename, "r");
	printf("test2\n");
	if (!save_file)
	{
		printf("ERROR: reload - %s does not exist.\n", filename);
		return -1;
	}

	char buffer[BUFFER_SIZE] = "";
	//read in the first line (the heading info)
	fgets(buffer, BUFFER_SIZE, save_file);
	
	while (fgets(buffer, BUFFER_SIZE, save_file))
	{
		char dirname[PATHNAME_SIZE] = "", basename[PATHNAME_SIZE] = "";
		char *type = strtok(buffer, "\t");
		char *pn = strtok(NULL, "\n"); 
		if (strcmp(pn, "/\n"))
		{	
			printf("pn = \"%s\"\n", pn);
			//skip inserting root
			if (type[0] == 'D')
			{
				strcpy(pathname, pn);
				mkdir(pn);
			}
			else if (type[0] == 'F')
			{
				strcpy(pathname, pn);
				creat(pn);
			}
		}
	}
	
	return 0;
}

int save(char *filename)
{
	FILE *save_file;
	//if no filename specified, this is the default save file name
	if (!strcmp(filename, ""))
		save_file = fopen("save_file_LAB2.dat", "w+");
	
	else
		save_file = fopen(filename, "w+");

	if (!save_file)
	{
		printf("ERROR: save - failed to open save file.\n");
		return -1;
	}
	
	//print heading to save_file.dat
	fprintf(save_file, "type\tpathname\n");
	
	char absolute_pathname[PATHNAME_SIZE] = "";
	NODE* p = root;

	save_helper(save_file, root);

	fclose(save_file);

  	return 0;
}

int menu()
{
  	return 0;
}

int quit()
{
	printf("Saving to default location...\n");
	save("");
  	printf("Program exit\n");
  	exit(0);
  	// improve quit() to SAVE the current tree as a Linux file
  	// for reload the file to reconstruct the original tree
}

int initialize()
{
    	root = (NODE *)malloc(sizeof(NODE));
    	strcpy(root->name, "/");
    	root->parent = root;
    	root->sibling = 0;
    	root->child = 0;
    	root->type = 'D';
    	cwd = root;
    	printf("Root initialized OK\n");
}

int main()
{
  	int index;

	//clear console window with regex
	printf("\e[1;1H\e[2J");

  	initialize();

  	printf("NOTE: commands = [mkdir|rmdir|creat|rm|cd|ls|save|quit|reload\n");

  	while(1)
  	{
		printf("\033[1;32m");
    		printf("User@Ethan-Vincent-Lab2-DEMO:");
		pwd_helper(cwd);
		printf("$ ");
		printf("\033[0m");
    		fgets(line, LINE_SIZE, stdin);
    		line[strlen(line)-1] = 0;

    		command[0] = pathname[0] = 0;
    		sscanf(line, "%s %s", command, pathname);
    		printf("command=%s pathname=%s\n", command, pathname);
      
    		if (command[0]==0) 
      			continue;

    		index = findCmd(command);
   		
		if (index >= 0) 
    			{int r = fptr[index](pathname);}  
      		else
			printf("That is not a valid command\n");
		
    		/*
    		ORIGINAL CALL METHOD
		switch (index)
    		{
      			case 0: mkdir(pathname); break;
      			case 1: ls();            break;
      			case 2: quit();          break;
    		}
    		*/
  	}
}


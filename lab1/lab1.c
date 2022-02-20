/*****Lab 1; CptS 360; Ethan Vincent******/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

/***** PART 1 *****/

char* ctable = "0123456789ABCDEF";
int DEC = 10; 		//to print decimal representation
int HEX = 16;		//to print hexadecimal representation
int OCT = 8;		//to print octal rep

int rpu(unsigned int x, int base)
{
	char c;
	if (x)
	{
		c = ctable[x % base];
		rpu(x / base, base);
		putchar(c);
	}
}

int printu(unsigned int x)
{
	(x==0)? putchar('0') : rpu(x, DEC);
}

void printd(int x)
{
	if (x < 1)
		putchar('-');	
	(x==0)? putchar('0') : rpu(abs(x), DEC);
}

void printx(unsigned int x)
{
	putchar('0');
	putchar('x');
	(x==0)? putchar('0') : rpu(x, HEX);
}

void printo(unsigned int x)
{
	putchar('0');
	(x==0)? putchar('0') : rpu(x, OCT);
}

void printc(char c)
{
	putchar(c);
}

void prints(char* str)
{
	for (int i = 0; str[i] != '\0'; i++)
	{
		putchar(str[i]);
	}
}

void myprintf(char *fmt, ...)
{
	char *cp = fmt;
	int *ip = (int*)&fmt + 1;
	//va_list args;	//book implementation to get arguments
	//va_start(args, fmt);
	while (*cp != '\0')
	{
		if (*cp == '%')
		{
			cp++;
			if (*cp == 'c')
			{
				//printc(va_arg(args, int));
				printc(*ip);
				ip++;
				cp++;
			}
			else if (*cp == 's')
			{
				//prints(va_arg(args, char*));
				prints((char*)*ip);
				ip++;
				cp++;
			}
			else if (*cp == 'u')
			{
				//printu(va_arg(args, unsigned int));
				printu(*ip);
				ip++;
				cp++;
			}
			else if (*cp == 'd')
			{
				//printd(va_arg(args, int));
				printd(*ip);
				ip++;
				cp++;
			}
			else if (*cp == 'o')
			{
				//printo(va_arg(args, unsigned int));
				printo(*ip);
				ip++;
				cp++;
			}
			else if (*cp == 'x')
			{
				//printx(va_arg(args, unsigned int));
				printx(*ip);
				ip++;
				cp++;
			}	
		}
		else if (*cp == '\n')
		{
			putchar('\n');
			cp++;
		}
		else 
		{
			putchar(*cp);
			cp++;
		}

	}
	//va_end(args);
}

/***** PART 2 *****/

typedef struct partition
{
	unsigned char drive;

	unsigned char head;	
	unsigned char sector;
	unsigned char cylinder;

	unsigned char sys_type;

	unsigned char end_head;
	unsigned char end_sector;
	unsigned char end_cylinder;

	unsigned int start_sector;
	unsigned int nr_sectors;
}Partition;

char *dev = "vdisk";
int fd;

//read a disk sector into char buf[512]
void read_sector(int fd, int sector, char *buf)
{
	lseek(fd, sector*512, SEEK_SET);
	read(fd, buf, 512);
}

/***** MAIN *****/

int main(int argc, char *argv[], char *env[])
{ 	
	/***** P1 MAIN *****/
	myprintf("myprintf test:\n");
	myprintf("argc = %d\n", argc);
	myprintf("*argv = ");
	//myprintf("argc = %d\n*argv = %s\n*env = %s\n", argc, *argv, *env);
	myprintf("argc = %d\n", argc);
	myprintf("*argv = ");
	for (int i = 0; i < argc; i++)
		myprintf("%s ", argv[i]);
	myprintf("\n*env = ");
	for (int i = 0; env[i] != NULL; i++)
		myprintf("%s\n", env[i]);

	/***** P2 MAIN *****/
	Partition* p;
	char buf[512];
	
	fd = open(dev, O_RDONLY);	//open dev for READ
	read_sector(fd, 0, buf);	//read in MBR at sector 0
	
	p = (Partition*)(&buf[0x1be]);	//p->P1

	//print Partitions' start_sector, nr_sectors, sys_type
	myprintf("\n");
	for (int i = 1; i < 5; i++)
	{
		myprintf("*****\n");
		myprintf("P%d start_sector = %u\n", i, p->start_sector);
		myprintf("P%d nr_sectors = %u\n", i, p->nr_sectors);
		myprintf("P%d sys_type = %x\n", i, p->sys_type);
		p++;
	}
	p--;
	myprintf("*****\n");

	//assume P4 is EXTEND type
	int extStart = p->start_sector;
	myprintf("\nextStart = %d\n", extStart);

	int localMBR = extStart;
	
	for (int i = 5; p->start_sector != 0; i++)
	{
		read_sector(fd, localMBR, buf);
		
		p = (Partition*)(&buf[0x1be]);
			
		//partition table of localMBR in buf[] has 2 entries
		myprintf("\n------\n");
		myprintf("Entry 1 start_sector = %u", p->start_sector);
		myprintf("   Entry 1 nr_sectors = %u\n", p->nr_sectors);
		unsigned int new_start_sector = localMBR + p->start_sector;
		unsigned int new_end_sector   = new_start_sector + p->nr_sectors - 1;
		unsigned int new_nr_sectors   = p->nr_sectors;
		p++;
		myprintf("Entry 2 start_sector = %u", p->start_sector);
		myprintf("   Entry 2 nr_sectors = %u\n", p->nr_sectors);
		myprintf("-----\n\n");

		//compute and print subsequent partitions' begin, end, nr_sectors
		myprintf("P%d start_sector = %u", i, new_start_sector);
		myprintf("   P%d end_sector = %u", i, new_end_sector);
		myprintf("   P%d nr_sectors = %u\n", i, new_nr_sectors);

		localMBR = extStart + p->start_sector;
		if (localMBR == extStart)
			myprintf("\nend of extended partitions\n");
		else
			myprintf("\nlocalMBR = %u\n", localMBR);
	}
	
}

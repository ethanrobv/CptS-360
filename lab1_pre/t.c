/************* t.c file ********************/
#include <stdio.h>
#include <stdlib.h>

int *FP;

int A(int x, int y);
int B(int x, int y);
int C(int x, int y);
extern void* getebp();


int main(int argc, char *argv[ ], char *env[ ])
{
	int a,b,c;
	printf("enter main\n");
  
	printf("&argc=%p argv=%p env=%p\n", (void*)&argc, (void*)&argv, (void*)&env);
	printf("&a=%8p &b=%8p &c=%8p\n", (void*)&a, (void*)&b, (void*)&c);

	//1. Write C code to print values of argc and argv[] entries
	printf("*argc=%d\n*argv=", argc);
	for(int i = 0; i < argc; i++)
	{
		printf("\n%s", argv[i]);
	}
	printf("\n");
	//end 1.

	a = 1; b = 2; c = 3;
	A(a,b);
	printf("\nexit main\n");
}

int __attribute__ ((noinline)) A(int x, int y)
{
	int d,e,f;
	printf("\n\nenter A\n");
	
	// write C code to print address of d,e,f
	printf("&d=%8p &e=%8p &f=%8p\n", (void*)&d, (void*)&e, (void*)&f);
	// done.

	d=4; e=5; f=6;
	B(d,e);
	printf("\nexit A\n");
}

int __attribute__ ((noinline)) B(int x, int y)
{
	int g,h,i;
	printf("\nenter B\n");
	
	// write C code to print address of g,h,i
	printf("&g=%8p &h=%8p &i=%8p\n", (void*)&g, (void*)&h, (void*)&i);
	//done.
	
	g=7; h=8; i=9;
	C(g,h);
	printf("\nexit B\n");
}

int __attribute__ ((noinline)) C(int x, int y)
{
	int u,v,w,i,*p;

	printf("\nenter C\n");

	// write C code to print address of u,v,w,i,p
	printf("&u=%8p &v=%8p &w=%8p &i=%8p &p=%8p\n", (void*)&u, (void*)&v, (void*)&w, (void*)&i, &p);
	//done.

	u=10; v=11; w=12; i=13;

	FP = (int *)getebp(); // FP = stack frame pointer of the C() function
	printf("FP=%8p\n", FP);

	//2. write C code to print the stack frame link list
	printf("stack frame linked list\n");
	while(FP)
	{
		printf("%p -> ", (void*)FP);
		FP = (int*)*FP;
	}
	printf("%p",(void*)FP);	
	//done 2.

	p = (int *)&p;
	p = (int *)&u;

	//3. print the stack contents from p to the frame of main()
	//YOU MAY JUST PRINT 128 ENTIRES OF THE STACK CONTENTS
	printf("\n128 stack entries\n");
	for(int i = 0; i < 128; i++)
	{
		printf("%p     %x\n", (void*)p, *p);
		p++;
	}
	//done 3.
	
	printf("\nexit C\n");
	//4. on a hard copy of the print out, identify the stack cotents as 
	//LOCAL VARIABLES, PARAMTERS< stack frame pointer of each function
}

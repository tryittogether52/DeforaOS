/* echo.c */



#include <unistd.h>
#include <stdio.h>


/* echo */
static int _echo(int argc, char * argv[])
{
	int i;

	if(argc != 0)
	{
		printf("%s", argv[0]);
		for(i = 1; i < argc; i++)
			printf(" %s", argv[i]);
	}
	fputc('\n', stdout);
	return 0;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, "%s", "Usage: echo [string...]\n");
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;

	while((o = getopt(argc, argv, "")) != -1)
	{
		switch(o)
		{
			case '?':
				return _usage();
		}
	}
	return _echo(argc - 1, &argv[1]);
}

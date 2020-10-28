#include <unistd.h>
#include <bfd.h>
#include <stdio.h>
#include <stdlib.h>

static struct option options[] = {
	{"all",no-argument,0,'a'},
	{0,no-argument,0,0}
};

static void options_process(int argc,char *argv[])
{
	int c = 0;
	while((c = getopt_long(argc,argv,"a",options,NULL)) != EOF){
		switch(c){
			case 0:
				break;
			case 'a':
				break;
		}
	}
}

static struct bfd file_bfd;


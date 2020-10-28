#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define file_type(f_types) (\
		(ET_NONE == f_types) ? "NONE" : (\
		(ET_REL  == f_types) ? "REL"  : (\
		(ET_EXEC == f_types) ? "EXEC" : (\
		(ET_DYN  == f_types) ? "DYN" : (\
		(ET_CORE == f_types) ? "CORE": (\
		(ET_LOPROC == f_types) ? "LOPROC" : (\
		(ET_HIPROC == f_types) ? "HIPROC" : "UNKNOWN")))))))

static void print_ehdr(Elf64_Ehdr *ehdr)
{
	int i = 0;

	printf("Magic number:\t");
	for(i = 0;i < EI_NIDENT;i++){
		printf(" %x",ehdr->e_ident[i]);
	}
	printf("\n Object file type:\t0x%x",ehdr->e_type);
	printf("\n Arch:            \t0x%x",ehdr->e_machine);
	printf("\n Object file version:\t0x%x",ehdr->e_version);
	printf("\n Entry point vir addr:\t0x%lx",ehdr->e_entry);
	printf("\n Program header table file offset:\t0x%lx",ehdr->e_phoff);
	printf("\n Section header table file offset:\t0x%lx",ehdr->e_shoff);
	printf("\n Processor-specific flags:\t0x%x",ehdr->e_flags);
	printf("\n ELF header size in bytes:\t0x%x",ehdr->e_ehsize);
	printf("\n Program header table entry size:\t0x%x",ehdr->e_phentsize);
	printf("\n Program header table entry count:\t0x%x",ehdr->e_phnum);
	printf("\n Section header table entry size:\t0x%x",ehdr->e_shentsize);
	printf("\n Section header table entry count:\t0x%x",ehdr->e_shnum);	
	printf("\n Section header string table index:\t0x%x\n",ehdr->e_shstrndx);

}

int main(int argc,char *argv[])
{
	Elf64_Ehdr	ehdr;

	int fd = 0;
	int ret = 0;
	void *buf = NULL;
	int len = 0;
	struct stat state;
	if(2 > argc){
		return -1;
	}

	ret = stat(argv[1],&state);
	if(ret)
		return ret;
	fd = open(argv[1],O_RDONLY);
	if(0 > fd){	
		return -1;
	}
	len = state.st_size;	
	ret = read(fd,&ehdr,sizeof(Elf64_Ehdr));
	if(sizeof(Elf64_Ehdr) != ret){
		return -2;
	}
	buf = mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
	if(!buf){
		close(fd);
		printf("%s:%d\n",__func__,__LINE__);
		return -1;
	}
			
	print_ehdr(&ehdr);
	munmap(buf,len);
	close(fd);
	return 0;
}

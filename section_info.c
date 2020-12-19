#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct type_hash {
	unsigned int index;
	char *name;
};

struct type_hash sh_type_index[] = {
	{
		.index = SHT_NULL,
		.name = "NULL",
	},
	{
		.index = SHT_PROGBITS,
		.name = "PROGBITS",
	},
	{
		.index = SHT_SYMTAB,
		.name = "SYMTAB",
	},
	{
		.index = SHT_STRTAB,
		.name = "STRTAB",
	},
	{
		.index = SHT_RELA,
		.name = "RELA",
	},
	{
		.index = SHT_HASH,
		.name = "HASH",
	},
	{
		.index = SHT_DYNAMIC,
		.name = "DYNAMIC",
	},
	{
		.index = SHT_NOTE,
		.name = "NOTE",
	},
	{
		.index = SHT_NOBITS,
		.name = "NOBITS",
	},
	{
		.index = SHT_REL,
		.name = "REL",
	},
	{
		.index = SHT_SHLIB,
		.name = "SHLIB",
	},
	{
		.index = SHT_DYNSYM,
		.name = "DYNSYM",
	},
	{
		.index = SHT_INIT_ARRAY,
		.name = "INIT_ARRAY",
	},
	{
		.index = SHT_FINI_ARRAY,
		.name = "FINI_ARRAY",
	},
	{
		.index = SHT_PREINIT_ARRAY,
		.name = "PREINIT_ARRAY",
	},
	{
		.index = SHT_GROUP,
		.name = "GROUP",
	},
	{
		.index = SHT_SYMTAB_SHNDX,
		.name = "GROUP",
	},
	{
		.index = SHT_NUM,
		.name = "NUM",
	},
	{
		.index = SHT_LOOS,
		.name = "LOOS",
	},
	{
		.index = SHT_GNU_ATTRIBUTES,
		.name = "GNU_ATTRIBUTES",
	},
	{
		.index = SHT_GNU_HASH,
		.name = "GNU_HASH",
	},
	{
		.index = SHT_GNU_LIBLIST,
		.name = "GNU_LIBLIST",
	},
	{
		.index = SHT_CHECKSUM,
		.name = "CHECKSUM",
	},
	{
		.index = SHT_LOSUNW,
		.name = "LOSUNW",
	},
	{
		.index = SHT_SUNW_move,
      	        .name = "SUNW_move",
	},
	{
		.index = SHT_SUNW_COMDAT,
		.name = "SUNW_COMDAT",
	},
	{
		.index = SHT_SUNW_syminfo,
		.name = "SUNW_syminfo;=",
	},
	{
		.index = SHT_GNU_verdef,
		.name = "GNU_verdef",
	},
	{
		.index = SHT_GNU_verneed,
		.name = "GNU_verneed",
	},
	{
		.index = SHT_GNU_versym,
		.name = "GNU_versym",
	},
	{
		.index = SHT_HISUNW,
		.name = "HISUNW",
	},
	{
		.index = SHT_HIOS,
		.name = "HIOS",
	},
	{
		.index = SHT_LOPROC,
		.name = "LOPROC",
	},
	{
		.index = SHT_HIPROC,
		.name = "HIPROC",
	},
	{
		.index = SHT_LOUSER,
		.name = "LOUSER",
	},
	{
		.index = SHT_HIUSER,
		.name = "HIUSER",
	},
	{}
	};

#define sh_type(s_type) ({ \
			int size_array = sizeof(sh_type_index)/sizeof(sh_type_index[0]),i = 0; \
			for(i = 0;i < size_array;i++)\
				if(sh_type_index[i].index == s_type)\
					break;\
			(i == size_array) ? "NULL" : sh_type_index[i].name \
			;})


struct type_hash sh_flags_index[] = {
	{
		.index = SHF_WRITE,
		.name = "W",
	},	
	{
		.index = SHF_ALLOC,
		.name = "A",
	},
	{
		.index = SHF_EXECINSTR,
		.name = "X",
	},
	{
		.index = SHF_MERGE,
		.name = "M",
	},
	{
		.index = SHF_STRINGS,
		.name = "S",
	},
	{
		.index = SHF_INFO_LINK,
		.name = "I",
	},
	{
		.index = SHF_LINK_ORDER,
		.name = "L",
	},
	{
		.index = SHF_OS_NONCONFORMING,
		.name = "O",
	},
	{
		.index = SHF_GROUP,
		.name = "G",
	},	
	{
		.index = SHF_TLS,
		.name = "T",
	},	
	{
		.index = SHF_COMPRESSED,
		.name = "C",
	},
	{
		.index = SHF_MASKOS,
		.name = "+MO+",
	},	
	{
		.index = SHF_MASKPROC,
		.name = "+MASKPROC+",
	},
	{
		.index = SHF_ORDERED,
		.name = "+ORDERED+",
	},
	{
		.index = SHF_EXCLUDE,
   		.name = "+EXCLUDE+",
	},
	{}
};


#define sh_flags(sh_flags) ({\
			int h = 0,s_offset = 0,size_array = sizeof(sh_flags_index)/sizeof(sh_flags_index[0]),i = 0; \
			char buf[1024];\
			for(i = 0;i < size_array;i++)\
				if(sh_flags & sh_flags_index[i].index){\
				     strcpy(buf + s_offset,sh_flags_index[i].name);\
				     s_offset +=strlen(sh_flags_index[i].name);\
				     h++;\
				     }\
			(h == 0) ? "" : buf\
			;})

static void print_shdr(void **buf,Elf64_Ehdr *ehdr)
{
	Elf64_Shdr	*shdr = NULL;
	int i = 0,ret = 0;
	char type_name[64];
	char *addr = (char *)*buf;
	int str_offset = 0;
	char *str = NULL;
	
	printf("\t Section header table e_shnum = %d\n",ehdr->e_shnum);
	printf("\tname\ttype\t flags\tvirtual addr\tfile offset\t size\tlink\tinfo\talign\tentry size\n");
	shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + ehdr->e_shstrndx * sizeof(Elf64_Shdr)];
	str_offset = shdr->sh_offset;	
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		printf("[%d]  %-16s  %-8s  %08s  %016lx  %016lx  %08x  %08x  %16lx  %016lx\n",i,(char *)&addr[str_offset + shdr->sh_name],sh_type(shdr->sh_type),sh_flags(shdr->sh_flags),shdr->sh_offset,shdr->sh_size,shdr->sh_link,shdr->sh_info,shdr->sh_addralign,shdr->sh_entsize);	
	
	}

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
			
	print_shdr(&buf,&ehdr);
	munmap(buf,len);
	close(fd);
	return 0;
}

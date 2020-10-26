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



#define sh_type(s_type) (\
		(SHT_PROGBITS == s_type)?\
			"PROGBITS":(\
		(SHT_SYMTAB == s_type) ?\
			"SYMTAB":(\
		(SHT_STRTAB == s_type) ?\
			"STRTAB":(\
	        (SHT_RELA == s_type) ?\
			"RELA":(\
	        (SHT_HASH == s_type) ?\
			"HASH":(\
	        (SHT_DYNAMIC == s_type) ? \
			"SYNAMIC":(\
	        (SHT_NOTE == s_type) ? \
			"NOTE":(\
	        (SHT_NOBITS == s_type) ? \
			"NOBITS":(\
	        (SHT_REL == s_type) ? \
			"REL": (\
	        (SHT_SHLIB == s_type) ? \
			"SHLIB": (\
	        (SHT_DYNSYM == s_type) ? \
			"SYNSYM":(\
	        (SHT_LOPROC == s_type) ? \
			"LOPROC":(\
	        (SHT_HIPROC == s_type) ? \
			"HIPROC":(\
	        (SHT_LOUSER == s_type) ? \
			"LOUSER":(\
	        (SHT_HIUSER == s_type) ? \
			"HIUSER":"UNKNOWN")))))))))))))))

#define sh_flags(sh_flags) (\
		(SHF_WRITE == sh_flags) ? "W" : (\
		(SHF_ALLOC == sh_flags) ? "A" : (\
		(SHF_EXECINSTR == sh_flags) ? "E" : (\
		((SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR) == sh_flags) ? "WAE" : (\
		((SHF_WRITE | SHF_ALLOC ) == sh_flags) ? "WA" : (\
		((SHF_WRITE | SHF_EXECINSTR ) == sh_flags) ? "WE" : (\
		((SHF_ALLOC | SHF_EXECINSTR ) == sh_flags) ? "AE" : "UNKNOWN")))))))
		       //	(\
		//(SHF_RELA_LIVEPATCH == sh_flags) ? "LIVEPATCH" : (\
	//	(SHF_RO_AFTER_INIT  == sh_flags) ? "RO_AFTER_INIT" : (\
	//	(SHF_MASKPROC       == sh_flags) ? "MASKPROC":"UNKNOWN"))))))

#define ph_type(ph_type)(\
		(PT_LOAD == ph_type)?\
			"LOAD":(\
		(PT_DYNAMIC == ph_type) ?\
			"DYNAMIC":(\
		(PT_INTERP == ph_type) ?\
			"INTERP":(\
	        (PT_NOTE == ph_type) ?\
			"NOTE":(\
	        (PT_SHLIB == ph_type) ?\
			"SHLIB":(\
	        (PT_PHDR == ph_type) ? \
			"PHDR":(\
	        (PT_TLS == ph_type) ? \
			"TLS":(\
	        (PT_LOOS == ph_type) ? \
			"LOOP":(\
	        (PT_HIOS == ph_type) ? \
			"HIOS": (\
	        (PT_LOPROC == ph_type) ? \
			"LOPROC": (\
	        (PT_HIPROC == ph_type) ? \
			"HIPROC":(\
	        (PT_GNU_EH_FRAME == ph_type) ? \
			"EH_FRAME":(\
	        (PT_GNU_STACK == ph_type) ? \
			"STACK":"UNKNOWN")))))))))))))
#define ph_flags(p_flags) (\
	((PF_W | PF_X | PF_R) == p_flags) ? "R W X" \
        	: (\
	((PF_W | PF_X) == p_flags) ? "W X" : (\
	((PF_R | PF_X) == p_flags) ? "R X" : (\
	((PF_R | PF_W) == p_flags) ? "R W" : (\
	(PF_W == p_flags) ? "W" : (\
	(PF_R == p_flags) ? "R" : (\
	(PF_R == p_flags) ? "X" : "UNKNOWN")))))))



#define sym_type(s_type) (\
		(STT_NOTYPE == s_type) ? "NOTYPE" : (\
		(STT_OBJECT == s_type) ? "OBJECT" : (\
		(STT_FUNC == s_type) ? "FUNC" : (\
		(STT_SECTION == s_type) ? "SECTION" : (\
		(STT_FILE == s_type) ? "FILE" : (\
		(STT_LOPROC == s_type) ? "LOPROC" : (\
		(STT_HIPROC == s_type) ? "HIPROC" : "NOTYPE")))))))

//#define 



static void dis_program(void **buf,Elf64_Ehdr *ehdr)//SHT_PROGBITS
{

}
static void print_nhdr(void **buf,Elf64_Ehdr *ehdr)
{
	Elf64_Shdr *shdr = NULL;
	char *addr = (char *)*buf;
	int i = 0,flag = 0;
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		if(SHT_NOTE == shdr->sh_type){
			note_info(ehdr,buf,shdr->sh_offset,shdr->sh_size);
		}

	}

}



static void dsym_info(Elf64_Ehdr *ehdr,void **buf,Elf64_Off sh_offset,Elf64_Xword sh_size)
{

	Elf64_Shdr *shdr = NULL;
	Elf64_Sym *sym = NULL;
	int i = 0,size = 0;
	Elf64_Off str_offset = 0;

	char *addr = (char *)*buf;
	shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + ehdr->e_shstrndx * sizeof(Elf64_Shdr)];
	str_offset = shdr->sh_offset;	
	size = sh_size / sizeof(Elf64_Sym);
	printf("\nSYMBOL TABLE:entrys = %d\n",size);
	printf("st_name\tst_info\tst_other\tst_shndx\tst_value\tst_size\n");
	for(i = 0;i < size;i++){
		sym = (Elf64_Sym *)&addr[sh_offset + i * sizeof(Elf64_Sym)];
	printf("[%d]\t%s\n",i,(char *)&addr[str_offset + sym->st_name]);	
	}
}
static void print_dsym(void **buf,Elf64_Ehdr *ehdr)
{

	Elf64_Shdr *shdr = NULL;
	char *addr = (char *)*buf;
	int i = 0,flag = 0;
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		if(SHT_DYNSYM == shdr->sh_type){
			dsym_info(ehdr,buf,shdr->sh_offset,shdr->sh_size);
		}

	}

}
static void sym_info(Elf64_Ehdr *ehdr,void **buf,Elf64_Off sh_offset,Elf64_Xword sh_size) //SHT_SYMTAB
{

	Elf64_Shdr *shdr = NULL;
	Elf64_Sym *sym = NULL;
	int i = 0,size = 0;
	Elf64_Off str_offset = 0;

	char *addr = (char *)*buf;
	shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + ehdr->e_shstrndx * sizeof(Elf64_Shdr)];
	str_offset = shdr->sh_offset;	
	size = sh_size / sizeof(Elf64_Sym);
	printf("\nSYMBOL TABLE:entrys = %d\n",size);
	printf("st_name\tst_info\tst_other\tst_shndx\tst_value\tst_size\n");
	for(i = 0;i < size;i++){
		sym = (Elf64_Sym *)&addr[sh_offset + i * sizeof(Elf64_Sym)];
	printf("[%d]\t%s\n",i,(char *)&addr[str_offset + sym->st_name]);	
	}
}
static void print_sym(void **buf,Elf64_Ehdr *ehdr)
{
	Elf64_Shdr *shdr = NULL;
	char *addr = (char *)*buf;
	int i = 0,flag = 0;
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		if(SHT_SYMTAB == shdr->sh_type){
			sym_info(ehdr,buf,shdr->sh_offset,shdr->sh_size);
		}

	}
}	
static void print_shdr(void **buf,Elf64_Ehdr *ehdr)
{
	Elf64_Shdr	*shdr = NULL;
	int i = 0,ret = 0;
	char type_name[64];
	char *addr = (char *)*buf;
	int str_offset = 0;
	
	printf("\t Section header table e_shnum = %d\n",ehdr->e_shnum);
	printf("\tname\ttype\t flags\tvirtual addr\tfile offset\t size\tlink\tinfo\talign\tentry size\n");
	shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + ehdr->e_shstrndx * sizeof(Elf64_Shdr)];
	str_offset = shdr->sh_offset;	
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		printf("[%d]  %-16s  %-8s  0x%08s  %016lx  %016lx  %08x  %08x  %16lx  %016lx\n",i,(char *)&addr[str_offset + shdr->sh_name],sh_type(shdr->sh_type),sh_flags(shdr->sh_flags),shdr->sh_offset,shdr->sh_size,shdr->sh_link,shdr->sh_info,shdr->sh_addralign,shdr->sh_entsize);	
	}

}

static void print_phdr(void **buf, Elf64_Ehdr *ehdr)
{
	Elf64_Phdr	*phdr = NULL;
	Elf64_Shdr	*shdr = NULL;
	char *addr = (char *)*buf;
	int ret = 0;
	int i = 0,j = 0;
	int str_offset = 0;

	shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + ehdr->e_shstrndx * sizeof(Elf64_Shdr)];
	str_offset = shdr->sh_offset;	
	printf("\n Program header table e_phnum = %x\n",ehdr->e_phnum);

	printf("index  type flags offset  virtual address   physical address   size in file  size in mem  align\n");
	
	for(i = 0;i < ehdr->e_phnum;i++){
		phdr = ((Elf64_Phdr *)(&(((char *)(*buf))[ehdr->e_phoff + i * sizeof(Elf64_Phdr)])));
		printf("[%d]  %08s  0x%08s  0x%016lx  0x%016lx  0x%016lx  0x%016lx  0x%016lx  0x%016lx\n",i,ph_type(phdr->p_type),ph_flags(phdr->p_flags),phdr->p_offset,phdr->p_vaddr,phdr->p_paddr,phdr->p_filesz,phdr->p_memsz,phdr->p_align);	
	}
	
	printf("\nSection to Segment mapping:\n");
	printf("Segment : Sections...\n");
	for(i = 0;i < ehdr->e_phnum;i++){
		printf("[%02d]\t",i);
		phdr = ((Elf64_Phdr *)(&(((char *)(*buf))[ehdr->e_phoff + i * sizeof(Elf64_Phdr)])));
		for(j = 0;j < ehdr->e_shnum;j++){
			shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + j * sizeof(Elf64_Shdr)];
			if((shdr->sh_offset >=  phdr->p_offset) && (shdr->sh_offset < (phdr->p_offset + phdr->p_filesz))){
				printf("%s,",(char *)&addr[str_offset + shdr->sh_name]);
			}
		}
		printf("\n");
	}	
}

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
	print_shdr(&buf,&ehdr);
	print_phdr(&buf,&ehdr);
	//print_sym(&buf,&ehdr);
	print_dsym(&buf,&ehdr);
	//printf("buf +size=%lx\n",buf + len);
	munmap(buf,len);
	close(fd);
	return 0;
}

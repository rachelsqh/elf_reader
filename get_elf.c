#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>

#if 0
static void print_chdr(int fd)
{
	Elf64_Chdr	chdr;
	lseek(fd,,SEEK_SET);	
	printf("\t compression format\tuncompressed data size\t uncompressed data alignment\n");
	printf("%08x\t%08x\t%016x\t%016x\n",chdr.ch_type,chdr.ch_reserved,chdr.ch_size,chdr.ch_addralign);
}

static void print_sym(int fd)
{
	Elf64_Sym	sym;
	lseek(fd,,SEEK_SET);	
	printf("\t name\ttype/binding\t visibility\tindex\tvalue\tsize\n");
	printf("%08x\t%04x\t%04x\t%08x\t%016lx\t%016lx\n",sym.st_name,sym.st_info,sym.st_other,sym.st_shndx,sym.st_value,sym.st_size);
}
static void print_syminfo()
{
	Elf64_Syminfo syminfo;
	printf("\tdirect bindings/symbol bound to\n");
	printf("%08x\t%08x\n",syminfo.si_boundto,syminfo.si_flags);


}

void print_rel()
{
	Elf64_Rel rel;

	printf("\t r_offset\tr_info\n");
	printf("%016lx\t%016lx\n",rel.r_offset,rel.r_info);
}


void print_rela()
{
	Elf64_Rela rela;

	printf("\t r_offset\tr_info\n");
	printf("%016lx\t%016lx\t%016x\n",rela.r_offset,rela.r_info,rela.r_addend);
}


#endif
#define sec_type(sh_type)(\
		(SHT_PROGBITS == sh_type)?\
			"progbits":(\
		(SHT_SYMTAB == sh_type) ?\
			"symtab":(\
		(SHT_STRTAB == sh_type) ?\
			"strtab":(\
	        (SHT_RELA == sh_type) ?\
			"rela":(\
	        (SHT_HASH == sh_type) ?\
			"hash":(\
	        (SHT_DYNAMIC == sh_type) ? \
			"dynamic":(\
	        (SHT_NOTE == sh_type) ? \
			"note":(\
	        (SHT_NOBITS == sh_type) ? \
			"nobits":(\
	        (SHT_REL == sh_type) ? \
			"rel": (\
	        (SHT_SHLIB == sh_type) ? \
			"shlib": (\
	        (SHT_DYNSYM == sh_type) ? \
			"dynsym":(\
	        (SHT_LOPROC == sh_type) ? \
			"loproc":(\
	        (SHT_HIPROC == sh_type) ? \
			"hiproc":(\
	        (SHT_LOUSER == sh_type) ? \
			"louser":(\
	        (SHT_HIUSER == sh_type) ? \
			"hiuser":"none")))))))))))))))


#define ph_type(ph_type)(\
		(PT_LOAD == ph_type)?\
			"load":(\
		(PT_DYNAMIC == ph_type) ?\
			"dynamic":(\
		(PT_INTERP == ph_type) ?\
			"interp":(\
	        (PT_NOTE == ph_type) ?\
			"note":(\
	        (PT_SHLIB == ph_type) ?\
			"shlib":(\
	        (PT_PHDR == ph_type) ? \
			"phdr":(\
	        (PT_TLS == ph_type) ? \
			"tls":(\
	        (PT_LOOS == ph_type) ? \
			"loop":(\
	        (PT_HIOS == ph_type) ? \
			"hios": (\
	        (PT_LOPROC == ph_type) ? \
			"loproc": (\
	        (PT_HIPROC == ph_type) ? \
			"hiproc":(\
	        (PT_GNU_EH_FRAME == ph_type) ? \
			"eh_frame":(\
	        (PT_GNU_STACK == ph_type) ? \
			"stack":"none")))))))))))))


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
		printf("[%d]\t%-8s\t%-8s\t0x%016lx\t%016lx\t%016lx\t%08x\t%08x\t%16lx\t%016lx\n",i,(char *)&addr[str_offset + shdr->sh_name],sec_type(shdr->sh_type),shdr->sh_flags,shdr->sh_offset,shdr->sh_size,shdr->sh_link,shdr->sh_info,shdr->sh_addralign,shdr->sh_entsize);	
	}

}


static void print_phdr(void **buf, Elf64_Ehdr *ehdr)
{
	Elf64_Phdr	*phdr = NULL;
	int ret = 0;
	int i = 0;

	printf("\t Program header table e_phnum = %x\n",ehdr->e_phnum);

	printf("index\ttype\toffset\tvirtual address\t physical address\t size in file\tsize in mem\talign\n");
	
	for(i = 0;i < ehdr->e_phnum;i++){
		phdr = ((Elf64_Phdr *)(&(((char *)(*buf))[ehdr->e_phoff + i * sizeof(Elf64_Phdr)])));
		printf("[%d]\t%08s\t0x%08x\t%016lx\t%016lx\t%016lx\t%016lx\t%016lx\t%016lx\n",i,ph_type(phdr->p_type),phdr->p_flags,phdr->p_offset,phdr->p_vaddr,phdr->p_paddr,phdr->p_filesz,phdr->p_memsz,phdr->p_align);	
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
	munmap(buf,len);
	close(fd);
	return 0;
}

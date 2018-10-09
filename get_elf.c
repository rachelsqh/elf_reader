#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

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


static void print_shdr(int fd,Elf64_Ehdr *ehdr)
{
	Elf64_Shdr	shdr;
	int i = 0,ret = 0;
	lseek(fd,ehdr->e_shoff,SEEK_SET);
	printf("\t Section header table e_shnum = %d\n",ehdr->e_shnum);
	printf("\tname\ttype\t flags\tvirtual addr\tfile offset\t size\tlink\tinfo\talign\tentry size\n");
		
	
	for(i = 0;i < ehdr->e_shnum;i++){
		ret = read(fd,&shdr,sizeof(Elf64_Shdr));
		if(sizeof(Elf64_Shdr) != ret){
			return;
		}
		printf("[%d]\t0x%08x\t0x%08x\t0x%016lx\t%016lx\t%016lx\t%08x\t%08x\t%16lx\t%016lx\n",i,shdr.sh_name,shdr.sh_type,shdr.sh_flags,shdr.sh_offset,shdr.sh_size,shdr.sh_link,shdr.sh_info,shdr.sh_addralign,shdr.sh_entsize);	
	}

}



static void print_phdr(int fd, Elf64_Ehdr *ehdr)
{
	Elf64_Phdr	phdr;
	int ret = 0;
	int i = 0;

	lseek(fd,ehdr->e_phoff,SEEK_SET);

	printf("\t Program header table e_phnum = %x\n",ehdr->e_phnum);

	printf("index\ttype\toffset\tvirtual address\t physical address\t size in file\tsize in mem\talign\n");
	for(i = 0;i < ehdr->e_phnum;i++){
		ret = read(fd,&phdr,sizeof(Elf64_Phdr));
		if(sizeof(Elf64_Phdr) != ret){
			return;
		}
		printf("[%d]\t0x%08x\t0x%08x\t%016lx\t%016lx\t%016lx\t%016lx\t%016lx\t%016lx\n",i,phdr.p_type,phdr.p_flags,
			phdr.p_offset,phdr.p_vaddr,phdr.p_paddr,phdr.p_filesz,phdr.p_memsz,phdr.p_align);	
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
	
	if(2 > argc){
		return -1;
	}

	fd = open(argv[1],O_RDONLY);
	if(0 > fd){	
		return -1;
	}

		
	ret = read(fd,&ehdr,sizeof(Elf64_Ehdr));
	if(sizeof(Elf64_Ehdr) != ret){
		return -2;
	}

			
	print_ehdr(&ehdr);
	print_phdr(fd,&ehdr);
	print_shdr(fd,&ehdr);
	close(fd);
	return 0;
}

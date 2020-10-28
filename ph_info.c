#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>

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
	printf("\n Program header table e_phnum = %d\n",ehdr->e_phnum);

	printf("TYPE\t      OFFSET\t     VIRTUAL-ADDRESS  \tPHYSICAL-ADDRESS\n      \t   SIZE-IN-FILE  \tSIZE-IN-MEM\t  FLAGS    ALIGN\n");
	
	for(i = 0;i < ehdr->e_phnum;i++){
		phdr = ((Elf64_Phdr *)(&(((char *)(*buf))[ehdr->e_phoff + i * sizeof(Elf64_Phdr)])));
		printf("%-08s  0x%016lx  0x%016lx  0x%016lx \n\t  0x%016lx  0x%016lx  %-08s 0x%04lx\n",ph_type(phdr->p_type),phdr->p_offset,phdr->p_vaddr,phdr->p_paddr,phdr->p_filesz,phdr->p_memsz,ph_flags(phdr->p_flags),phdr->p_align);
		if(PT_INTERP == phdr->p_type)
		      printf("\t\t%s\n",(char *)&addr[phdr->p_offset]);	
	
	}
	
	printf("\nSection to Segment mapping:\n");
	printf("Segment : Sections...\n");
	for(i = 0;i < ehdr->e_phnum;i++){
		printf("[%02d]\t",i);
		phdr = ((Elf64_Phdr *)(&(((char *)(*buf))[ehdr->e_phoff + i * sizeof(Elf64_Phdr)])));
		for(j = 0;j < ehdr->e_shnum;j++){
			shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + j * sizeof(Elf64_Shdr)];
			if((shdr->sh_offset >=  phdr->p_offset) && (shdr->sh_offset < (phdr->p_offset + phdr->p_filesz))){
				printf("%s ",(char *)&addr[str_offset + shdr->sh_name]);
			}
		}
		printf("\n");
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
			
	print_phdr(&buf,&ehdr);
	munmap(buf,len);
	close(fd);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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



struct Elf_note_data {
	Elf64_Nhdr	*nhdr;
	char		*name_data;
	char		*desc_data;
};
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
	
		if(SHT_NOTE == shdr->sh_type){
			struct Elf_note_data note;
			note.nhdr = (Elf64_Nhdr *)&addr[shdr->sh_offset];
			note.name_data = (char *)note.nhdr + sizeof(Elf64_Nhdr);
			note.desc_data = note.name_data + note.nhdr->n_namesz;
			printf("\t\tn_namesz   :n_descsz:  n_type\n");
			printf("\t\t0x%-8lx    0x%-08lx    0x%-08lx\n",note.nhdr->n_namesz,note.nhdr->n_descsz,note.nhdr->n_type);
			printf("\t\tname:%s desc:%x\n",note.name_data,note.desc_data);

		}	
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

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
		((SHF_ALLOC | SHF_EXECINSTR ) == sh_flags) ? "AE" : "UNKNOWN"))))))

static char type[32];
#define sym_bind(type) (\
	(STB_LOCAL == type) ? "LOCAL" : (\
	(STB_GLOBAL == type) ? "GLOBAL" : (\
	(STB_WEAK == type) ? "WEAK" : "UNKNOWN")))
#define sym_type(type) (\
	(STT_NOTYPE == type) ? "NOTYPE" : (\
	(STT_OBJECT == type) ? "OBJECT" : (\
	(STT_FUNC   == type) ? "FUNC"   : (\
	(STT_SECTION== type) ? "SECTION": (\
	(STT_FILE   == type) ? "FILE"   : (\
	(STT_COMMON == type) ? "COMMON" : (\
	(STT_TLS    == type) ? "TLS" : "UNKNOWN")))))))

	//(SHN_LIVEPATCH == index) ? "LIVEPATCH" : (
#define sym_sindex(index) (\
	(SHN_UNDEF == index) ? "UNDEF" : (\
	(SHN_LORESERVE == index) ? "LORESERVE" : (\
	(SHN_LOPROC == index) ? "LOPROC" : (\
	(SHN_HIPROC == index) ? "HIPROC" : (\
	(SHN_ABS       == index) ? "ABS" : (\
	(SHN_COMMON    == index) ? "COMMON" : (\
	(SHN_HIRESERVE == index) ? "HIRESERVE" : "UNKNOWN")))))))


static void print_shdr(void **buf,Elf64_Ehdr *ehdr)
{
	Elf64_Shdr	*shdr = NULL;
	Elf64_Sym	*sym = NULL;
	int i = 0,j = 0,ret = 0,dynsym_count = 0;
	char type_name[64];
	char *addr = (char *)*buf;
	int str_offset = 0,dym_str_offset = 0;
	char *str = NULL;
	
	shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + ehdr->e_shstrndx * sizeof(Elf64_Shdr)];
	str_offset = shdr->sh_offset;	
	
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		if(SHT_STRTAB == shdr->sh_type){
			if(!strcmp((char *)&addr[str_offset + shdr->sh_name],".dynstr")){
				dym_str_offset = shdr->sh_offset;
				break;
			}
		}
	}
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		
		if(SHT_DYNSYM == shdr->sh_type){
			dynsym_count = shdr->sh_size / sizeof(Elf64_Sym);
			printf("elf64_sym len = %d\n",sizeof(Elf64_Sym));
			printf("dynsym_count = %d\n",dynsym_count);
			printf("SHT_DYNSYM section name:%s\n",(char *)&addr[shdr->sh_name + str_offset]);
			
			printf("COUNT\tVALUTE\tSIZE\tTYPE\tBIND\tSECTION-INDEX\n");
			for(j = 0;j < dynsym_count;j++){
			sym = (Elf64_Sym *)&addr[shdr->sh_offset + j * sizeof(Elf64_Sym)];
			printf("%02d  0x%016x  %4d  %6s  %6s  %6s  %s\n",j,sym->st_value,sym->st_size,sym_type(ELF64_ST_TYPE(sym->st_info)),sym_bind(ELF64_ST_BIND(sym->st_info)),sym_sindex(sym->st_shndx),(char *)&addr[dym_str_offset + sym->st_name]);

			}
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

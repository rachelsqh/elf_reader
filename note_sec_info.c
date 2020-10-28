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
#define desc_freebsd "UNKNOWN"
#define desc_netbsd "UNKNOWN"
static char	desc_str[128]; 
struct Elf_gnu_abi_tag {
	Elf64_Word	os;
	Elf64_Word	major;
	Elf64_Word	minor;
	Elf64_Word	subminor;
};
#define desc_gnu(type,desc,desc_len)  (\
		( NT_GNU_BUILD_ID == type) ?({\
			int i = 0;\
			sprintf(desc_str,"Build ID:");\
			for(i = 0;i < desc_len;i++){\
			    sprintf(desc_str,"%s%02x",desc[i]);\
		    	}\
			sprintf(desc_str,"\n");\
	    		desc_str;\
		}) : (\
		(NT_GNU_ABI_TAG == type) ? ({\
		        int i = 0;\
			if(desc_len < 16){\
		        	sprintf(desc_str," <corrupt GNU_ABI_TAB>\n");\
		        	desc_str;\
			}else{\
				struct Elf_gnu_abi_tag *tag;\
				tag = (struct Elf_gnu_abi_tag *)desc;\
				sprintf(desc_str,"OS: "); \
				(GNU_ABI_TAG_LINUX == tag->os) ? ({\
					sprintf(desc_str,"%s%s",desc_str,"Linux");\
				}) : ({\
				 (GNU_ABI_TAG_HURD == tag->os) ? ({\
					sprintf(desc_str,"%s%s",desc_str,"Hurd");\
				}) : ({ \
				 (GNU_ABI_TAG_SOLARIS == tag->os) ? ({\
					sprintf(desc_str,"%s%s",desc_str,"Solaris");\
				}) : ({\
				 (GNU_ABI_TAG_FREEBSD == tag->os) ? ({\
					sprintf(desc_str,"%s%s",desc_str,"FreeBSD");\
				}) : ({\
				 (GNU_ABI_TAG_NETBSD == tag->os) ? ({\
					sprintf(desc_str,"%s%s",desc_str,"NetBSD");\
				}) : ({\
				 (GNU_ABI_TAG_SYLLABLE == tag->os) ? ({\
				  	sprintf(desc_str,"%s%s",desc_str,"Syllable");\
				}) : ({\
				 (GNU_ABI_TAG_NACL == tag->os) ? ({\
					sprintf(desc_str,"%s%s",desc_str,"NaCl");\
				}) : ({\
					sprintf(desc_str,"%s%s",desc_str,"UnKnown");})))) \
				sprintf(desc_str,"%s,ABI: %ld.%ld.%ld\n",desc_str,tag->os,tag->major,tag->minor,tag->subminor);\
				: ({\
			(NT_GNU_GOLD_VERSION == type) ? ({\
				int i = 0;\
				sprintf(desc_str,"Version: ");\
				for(i = 0;i < desc_len;i++){\
					sprintf(desc_str,"%s%c",desc_str,desc[i]);\
				}\
				sprintf(desc_str,"%s\n",desc_str);\
			}) : ({\
			 (NT_GNU_HWCAP == type) ? ({\
				sprintf(desc_str," Hardware Capabilities :");\
				if(desc_len < 8)\
				   sprintf(desc_str,"%s <corrupt GNU_HWCAP> \n",desc_str); \
			  	else { \
				    


				
			    
		        }) : (\
		(== type) ?  : (\
		( == type) ?  : "UNKNOWN"))))

#define print_note_desc(type,name,desc_len,desc) (\
		(!strcmp(name,"GNU")) ? desc_gnu(type,desc,desc_len) : (\
		(!strcmp(name,"FreeBSD")) ? desc_freebsd(type,desc) : (\
		(!strcmp(name,"NetBsd")) ? desc_netbsd(type,desc) : "UNKNOW")))

static void print_shdr(void **buf,Elf64_Ehdr *ehdr)
{
	Elf64_Shdr	*shdr = NULL;
	int i = 0,ret = 0;
	char type_name[64];
	char *addr = (char *)*buf;
	int str_offset = 0;
	char *str = NULL;
	
	shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + ehdr->e_shstrndx * sizeof(Elf64_Shdr)];
	str_offset = shdr->sh_offset;	
	for(i = 0;i < ehdr->e_shnum;i++){
		shdr = (Elf64_Shdr *)&addr[ehdr->e_shoff + i * sizeof(Elf64_Shdr)];
		if(SHT_NOTE == shdr->sh_type){
			struct Elf_note_data note;
			note.nhdr = (Elf64_Nhdr *)&addr[shdr->sh_offset];
			note.name_data = (char *)note.nhdr + sizeof(Elf64_Nhdr);
			note.desc_data = note.name_data + note.nhdr->n_namesz;
			printf("\tname  n_namesz\tn_descsz\tn_type\tdesc\n");
			printf("\t%s\t0x%-8lx    0x%-08lx    0x%-08lx\t %s\n",note.name_data,note.nhdr->n_namesz,note.nhdr->n_descsz,note.nhdr->n_type,note.desc_data,print_note_desc(note.name_data,note.desc_data));

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

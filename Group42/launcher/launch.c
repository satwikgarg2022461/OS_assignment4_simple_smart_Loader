#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;


// the following has been taken from this website: https://wiki.osdev.org/ELF_Tutorial
int elf_check_file(Elf32_Ehdr *ehdr) {
	if(!ehdr) return 0;
	if(ehdr->e_ident[EI_MAG0] != ELFMAG0) {
		perror("ELF Header EI_MAG0 incorrect.\n");
		return 0;
	}
	if(ehdr->e_ident[EI_MAG1] != ELFMAG1) {
		perror("ELF Header EI_MAG1 incorrect.\n");
		return 0;
	}
	if(ehdr->e_ident[EI_MAG2] != ELFMAG2) {
		perror("ELF Header EI_MAG2 incorrect.\n");
		return 0;
	}
	if(ehdr->e_ident[EI_MAG3] != ELFMAG3) {
		perror("ELF Header EI_MAG3 incorrect.\n");
		return 0;
	}
	return 1;
}

int elf_check_supported(Elf32_Ehdr *ehdr) {
	if(!elf_check_file(ehdr)) {
		perror("Invalid ELF File.\n");
		return 0;
	}
	if(ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
		perror("Unsupported ELF File Class.\n");
		return 0;
	}
	if(ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
		perror("Unsupported ELF File byte order.\n");
		return 0;
	}
	if(ehdr->e_machine != EM_386) {
		perror("Unsupported ELF File target.\n");
		return 0;
	}
	if(ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
		perror("Unsupported ELF File version.\n");
		return 0;
	}
	if(ehdr->e_type != ET_REL && ehdr->e_type != ET_EXEC) {
		perror("Unsupported ELF File type.\n");
		return 0;
	}
	return 1;
}
// till here----------------------------------------------------------------------------------
int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  fd = open(argv[1], O_RDONLY);
  off_t f_size = lseek(fd, 0, SEEK_END);
  lseek(fd,0,SEEK_SET);
  char *memory = (char*)malloc( f_size);
  ssize_t readData = read(fd,memory, f_size);
  close(fd);
  ehdr=(Elf32_Ehdr*)memory;
  if (elf_check_supported(ehdr)==0){
    exit(1);
  }
  
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv);
  // 3. invoke the cleanup routine inside the loader  
  // loader_cleanup();
  free(memory);
  return 0;
}

#include "loader.h"


typedef int(*startfunction)(void);
Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
Elf32_Shdr *shdr;
Elf32_Phdr *p;
Elf32_Phdr* a;
startfunction _start;
void* virtual_mem;
int fd;
void * s;


/*
 * release memory and other cleanups
 */
void loader_cleanup(char *mem,void *virtual_mem,Elf32_Phdr *p) {
  free(mem);
  munmap(virtual_mem,p->p_memsz);
  
}

// void *virtual_mem_allocator(Elf32_Phdr *p){
//    // 3. Allocate memory of the size "p_memsz" using mmap function 
//   //    and then copy the segment content
//   void *x=mmap(NULL, p->p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE,0,0);
//   return x;
// }

void mem_alloc_checker(int f,ssize_t re_rtn){
  if (re_rtn==-1){
    printf("Error in allocating memory");
    close(f);
    exit(1);
  }
}



void segfault_handler(int signo) {
    printf("Segmentation fault detected.\n");
    void *fault_address = (void *)(((unsigned long)(&signo)) - sizeof(void *));
    virtual_mem = mmap(NULL, a->p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE,0,0);
    s=(void*)((uintptr_t)ehdr+a->p_offset);
    memcpy(virtual_mem, s, a->p_filesz);
    printf("hi\n");
    printf("p signal %x\n",a->p_vaddr);
    _start=(startfunction)(virtual_mem+(ehdr->e_entry - a->p_vaddr));
    printf("_start %p\n", _start);
    int result = _start();
    printf("User _start return value = %d\n",result);
    
    
    exit(1);
}




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
// --------------------------------------till here---------------------------------------------------------------


void load_and_run_elf(char** exe) {
  signal(SIGSEGV, segfault_handler);


  // 1. Load entire binary content into the memory from the ELF file.
  fd = open(exe[1], O_RDONLY);
  off_t f_size = lseek(fd, 0, SEEK_END);
  lseek(fd,0,SEEK_SET);
  char *memory = (char*)malloc( f_size);
  ssize_t readData = read(fd,memory, f_size);
  mem_alloc_checker(fd,readData);
  close(fd);




// 2. Iterate through the PHDR table and find the section of PT_LOAD 
//    type that contains the address of the entrypoint method in fib.c
  ehdr=(Elf32_Ehdr*)memory;
  phdr=(Elf32_Phdr*)(ehdr+(ehdr->e_phoff)/(sizeof(Elf32_Ehdr)));
  p=phdr;
  unsigned int entry_point=ehdr->e_entry;
  // printf("entry point %x\n",entry_point);

  int size;
  
  int v_add;
  for(int i=0;i<ehdr->e_phnum;i++)
  {
    p=phdr+i*(ehdr->e_phentsize)/sizeof(Elf32_Phdr);
    if(p->p_type==PT_LOAD)
    {
      Elf32_Phdr* p2=p+i*(ehdr->e_phentsize)/sizeof(Elf32_Phdr);
      if(p->p_vaddr <= entry_point && entry_point<= p2->p_vaddr)
      {
        printf("p pre %x\n",p->p_vaddr);
        // virtual_mem=virtual_mem_allocator(p);
        // // virtual_mem=mmap(NULL, p->p_memsz, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE,0,0);
        // s=(void*)((uintptr_t)ehdr+p->p_offset);
        // memcpy(virtual_mem,s,p->p_filesz);
        a=p;
      }
    }
  }
  printf("a post %x\n",a->p_vaddr);
  printf("_start %p\n",(void*) _start);
  int result = _start();
  printf("User _start return value = %d\n",result);
  printf("p after start %x\n",p->p_vaddr);
  loader_cleanup(memory,virtual_mem,p);
}

#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
void *virtual_mem;
int fd;
void *s;
int no_of_page_faults = 0;
int total_no_of_pages = 0;
double total_internal_fragmentation = 0;

void** fault_address;
int fault_counter = 0; 



void loader_cleanup(char *mem, int fd)
{
  close(fd);
  free(mem);
  // munmap(virtual_mem, p->p_memsz);
  size_t size = 4000;
  for(int i=0; i < fault_counter; i++)
  {
    void * fault = fault_address[i];
    if (munmap(fault,size) == -1){
      perror("munmap error\n");
      exit(1);
    }
  }
  free(fault_address);
}


void sigsegv_handler(int signo, siginfo_t *info, void *context)
{
  if(signo != SIGSEGV)
  {
    perror("error\n");
    exit(1);
  }

  printf("Segmentation fault address = %p\n", info->si_addr);
  
  
  
  
  no_of_page_faults++;

  Elf32_Phdr *p;
  int page_size = 4096;
  int no_of_pages = 1;
  int offset = 0;

  for (int i = 0; i < ehdr->e_phnum; i++)
  {
    p = phdr + i * (ehdr->e_phentsize) / sizeof(Elf32_Phdr);
    if (p->p_vaddr <= (int)info->si_addr && (int)info->si_addr <= p->p_vaddr + p->p_memsz)
    {
      while(p->p_vaddr+offset+4096<=(int)info->si_addr)
      {
        offset+=4096;
      }

      printf("Starting virtual address of required segment = %x\n", p->p_vaddr);
      printf("Size of segment = %d\n", p->p_memsz );

      
      
      total_no_of_pages += no_of_pages;
      double internal_fragmentation =0;

      if ((int)(page_size-(p->p_memsz-offset))>0)
      {
        internal_fragmentation=page_size-(p->p_memsz-offset);
      }

      total_internal_fragmentation += internal_fragmentation;

      printf("segment left to be loaded = %d\n",p->p_memsz - offset);
      printf("No of pages allocated = %d\n", no_of_pages);
      printf("Page size alloacted for segment = %d\n", page_size);
      printf("Internal fragmentation = %f KB\n", internal_fragmentation/1024);
      printf("..........................................................\n");
      // ---mmap
      virtual_mem = mmap(info->si_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
      // printf("fault %p\n",info->si_addr);
      // printf("virtual mem %p\n",virtual_mem);
      if(virtual_mem == MAP_FAILED)
      {
        perror("mmap error\n");
        exit(1);
      }
      fault_address[fault_counter] = virtual_mem;
      fault_counter++;

      s = (void *)((uintptr_t)ehdr + p->p_offset+offset);
      // ---memcopy
      void * cpyerror=memcpy(virtual_mem, s, page_size);
      if (cpyerror==NULL){
        perror("memcpy error\n");
        exit(1);
      }
    }

  }
}


// the following has been taken from this website: https://wiki.osdev.org/ELF_Tutorial
int elf_check_file(Elf32_Ehdr *ehdr)
{
  if (!ehdr)
    return 0;
  if (ehdr->e_ident[EI_MAG0] != ELFMAG0)
  {
    perror("ELF Header EI_MAG0 incorrect.\n");
    return 0;
  }
  if (ehdr->e_ident[EI_MAG1] != ELFMAG1)
  {
    perror("ELF Header EI_MAG1 incorrect.\n");
    return 0;
  }
  if (ehdr->e_ident[EI_MAG2] != ELFMAG2)
  {
    perror("ELF Header EI_MAG2 incorrect.\n");
    return 0;
  }
  if (ehdr->e_ident[EI_MAG3] != ELFMAG3)
  {
    perror("ELF Header EI_MAG3 incorrect.\n");
    return 0;
  }
  return 1;
}

int elf_check_supported(Elf32_Ehdr *ehdr)
{
  if (!elf_check_file(ehdr))
  {
    perror("Invalid ELF File.\n");
    return 0;
  }
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS32)
  {
    perror("Unsupported ELF File Class.\n");
    return 0;
  }
  if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
  {
    perror("Unsupported ELF File byte order.\n");
    return 0;
  }
  if (ehdr->e_machine != EM_386)
  {
    perror("Unsupported ELF File target.\n");
    return 0;
  }
  if (ehdr->e_ident[EI_VERSION] != EV_CURRENT)
  {
    perror("Unsupported ELF File version.\n");
    return 0;
  }
  if (ehdr->e_type != ET_REL && ehdr->e_type != ET_EXEC)
  {
    perror("Unsupported ELF File type.\n");
    return 0;
  }
  return 1;
}
// --------------------------------------till here---------------------------------------------------------------


void load_and_run_elf(char **exe)
{
  fault_address = malloc(sizeof(void*));
  if(fault_address == NULL)
  {
    perror("malloc\n");
    free(fault_address);
    exit(1);
  }
  // ---signal handler
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigsegv_handler;
  int check_signal = sigaction(SIGSEGV, &sa, NULL);
  if(check_signal == -1)
  {
    perror("sigaction\n");
    exit(1);
  }

// ---reading the elf file
  fd = open(exe[1], O_RDONLY);
  if(fd == -1)
  {
    perror("open the file\n");
    close(fd);
    exit(1);
  }

  off_t f_size = lseek(fd, 0, SEEK_END);
  if(f_size == -1)
  {
    perror("lseek\n");
    close(fd);
    exit(1);
  }

  off_t check_lseek = lseek(fd, 0, SEEK_SET);
  if(check_lseek == -1)
  {
    perror("lseek \n");
    close(fd);
    exit(1);
  }

  char *memory = (char *)malloc(f_size);
  if(memory == NULL)
  {
    perror("malloc\n");
    free(memory);
    exit(1);
  }

  ssize_t readData = read(fd, memory, f_size);
  if(readData == -1)
  {
    perror("read");
    close(fd);
    exit(1);
  }

  // // ---closing fd
  // close(fd);
    
  ehdr = (Elf32_Ehdr *)memory;
  elf_check_file(ehdr);
  elf_check_supported(ehdr);
  phdr = (Elf32_Phdr *)(ehdr + (ehdr->e_phoff) / (sizeof(Elf32_Ehdr)));

// ----typecasting the entry address
  typedef int (*startfunction)(void);
  startfunction _start = (startfunction)ehdr->e_entry;
  int result = _start();

  // ---printing final result
  printf("User _start return value = %d\n", result);
  printf("Total Page Faults = %d\n", no_of_page_faults);
  printf("Total Page Allocations = %d\n", total_no_of_pages);
  printf("Total internal fragmentation = %f KB\n", (total_internal_fragmentation)/1024);

 
// --clean up
  loader_cleanup(memory,fd);
}
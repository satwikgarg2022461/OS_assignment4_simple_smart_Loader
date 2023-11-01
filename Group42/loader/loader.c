#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
Elf32_Shdr *shdr;
int fd;
void *s;
int no_of_page_faults = 0;
int total_no_of_pages = 0;
double total_internal_fragmentation = 0;
int remaining_segment_size = 0;

/*
 * release memory and other cleanups
 */
void loader_cleanup(char *mem, void *virtual_mem, Elf32_Phdr *p)
{
  free(mem);
  munmap(virtual_mem, p->p_memsz);
}

void sigsegv_handler(int signo, siginfo_t *info, void *context)
{
  printf("Segmentation fault address = %p\n", info->si_addr);
  no_of_page_faults++;

  Elf32_Phdr *p;
  void *virtual_mem;
  int size;
  Elf32_Phdr *a;
  int v_add;
  unsigned int entry_point = ehdr->e_entry;
  for (int i = 0; i < ehdr->e_phnum; i++)
  {
    p = phdr + i * (ehdr->e_phentsize) / sizeof(Elf32_Phdr);
    // printf("p outside %x\n",p->p_vaddr);
    if (p->p_vaddr <= (int)info->si_addr && (int)info->si_addr <= p->p_vaddr + p->p_memsz)
    {
      printf("Starting virtual address of required segment = %x\n", p->p_vaddr);
      printf("Size of segment = %d\n", (p->p_memsz - remaining_segment_size));
      int page_size = 4096;
      int no_of_pages = 1;
      // while (1)
      // {
      //   if (page_size < p->p_memsz)
      //   {
      //     no_of_pages++;
      //     page_size = no_of_pages * 4096;
      //   }
      //   else
      //   {
      //     break;
      //   }
      // }
      total_no_of_pages += no_of_pages;
      double internal_fragmentation = 0;
      int si = p->p_memsz - remaining_segment_size; 
      if((page_size - si) >= 0)
      {
        internal_fragmentation = page_size - si;
      }
      else
      {
        internal_fragmentation = 0;
        remaining_segment_size = 4096;
      }
      // int internal_fragmentation = page_size - p->p_memsz;
      total_internal_fragmentation += internal_fragmentation;
      printf("No of pages allocated = %d\n", no_of_pages);
      printf("Page size alloacted for segment = %d\n", page_size);
      printf("Internal fragmentation = %f\n", (internal_fragmentation/4096));
      printf("..........................................................\n");
      virtual_mem = mmap(info->si_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
      s = (void *)((uintptr_t)ehdr + p->p_offset);
      memcpy(virtual_mem, s, page_size);
    }

  }
}

// exit(signo);

void *virtual_mem_allocator(Elf32_Phdr *p)
{
  // 3. Allocate memory of the size "p_memsz" using mmap function
  //    and then copy the segment content
  void *x = mmap(NULL, p->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
  return x;
}

void mem_alloc_checker(int f, ssize_t re_rtn)
{
  if (re_rtn == -1)
  {
    printf("Error in allocating memory");
    close(f);
    exit(1);
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
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigsegv_handler;
  sigaction(SIGSEGV, &sa, NULL);
  // signal(SIGSEGV,sigsegv_handler);

  // 1. Load entire binary content into the memory from the ELF file.
  fd = open(exe[1], O_RDONLY);
  off_t f_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *memory = (char *)malloc(f_size);
  ssize_t readData = read(fd, memory, f_size);
  mem_alloc_checker(fd, readData);
  close(fd);

  // 2. Iterate through the PHDR table and find the section of PT_LOAD
  //    type that contains the address of the entrypoint method in fib.c
  ehdr = (Elf32_Ehdr *)memory;
  phdr = (Elf32_Phdr *)(ehdr + (ehdr->e_phoff) / (sizeof(Elf32_Ehdr)));
  Elf32_Phdr *p = phdr;

  // printf("entry point %x\n",entry_point);

  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  typedef int (*startfunction)(void);
  startfunction _start = (startfunction)ehdr->e_entry;
  // printf("_start %p\n",_start);

  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n", result);
  printf("Total Page Faults = %d\n", no_of_page_faults);
  printf("Total Page Allocations = %d\n", total_no_of_pages);
  printf("Total internal fragmentation = %f\n", (total_internal_fragmentation/4096));

  // loader_cleanup(memory,virtual_mem,p);
}
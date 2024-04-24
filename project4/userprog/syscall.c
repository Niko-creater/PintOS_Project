#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "process.h"
#include "pagedir.h"
#include <threads/vaddr.h>
#include <filesys/filesys.h>
#include <devices/shutdown.h>
#include <filesys/file.h>
#include <devices/input.h>
#include <threads/malloc.h>
#include <threads/palloc.h>
#define max_syscall 20

void sys_exit(struct intr_frame *f);  /* syscall exit. */
void sys_write(struct intr_frame *f); /* syscall write */

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Project 4 System call handler */

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  int type = *(int *)f->esp; // Read the system call number
  if (type <= 0 || type >= max_syscall)
  {
    thread_exit();
  }

  switch (type)
  {
  // If the system call number is SYS_EXIT, do sys_exit
  case SYS_EXIT: 
    sys_exit(f);
    break;

  // If the system call number is SYS_WRITE, do sys_write
  case SYS_WRITE:
    sys_write(f);
    break;
  
  default:
    break;
  }
}

void sys_exit(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  user_ptr++; // Skip the system call number          
  thread_current()->exit_status = *user_ptr; // Read the exit status
  thread_exit();
}

void sys_write(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  user_ptr++; // Skip the system call number
  const char *buffer = (const char *)*(user_ptr + 1); // Read the buffer from second arg
  off_t size = *(user_ptr + 2); // Read the size from third arg

  putbuf(buffer, size); // print to stdout
  f->eax = size; // return number written
}

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

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  int type = *(int *)f->esp; 
  if (type <= 0 || type >= max_syscall)
  {
    thread_exit();
  }

  switch (type)
  {
  case SYS_EXIT:
    sys_exit(f);
    break;

  case SYS_WRITE:
    sys_write(f);
    break;
  
  default:
    break;
  }
}

/* Do sytem exit */
void sys_exit(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  user_ptr++;              
  /* record the exit status of the process */
  thread_current()->st_exit = *user_ptr;
  thread_exit();
}

/* Do system write, Do writing in stdout and write in files */
void sys_write(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  user_ptr++;
  const char *buffer = (const char *)*(user_ptr + 1);
  off_t size = *(user_ptr + 2);

  putbuf(buffer, size);
  f->eax = size; // return number written
}

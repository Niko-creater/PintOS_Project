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
void sys_exec(struct intr_frame *f);  /* syscall exit. */
void sys_wait(struct intr_frame *f);  /* syscall write */

static void check_ptr(const void *vaddr);
static void exit_special (void);

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Project 5 Memory Access */

void exit_special(void)
{
  thread_current()->exit_status = -1;
  thread_exit();
}

void check_ptr(const void *vaddr)
{
  /* Judge address */
  if (!is_user_vaddr(vaddr)) 
  {
    exit_special();
  }
  /* Judge the page */
  void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr); 
  if (!ptr)
  {
    exit_special();
  }

}

/* Project 4 System call handler */

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  int type = *(int *)f->esp; // Read the system call number
  if (type <= 0 || type >= max_syscall)
  {
    exit_special ();
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
  /* Project5 wait() */
  // If the system call number is SYS_WAIT, do sys_wait
  case SYS_WAIT:
    sys_wait(f);
    break;
  /* Project5 exec() */
  // If the system call number is SYS_EXEC, do sys_exec
  case SYS_EXEC:
    sys_exec(f);
    break;

  default:
    break;
  }
}

void sys_exit(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr (user_ptr + 1);
  user_ptr++;                                // Skip the system call number
  thread_current()->exit_status = *user_ptr; // Read the exit status
  thread_exit();
}

void sys_write(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  user_ptr++;                                         // Skip the system call number
  const char *buffer = (const char *)*(user_ptr + 1); // Read the buffer from second arg
  off_t size = *(user_ptr + 2);                       // Read the size from third arg

  putbuf(buffer, size); // print to stdout
  f->eax = size;        // return number written
}
/* Project5 exec() */
/* Do sytem exec */
void sys_exec(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr(user_ptr + 1);    // Check the address of the first parameter
  check_ptr(*(user_ptr + 1)); // Check the value of the first argument, the address pointed to by const char *file
  *user_ptr++;
  f->eax = process_execute((char *)*user_ptr); // Do process execute
}
/* Project5 wait() */
/* Do sytem wait */
void sys_wait(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr(user_ptr + 1); // Check the address of the first parameter
  *user_ptr++;
  f->eax = process_wait(*user_ptr); // Do process wait
}

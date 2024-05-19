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

struct thread_file *find_file_id(int fd);
void sys_create(struct intr_frame *f);   /* syscall create */
void sys_remove(struct intr_frame *f);   /* syscall remove */
void sys_open(struct intr_frame *f);     /* syscall open */
void sys_filesize(struct intr_frame *f); /* syscall filesize */
void sys_read(struct intr_frame *f);     /* syscall read */
void sys_seek(struct intr_frame *f);     /* syscall seek */
void sys_tell(struct intr_frame *f);     /* syscall tell */
void sys_close(struct intr_frame *f);    /* syscall close */

static void check_ptr(const void *vaddr);
static void exit_special(void);

static void syscall_handler(struct intr_frame *);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* Project 5 Memory Access */

// Handle the exit with error
void exit_special(void)
{
  thread_current()->exit_status = -1;
  thread_exit();
}

// Check if the user pointer is valid
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
    exit_special();
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
  /* Project6 */
  case SYS_HALT:
    sys_halt(f);
    break;
  case SYS_OPEN:
    sys_open(f);
    break;
  case SYS_CREATE:
    sys_create(f);
    break;
  case SYS_FILESIZE:
    sys_filesize(f);
    break;
  case SYS_READ:
    sys_read(f);
    break;
  case SYS_REMOVE:
    sys_remove(f);
    break;
  case SYS_SEEK:
    sys_seek(f);
    break;
  case SYS_CLOSE:
    sys_close(f);
    break;
  case SYS_TELL:
    sys_tell(f);
    break;

  default:
    break;
  }
}

void sys_exit(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr(user_ptr + 1);
  user_ptr++;                                // Skip the system call number
  thread_current()->exit_status = *user_ptr; // Read the exit status
  thread_exit();
}

void sys_write(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr ((user_ptr + 1));
  check_ptr (*(user_ptr + 2));
  user_ptr++; // Skip the system call number
  int fd = *user_ptr;
  const char *buffer = (const char *)*(user_ptr + 1); // Read the buffer from second arg
  off_t size = *(user_ptr + 2);                       // Read the size from third arg
  if (fd == 1)
  {
    putbuf(buffer, size); // print to stdout
    f->eax = size;        // return number written
  }
  else
  {
    /* Project6 */
    /* Write to Files */
    struct thread_file *thread_file_temp = find_file_id(*user_ptr);
    if (thread_file_temp)
    {
      acquire_lock_f(); // file operating needs lock
      f->eax = file_write(thread_file_temp->file, buffer, size);
      release_lock_f();
    }
    else
    {
      f->eax = 0; // can't write,return 0
    }
  }
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
  user_ptr++;
  f->eax = process_wait(*user_ptr); // Do process wait
}

/* Project6 */

/* Find file by the file's fd */
struct thread_file *
find_file_id(int file_fd)
{
  struct list_elem *e;
  struct thread_file *thread_file_temp = NULL;
  struct list *files = &thread_current()->files;
  for (e = list_begin(files); e != list_end(files); e = list_next(e))
  {
    thread_file_temp = list_entry(e, struct thread_file, file_elem);
    if (file_fd == thread_file_temp->fd)
      return thread_file_temp;
  }
  return NULL;
}

/* Do sytem halt */
void sys_halt(struct intr_frame *f)
{
  shutdown_power_off();
}

/* Do sytem open, by calling the method filesys_open */
void sys_open(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr(user_ptr + 1); // arg address
  check_ptr(*(user_ptr + 1)); // file name address
  user_ptr++;
  acquire_lock_f();
  struct file *file_opened = filesys_open((const char *)*user_ptr);
  release_lock_f();
  struct thread *t = thread_current();
  if (file_opened)
  {
    struct thread_file *thread_file_temp = malloc(sizeof(struct thread_file)); // Create thread_file
    thread_file_temp->fd = t->max_file_fd++; // Generate fd
    thread_file_temp->file = file_opened;
    list_push_back(&t->files, &thread_file_temp->file_elem); // Add opened file to list
    f->eax = thread_file_temp->fd; // Return fd
  }
  else // the file could not be opened
  {
    f->eax = -1;
  }
}

/* Do sytem filesize, by calling the method file_length */
void sys_filesize(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr(user_ptr + 1); // pd address
  user_ptr++; 
  struct thread_file *thread_file_temp = find_file_id(*user_ptr);
  if (thread_file_temp)
  {
    acquire_lock_f();
    f->eax = file_length(thread_file_temp->file); // return the size in bytes
    release_lock_f();
  }
  else
  {
    f->eax = -1;
  }
}

/* Do system create, by calling the method filesys_create */
void sys_create(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr (user_ptr); // arg address
  check_ptr (*(user_ptr + 1)); // file name address
  user_ptr++;
  acquire_lock_f ();
  f->eax = filesys_create ((const char *)*user_ptr, *(user_ptr+1));
  release_lock_f ();
}

/* Do system remove, by calling the method filesys_remove */
void sys_remove(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr(user_ptr + 1);    // arg address
  check_ptr(*(user_ptr + 1)); // file name address
  user_ptr++;
  acquire_lock_f();
  f->eax = filesys_remove((const char *)*user_ptr);
  release_lock_f();
}

/* Do system read, by calling the function file_tell() in filesystem */
void 
sys_read (struct intr_frame* f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr(user_ptr+1);
  check_ptr(user_ptr+2);
  check_ptr(*(user_ptr+2));
  check_ptr(user_ptr+3);
  user_ptr++;
  int fd = *user_ptr;
  uint8_t * buffer = (uint8_t*)*(user_ptr+1);
  off_t size = *(user_ptr+2);
  /* get the files buffer */
  if (fd == 0) //stdin
  {
    int i;
    for (i = 0; i < size; i++)
      buffer[i] = input_getc();
    f->eax = size;
  }
  else
  {
    struct thread_file * thread_file_temp = find_file_id (*user_ptr);
    if (thread_file_temp)
    {
      acquire_lock_f ();
      f->eax = file_read (thread_file_temp->file, buffer, size);
      release_lock_f ();
    } 
    else//can't read
    {
      f->eax = -1;
    }
  }
}

/* Do system seek, by calling the function file_seek() in filesystem */
void 
sys_seek(struct intr_frame* f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr (user_ptr + 1);
  check_ptr (user_ptr + 2);
  user_ptr++; //fd
  struct thread_file *file_temp = find_file_id (*user_ptr);
  if (file_temp)
  {
    acquire_lock_f ();
    file_seek (file_temp->file, *(user_ptr+1));
    release_lock_f ();
  }
}

/* Do system tell, by calling the function file_tell() in filesystem */
void 
sys_tell (struct intr_frame* f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr (user_ptr + 1);
  user_ptr++; //fd
  struct thread_file *thread_file_temp = find_file_id (*user_ptr);
  if (thread_file_temp)
  {
    acquire_lock_f ();
    f->eax = file_tell (thread_file_temp->file);
    release_lock_f ();
  }else{
    f->eax = -1;
  }
}

/* Do system close, by calling the function file_close() in filesystem */
void 
sys_close (struct intr_frame* f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr (user_ptr + 1);
  user_ptr++;
  struct thread_file * opened_file = find_file_id (*user_ptr);
  if (opened_file)
  {
    acquire_lock_f ();
    file_close (opened_file->file);
    release_lock_f ();
    /* Remove the opened file from the list */
    list_remove (&opened_file->file_elem);
    /* Free opened files */
    free (opened_file);
  }
}
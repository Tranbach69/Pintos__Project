#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static int sys_halt (void);
static int sys_exit (int status);
static int sys_exec (const char *ufile);
static int sys_wait (tid_t);

static void copy_in (void *, const void *, size_t);
static struct lock fs_lock;

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  
  lock_init (&fs_lock);
}

static void
syscall_handler (struct intr_frame *f ) 
{
  typedef int syscall_function (int, int, int);

  /* A system call. */
  struct syscall
    {
      size_t arg_cnt;           /* Number of arguments. */
      syscall_function *func;   /* Implementation. */
    };

  /* Table of system calls. */
  /* First number on each row refers to the number of arguments.
     second number on each row is a pointer to the corresponding syscall
     function. */
  static const struct syscall syscall_table[] =
    {
      {0, (syscall_function *) sys_halt},
      {1, (syscall_function *) sys_exit},
      {1, (syscall_function *) sys_exec},
      {1, (syscall_function *) sys_wait},
      {2, (syscall_function *) sys_create},
      {1, (syscall_function *) sys_remove},
      {1, (syscall_function *) sys_open},
      {1, (syscall_function *) sys_filesize},
      {3, (syscall_function *) sys_read},
      {3, (syscall_function *) sys_write},
      {2, (syscall_function *) sys_seek},
      {1, (syscall_function *) sys_tell},
      {1, (syscall_function *) sys_close},
    };

  const struct syscall *sc;
  unsigned call_nr;
  int args[3];
  
  copy_in (&call_nr, f->esp, sizeof call_nr);
  if (call_nr >= sizeof syscall_table / sizeof *syscall_table)
  
  printf ("system call!\n");
  thread_exit ();
  sc = syscall_table + call_nr;

  /* Get the system call arguments. */
  /* notice that the argument array args must be able
      to hold all arguments requested by the system call. */
  ASSERT (sc->arg_cnt <= sizeof args / sizeof *args);
  memset (args, 0, sizeof args);

  /* Copy all system call arguments from user's stack
      into the argument array args. Notice that the element
      at the top of the stack is the call number. This element
      has to be omitted from the copying (i.e. esp+1). */
  copy_in (args, (uint32_t *) f->esp + 1, (sizeof *args) * sc->arg_cnt);

  /* Execute the system call, and set the return value. */
  /* note that the system call return result will be kept
      in eax member of the struct intr_frame. */
  f->eax = sc->func (args[0], args[1], args[2]);
  
}

static bool
verify_user (const void *uaddr)
{
  return (uaddr < PHYS_BASE
          && pagedir_get_page (thread_current ()->pagedir, uaddr) != NULL);
}

/* Copies a byte from user address USRC to kernel address DST.
   USRC must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static inline bool get_user (uint8_t *dst, const uint8_t *usrc)
{
  int eax;
  asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:"
       : "=m" (*dst), "=&a" (eax) : "m" (*usrc));
  return eax != 0;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static inline bool put_user (uint8_t *udst, uint8_t byte)
{
  int eax;
  asm ("movl $1f, %%eax; movb %b2, %0; 1:"
       : "=m" (*udst), "=&a" (eax) : "q" (byte));
  return eax != 0;
}

/* Copies SIZE bytes from user address USRC to kernel address
   DST.
   Call thread_exit() if any of the user accesses are invalid. */
static void copy_in (void *dst_, const void *usrc_, size_t size)
{
  uint8_t *dst = dst_;
  const uint8_t *usrc = usrc_;

  for (; size > 0; size--, dst++, usrc++)
    if (usrc >= (uint8_t *) PHYS_BASE || !get_user (dst, usrc))
      thread_exit ();
}

/* Creates a copy of user string US in kernel memory
   and returns it as a page that must be freed with
   palloc_free_page().
   Truncates the string at PGSIZE bytes in size.
   Call thread_exit() if any of the user accesses are invalid. */
static char * copy_in_string (const char *us)
{
  char *ks;
  size_t length;

  /* NOTE: argument 0 here means to simply return a kernel page if available. */
  ks = palloc_get_page (0);
  if (ks == NULL)
    thread_exit ();

  for (length = 0; length < PGSIZE; length++)
    {
      if (us >= (char *) PHYS_BASE || !get_user (ks + length, us++))
        {
          palloc_free_page (ks);
          thread_exit ();
        }

      if (ks[length] == '\0')
        return ks;
    }

  /* The following applies to the case when the string to copied
     is larger than PGSIZE. In this case, we will never copy '\0' into the
     newly allocated kernel page in the above for loop. So the last byte
     of the kernel page must be set to '\0' to truncate the string being
     copied. */
  ks[PGSIZE - 1] = '\0';
  return ks;
}

/* Halt system call. */
static int sys_halt (void)
{
    /* My Code Begins */
    /* DELETE printf ("system call!\n");
	      thread_exit ();
	      return -1;
    */
    shutdown_power_off ();
    /* My Code Ends */
}

/* Exit system call. */
static int sys_exit (int exit_code)
{
    /* My Code Begins */
    /* DELETE printf ("system call!\n");
	      thread_exit ();
	      return -1;
    */
    thread_current ()->wait_status->exit_code = exit_code;
    thread_exit ();
    NOT_REACHED ();
    /* My Code Ends */
}

/* Exec system call. */
static int sys_exec (const char *ufile)
{
    /* My Code Begins */
    /* DELETE printf ("system call!\n");
	      thread_exit ();
	      return -1;
    */
    //execute the user program and return if the children process is succeeded
    tid_t tid;
    char *new_file = copy_in_string (ufile);

    lock_acquire (&fs_lock);
    tid = process_execute (new_file);
    lock_release (&fs_lock);
    palloc_free_page (new_file);
    return tid;
    /* My Code Ends */
}

/* Wait system call. */
static int sys_wait (tid_t child)
{
    /* My Code Begins */
    /* DELETE printf ("system call!\n");
	      thread_exit ();
	      return -1;
    */
    return process_wait (child);
    /* My Code Ends */
}


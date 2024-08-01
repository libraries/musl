#include <features.h>
#include <stddef.h>

#define START "_start"
#define ENTRYPOINT main

#include "crt_arch.h"
#include "syscall.h"

int ENTRYPOINT(int, char **, char **);

void __init_tls(size_t *);
void __libc_start_init(void);

/* A simpler entrypoint that suits CKB's needs */
void _start_c(long *p) {
  int argc = p[0];
  char **argv = (void *)(p + 1);
  __init_tls(0);
  __libc_start_init();

  int c = ENTRYPOINT(argc, argv, 0);
  for (;;) __syscall(SYS_exit, c);
}

/*
 * The following code is largely copied over and adapted from
 * https://git.musl-libc.org/cgit/musl/tree/src/env/__init_tls.c?id=ab31e9d6a0fa7c5c408856c89df2dfb12c344039
 * Basically we want a way to initialize tls (so errno and other values can be
 * accessed) but also suits CKB's particular environment.
 */
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#include "atomic.h"
#include "libc.h"
#include "pthread_impl.h"
#include "syscall.h"

static struct builtin_tls {
  char c;
  struct pthread pt;
  void *space[16];
} builtin_tls[1];
#define MIN_TLS_ALIGN offsetof(struct builtin_tls, pt)

static struct tls_module main_tls;

void __init_tls(size_t *aux) {
  (void)aux;
  void *mem;

  main_tls.size +=
      (-main_tls.size - (uintptr_t)main_tls.image) & (main_tls.align - 1);
#ifdef TLS_ABOVE_TP
  main_tls.offset = GAP_ABOVE_TP;
  main_tls.offset +=
      (-GAP_ABOVE_TP + (uintptr_t)main_tls.image) & (main_tls.align - 1);
#else
  main_tls.offset = main_tls.size;
#endif
  if (main_tls.align < MIN_TLS_ALIGN) main_tls.align = MIN_TLS_ALIGN;

  libc.tls_align = main_tls.align;
  libc.tls_size = 2 * sizeof(void *) + sizeof(struct pthread)
#ifdef TLS_ABOVE_TP
                      + main_tls.offset
#endif
                      + main_tls.size + main_tls.align + MIN_TLS_ALIGN - 1 &
                  -MIN_TLS_ALIGN;

  if (libc.tls_size > sizeof builtin_tls) {
    /* builtin_tls must be used */
    for (;;) a_crash();
  } else {
    mem = builtin_tls;
  }

  /* Failure to initialize thread pointer is always fatal. */
  if (__init_tp(__copy_tls(mem)) < 0) a_crash();

  libc.can_do_threads = 0;
}

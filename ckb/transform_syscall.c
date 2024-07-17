#include <features.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/uio.h>

#include "bits/syscall.h"
#include "syscall_arch.h"

extern char _end[];
static long __transform_brk(long n, long a, long b, long c, long d, long e,
                            long f, int *processed) {
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;

  if (n != SYS_brk) {
    return (long)-1;
  }

  *processed = 1;
  if (a == 0) {
    return (long)_end;
  } else if ((a >= (long)_end) && (a <= 0x00300000)) {
    return a;
  } else {
    return (long)-1;
  }
}

/* mmap always result in a failure to give alloc functions a chance to recover
 */
static long __transform_mmap(long n, long a, long b, long c, long d, long e,
                             long f, int *processed) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;

  if (n != SYS_mmap) {
    return (long)-1;
  }

  *processed = 1;
  return (long)-1;
}

/* set_tid_address is an always success with 0 as return result
 */
static long __transform_set_tid_address(long n, long a, long b, long c, long d,
                                        long e, long f, int *processed) {
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;

  if (n != SYS_set_tid_address) {
    return (long)-1;
  }

  *processed = 1;
  *((int *)a) = 0;
  return 0;
}

/* ioctl is tested when writing to stdout
 */
static long __transform_ioctl(long n, long a, long b, long c, long d, long e,
                              long f, int *processed) {
  (void)d;
  (void)e;
  (void)f;

  if (n != SYS_ioctl) {
    return (long)-1;
  }

  int fd = (int)a;
  if (fd != 1 && fd != 2) {
    /* Only ioctl to stdout/stderr is processed */
    return (long)-1;
  }
  int op = (int)b;
  if (op != TIOCGWINSZ) {
    return (long)-1;
  }

  struct winsize *wsz = (struct winsize *)c;
  wsz->ws_row = 80;
  wsz->ws_col = 25;
  wsz->ws_xpixel = 14;
  wsz->ws_ypixel = 14;
  *processed = 1;
  return 0;
}

/* This shortcut allows us to skip one level of recursion
 */
static inline long __ckb_debug(char *buffer) {
  register long a7 __asm__("a7") = 2177;
  register long a0 __asm__("a0") = (long)buffer;
  __asm_syscall("r"(a7), "0"(a0))
}

/* writev result is reinterpreted to ckb_debug syscall
 */
static long __transform_writev(long n, long a, long b, long c, long d, long e,
                               long f, int *processed) {
  (void)d;
  (void)e;
  (void)f;

  if (n != SYS_writev) {
    return (long)-1;
  }

  int fd = (int)a;
  if (fd != 1 && fd != 2) {
    /* Only writev to stdout/stderr is processed */
    return (long)-1;
  }

  *processed = 1;
  const struct iovec *iov = (const struct iovec *)b;
  int iovcnt = (int)c;

  ssize_t total = 0;
  for (int i = 0; i < iovcnt; i++) {
    size_t written = 0;
    while (written < iov[i].iov_len) {
      char buffer[1025];
      size_t wrote = iov[i].iov_len - written;
      if (wrote > 1024) {
        wrote = 1024;
      }
      memcpy(buffer, &((char *)iov[i].iov_base)[written], wrote);
      buffer[wrote] = '\0';
      __ckb_debug(buffer);
      written += wrote;
    }
    total += iov[i].iov_len;
  }
  return total;
}

hidden long __ckb_transform_syscall(long n, long a, long b, long c, long d,
                                    long e, long f, int *processed) {
  long code = __transform_brk(n, a, b, c, d, e, f, processed);
  if (*processed != 0) {
    return code;
  }
  code = __transform_writev(n, a, b, c, d, e, f, processed);
  if (*processed != 0) {
    return code;
  }
  code = __transform_ioctl(n, a, b, c, d, e, f, processed);
  if (*processed != 0) {
    return code;
  }
  code = __transform_mmap(n, a, b, c, d, e, f, processed);
  if (*processed != 0) {
    return code;
  }
  code = __transform_set_tid_address(n, a, b, c, d, e, f, processed);
  if (*processed != 0) {
    return code;
  }

  return (long)-1;
}

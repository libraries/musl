#include <stdlib.h>
#include "syscall.h"

_Noreturn void _Exit(int ec)
{
	// CKB-VM does not support SYS_exit_group syscalls
	// __syscall(SYS_exit_group, ec);
	for (;;) __syscall(SYS_exit, ec);
}

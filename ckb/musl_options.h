#ifndef CKB_MUSL_OPTIONS_H_
#define CKB_MUSL_OPTIONS_H_

#define CKB_MUSL_ENABLE_BRK_VIA_GROWING_ELF_END(max_value) \
  long __ckb_hijack_brk_max = (long)(max_value);

#define CKB_MUSL_ENABLE_BRK_VIA_BSS_VALUE(size)              \
  static unsigned char __ckb_hijack_brk_buffer[(size)]       \
      __attribute__((aligned(4096)));                        \
  long __ckb_hijack_brk_min = (long)__ckb_hijack_brk_buffer; \
  long __ckb_hijack_brk_max = (long)(&__ckb_hijack_brk_buffer[size]);

#define __CKB_MUSL_DEFINE_DUMMY_HIJACK_FUNCTION(name) \
  long name(void *c) {                                \
    (void)c;                                          \
    return (long)-1;                                  \
  }

#define CKB_MUSL_DISABLE_BRK_AND_MMAP                       \
  __CKB_MUSL_DEFINE_DUMMY_HIJACK_FUNCTION(__ckb_hijack_brk) \
  __CKB_MUSL_DEFINE_DUMMY_HIJACK_FUNCTION(__ckb_hijack_mmap)

#define CKB_MUSL_DISABLE_STDOUT_TO_CKB_DEBUG                   \
  __CKB_MUSL_DEFINE_DUMMY_HIJACK_FUNCTION(__ckb_hijack_writev) \
  __CKB_MUSL_DEFINE_DUMMY_HIJACK_FUNCTION(__ckb_hijack_ioctl)

#endif /* CKB_MUSL_OPTIONS_H_ */

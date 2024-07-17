#!/bin/bash
set -ex

CLANG="${CLANG:-clang-18}"
BASE_FLAGS="--target=riscv64 -march=rv64imc_zba_zbb_zbc_zbs -DPAGE_SIZE=4096"

mkdir -p release
CC="${CLANG}" CFLAGS="${BASE_FLAGS}" \
  ./configure \
    --target=riscv64-linux-musl \
    --disable-shared \
    --with-malloc=oldmalloc \
    --prefix=`pwd`/release
make AR="${CLANG/clang/llvm-ar}" RANLIB="${CLANG/clang/llvm-ranlib}" install
rm -rf release/bin

rm -rf release/lib/libgcc.a

CKB_FILES=("crt1" "transform_syscall")
for f in ${CKB_FILES[@]}; do
  $CLANG $BASE_FLAGS \
    -nostdinc -O2 \
    -I./arch/riscv64 -I./arch/generic -Iobj/src/internal -I./src/include -I./src/internal -Iobj/include -I./include \
    -Wall -Werror \
    -c ckb/$f.c -o release/$f.o

  "${CLANG/clang/llvm-ar}" rc release/lib/libgcc.a release/$f.o

  rm -rf release/$f.o
done

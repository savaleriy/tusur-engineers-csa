#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
mkdir -p build/ir
cd build/ir

echo "GCC: GIMPLE + optimized"
gcc -O2 -fdump-tree-gimple -fdump-tree-optimized ../../src/ir_demo.c -c

if command -v clang >/dev/null 2>&1; then
    echo "Clang: LLVM IR"
    clang -O2 -S -emit-llvm ../../src/ir_demo.c -o ir_demo.ll
else
    echo "skip LLVM IR: clang not found"
fi

echo
echo "Files:"
ls -1 ir_demo.c.*t.* ir_demo.ll 2>/dev/null || true

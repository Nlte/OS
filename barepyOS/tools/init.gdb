# -*- mode:gdb-script -*-

layout split
focus cmd
winheight cmd 25
target remote:1234

set history filename ~/.gdb_history
set history save

b *after_kmain
b kernel_panic

b *kmain

source utils.gdb

continue

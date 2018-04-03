# -*- mode:gdb-script -*-

layout split
focus cmd
winheight cmd 25
target remote:1234

set history filename ~/.gdb_history
set history save

b *kmain
b *sys_yield
b *sys_yieldto

source utils.gdb

continue

# -*- mode:gdb-script -*-

layout split
focus cmd
winheight cmd 25
target remote:1234

set history filename ~/.gdb_history
set history save

b *kmain
b *do_sys_yieldto
b *user_process_1
b *user_process_2
b sched.c:43

source utils.gdb

continue

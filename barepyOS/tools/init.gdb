# -*- mode:gdb-script -*-

layout split
focus cmd
winheight cmd 25
target remote:1234

set history filename ~/.gdb_history
set history save

b *kmain
b *irq_handler
b *c_halt

source utils.gdb

continue

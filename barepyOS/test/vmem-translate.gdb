# -*- mode: gdb-script -*-

set verbose off
set confirm off
 break kmain-vmem.c:13
commands
  set $ok = 1
  set $ok *= (res == 1193046)
  if $ok
    printf "test OK\n"
  else
    printf "test ERROR\n"
  end
  quit
end

target remote:1234
continue

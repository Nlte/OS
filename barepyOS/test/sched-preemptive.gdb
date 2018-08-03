# -*- mode: gdb-script -*-

set verbose off
set confirm off

break finish_line
commands
  printf "PASSED\n"
  quit
end

target remote:1234
continue

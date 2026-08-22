# gdb command file used by run-under-gdb.sh
set pagination off
set confirm off
set backtrace past-main on
handle SIGPIPE nostop noprint pass

run

# $_exitcode is void if the program did not exit on its own, i.e. it was
# stopped by a fatal signal.
if $_isvoid($_exitcode)
  echo \n========== CRASH DETAILS BELOW ==========\n
  info program
  echo \n---------- faulting thread ----------\n
  bt full
  echo \n---------- registers ----------\n
  info registers
  echo \n---------- all threads ----------\n
  thread apply all bt full
  echo \n---------- loaded libraries ----------\n
  info sharedlibrary
  echo \n---------- writing core file ----------\n
  generate-core-file
  echo \n========== END OF CRASH DETAILS ==========\n
else
  echo \n---------- JackTrip exited on its own, no crash was captured ----------\n
end
quit

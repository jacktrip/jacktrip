set pagination off
set confirm off
set backtrace past-main on
# Signals JackTrip and its Qt/Chromium threads use routinely. Stopping on any of
# these would end the session before the real crash ever happens.
handle SIGPIPE nostop noprint pass
handle SIGUSR1 nostop noprint pass
handle SIGUSR2 nostop noprint pass
run
# $_exitcode is void if the program did not exit on its own, i.e. it was
# stopped by a fatal signal.
if $_isvoid($_exitcode)
  # si_signo 2 is SIGINT: the user stopped it from the keyboard, which is
  # not a crash and produces no useful backtrace.
  set $sig = 0
  if !$_isvoid($_siginfo)
    set $sig = $_siginfo.si_signo
  end
  if $sig == 2
    echo \n---------- JackTrip was interrupted, not a crash ----------\n
  else
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
  end
else
  echo \n---------- JackTrip exited on its own, no crash was captured ----------\n
end
quit

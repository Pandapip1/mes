;;; fork-test.scm -- manual smoke test for lib/windows/fork.c's WOW64 clone.
;;; Not wired into check-boot.sh; run directly with MES_BOOT=scaffold/boot/fork-test.scm.
(define pid (primitive-fork))
(if (= pid 0)
    (begin
      (core:display "CHILD\n")
      (primitive-exit 42))
    (begin
      (core:display "PARENT pid=")
      (core:display pid)
      (core:display "\n")
      (define r (core:waitpid pid 0))
      (core:display "PARENT saw child status=")
      (core:display (cdr r))
      (core:display "\n")
      (primitive-exit 7)))

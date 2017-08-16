(begin
    (define (welcome)
        (displayln "Welcome to the REPL!")
        (displayln "Use Ctrl+D to exit!")
        (newline))

    (define (tle-eval expr)
        (eval expr (top-level-environment)))

    (define (goodbye)
        (newline)
        (displayln "Quitting!"))

    (define (repl)
        (display ">> ")
        (let ((expr (read)))
            (if (not (eof-object? expr))
                (let ((result (tle-eval expr)))
                     (if (not (void? result))
                             (writeln result))
                     (repl)))))

    (welcome)
    (repl)
    (goodbye))

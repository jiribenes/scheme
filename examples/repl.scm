(begin
    (define (welcome)
        (displayln "Welcome to the REPL!")
        (displayln "Use Ctrl+D to exit!")
        (newline))

    (define (goodbye)
        (newline)
        (displayln "Quitting!"))

    (define (repl)
        (display ">> ")
        ((lambda (expr)
            (if (not (eof-object? expr))
                (begin
                    (define result (eval expr cur-env))
                    (if (not (void? result))
                        (writeln result))
                    (repl))))
        (read)))

    (welcome)
    (repl)
    (goodbye))


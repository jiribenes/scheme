(begin
    (define (repl)
        (display ">> ")
        ((lambda (expr)
            (if (eof-object? expr)
                (displayln "Quitting!")
                (begin
                    (write (eval expr cur-env))
                    (repl))))
        (read)))

    (repl))


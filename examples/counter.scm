(begin
    (define counter 
        (let ((x 0))
            (lambda ()
                (set! x (+ x 1))
                x)))

    (displayln (counter))
    (displayln (counter))
    (displayln (counter))
)

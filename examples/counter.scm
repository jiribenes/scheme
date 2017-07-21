(begin
    (define counter 
        ((lambda (x) 
            (lambda () 
                (set! x (+ x 1))
                x)) 
         0))

    (display (counter))
    (display (counter))
    (display (counter))
)

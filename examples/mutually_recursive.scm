; the demo of mutually recursive functions
(begin
    (define (even? x)
        (if (= x 0)
            #t
            (odd? (- x 1))))

    (define (odd? x)
        (if (= x 0)
            #f
            (even? (- x 1))))

    (display "Is 10 even? ")
    (display (even? 10))
    (newline)

    (display "Is 14 odd? ")
    (display (odd? 14))
    (newline))

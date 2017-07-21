(begin
    (define add
        (lambda (x)
            (lambda (y)
                (+ x y))))

    (define inc (add 1))

    (test (equal? (inc 2) 3) #t)
)

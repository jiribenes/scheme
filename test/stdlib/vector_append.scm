(begin
    (let ((empty #()))
        (test (equal? (vector-append empty empty) empty) #t)
        (test (equal? (vector-append #() #()) empty) #t)
        (test (equal? (vector-append #(1) empty) #(1)) #t)
        (test (equal? (vector-append empty #(1)) #(1)) #t)
        (test (equal? (vector-append #(1 2 3 4) #(5 6)) #(1 2 3 4 5 6)) #t)))

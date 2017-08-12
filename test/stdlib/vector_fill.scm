(begin
    (define vec (vector 1 2 3))
    (vector-fill! vec 0)
    (test (equal? #(0 0 0) vec) #t)

    (set! vec (make-vector 5 5))
    (vector-fill! vec -1)
    (test (equal? #(-1 -1 -1 -1 -1) vec) #t)
)

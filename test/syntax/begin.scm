(begin
    (test (eq? (begin) (void)) #t)
    (test (eq? (begin 1) 1) #t)
    (test (eq? (begin 1 2 3) 3) #t)
    (test (eq? (begin 'a 'b 'c) 'c) #t)
    (test (equal? (begin (define a 20) (define a 30) a) 30) #t)
)

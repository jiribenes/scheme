(begin
    (test (equal? ((lambda (x) x) 5) 5) #t)
    (test (equal? ((lambda (x) x) 0) 0) #t)
    (test (equal? ((lambda (x) (- x)) 5) -5) #t)
    (test (equal? ((lambda (x) (+ x)) 5) 5) #t)
    (test (equal? ((lambda (x) x) -5) -5) #t)
    (test (equal? ((lambda (x) (+ x 2)) 5) 7) #t)
    (test (equal? ((lambda (x) (* x 2)) 5) 10) #t)
)

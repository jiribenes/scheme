(begin
    (test (equal? ((lambda () 0)) 0) #t)
    (test (equal? ((lambda () 'aaa)) 'aaa) #t)
    (test (equal? ((lambda () '(1 2))) '(1 2)) #t)
    (test (equal? ((lambda () "aaa")) "aaa") #t)

    (define a 10)
    (test (equal? ((lambda () a)) 10) #t)
    (test (equal? ((lambda () (define a 11) a)) 11) #t)
)

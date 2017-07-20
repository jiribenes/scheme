(begin
    ; these are directly copied from eq.scm
    (test (equal? #t #t) #t)
    (test (equal? #f #t) #f)
    (test (equal? #t #f) #f)
    (test (equal? #f #f) #t)

    (test (equal? 1 1) #t)
    (test (equal? 0 1) #f)
    (test (equal? -1 -1) #t)
    (test (equal? 0 -0) #f)
    (test (equal? 42.5 42.5) #t)
    (test (equal? 99.999 99.999) #t)
    
    (test (equal? 'a 'a) #t)
    (test (equal? 'a 'b) #f)

    (test (equal? "aaa" "aaa") #t)
    (test (equal? "aaa" "bbb") #f)
    (test (equal? "aaa" "a") #f)

    (test (equal? '(1 2 3) '(1 2 3)) #t)
    (test (equal? '('a 'b 'c) '('a 'b 'c)) #t)
    (test (equal? '(1 2 3) '(1 2)) #f)
    (test (equal? '("aaa" "bbb") '("aaa" "bbb")) #t)

    (test (equal? '(1 . 2) '(1 2)) #f)
    (test (equal? '(1 . 2) '(1 2 3)) #f)
    (test (equal? '((1 . 2) . 3) '(1 . (2 . 3))) #f)
    (test (equal? '(1 . (2 . 3)) '(1 2 . 3)) #t)
)

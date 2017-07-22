(begin
    (test (car '(1 2)) 1)
    (test (car '(a b)) 'a)
    (test (equal? (car '("aa" "a")) "aa") #t)
    (test (car '(1 2 3 4 5)) 1)
    (test (car '(1 . 2)) 1)
    (test (equal? (car '((1 . 2) . 2)) '(1 . 2)) #t)
    (test (equal? (car '((1 2 3) . 4)) '(1 2 3)) #t)
)

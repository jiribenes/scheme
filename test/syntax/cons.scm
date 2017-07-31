(begin
    (test (equal? '() (list)) #t)
    (test (equal? '(1 2 3) (list 1 2 3)) #t)
    (test (equal? '(1 (2) 3) (list 1 (list 2) 3)) #t)
    (test (equal? '(1 . 3) (cons 1 3)) #t)
    (test (equal? '(1 . (3)) (list 1 3)) #t)
)

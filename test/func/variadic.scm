(begin
    (define (list-test . args) args)
    (test (equal? (list-test 1 2 3 4 5) '(1 2 3 4 5)) #t)
    (test (equal? (list-test) '()) #t)

    (define (tail x . y) y)
    (test (equal? (tail 1 2 3 4 5) '(2 3 4 5)) #t)
    (test (equal? (tail 1 2) '(2)) #t)

    (define (head x . y) x)
    (test (equal? (head 1 2 3 4 5) 1) #t)
    (test (equal? (head '(1 2 3 4) 5) '(1 2 3 4)) #t)

    (define (snd x y . z) y)
    (test (equal? (snd 1 2 3 4 5) 2) #t)
    (test (equal? (snd "aaa" 'sym "bbb" '(1 2 3 4 5)) 'sym) #t)
    (test (equal? (snd '(1 2) '(3 4) '(5 6) 7 8 "end") '(3 4)) #t)
)

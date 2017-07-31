(begin
    (test ((lambda args (reduce + args)) 1 2 3 4 5) 15)
    (test (equal? ((lambda (head . tail) tail) 1 2 3 4 5) '(2 3 4 5)) #t)
    (test (equal? ((lambda (head . tail) head) 1 2 3 4 5) 1) #t)
    (test (equal? ((lambda (fst snd . rest) snd) 1 2 3 4 5) 2) #t)
)

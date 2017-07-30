(begin
    (define-macro (answer) 42)
    (test (eq? ((lambda () (answer))) 42) #t)
    (test (eq? (expand (answer)) 42) #t)
)

(begin
    (define a (gensym))
    (test (eq? a 'g0) #f)
    (test (eq? a a) #t)
    (test (equal? a 'g0) #f)
    (test (eq? (gensym) (gensym)) #f)
    (test (equal? (gensym) (gensym)) #f)
    (test ((lambda (sym) (eq? sym sym)) (gensym)) #t)
)

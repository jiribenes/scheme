(begin
    (define-macro (ifpos a then) (list 'if (list '> a 0) then))
    (test (equal? (ifpos 42 "aaa") "aaa") #t)
    (test (equal? (expand (ifpos a (display a))) '(if (> a 0) (display a))) #t)
)

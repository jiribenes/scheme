(begin
    (define x 10)
    (test (equal?
            (when (> 10 0)
                (define tmp x)
                (set! x 42)
                (list tmp x))
            (list 10 42))
          #t)
    (test x 42)
)

(begin
    (define x 1000)

    (let ()
        (test x 1000))

    (let ((x 90))
        (test x 90))

    (let ()
        (define x 2)
        (test (+ x 1) 3))

    (test (let ((x 2) (y 3))
               (let ((x 7)
                     (z (+ x y)))
                    (* z x))) 35)

    (let ()
        (define x 100)
        (set! x 10)
        (test (+ x 1) 11)))


; An example std library
(begin
    (define (caar x) (car (car x)))
    (define (cadr x) (car (cdr x)))
    (define (cdar x) (cdr (car x)))
    (define (cddr x) (cdr (cdr x)))

    (define pair? cons?)
    (define true #t)
    (define null ())
    (define false #f)

    (define (bool? x)
        (if (eq? x #f)
            #t
            (if (eq? x #t)
                #t
                #f)))

    (define (not x)
        (if x #f #t))

    (define (map f lst)
        (if (null? lst)
            ()
            (cons (fn (car lst))
                (map fn (cdr lst)))))

    'stdlib)

; An example std library
(begin
    (define (caar x) (car (car x)))
    (define (cadr x) (car (cdr x)))
    (define (cdar x) (cdr (car x)))
    (define (cddr x) (cdr (cdr x)))

    (define pair? cons?)
    (define true #t)
    (define null '())
    (define false #f)
    (define (null? x)
        (eq? x '()))

    (define (bool? x)
        (if (eq? x #f)
            #t
            (if (eq? x #t)
                #t
                #f)))

    (define (not x)
        (if x #f #t))

    (define (map fn lst)
        (if (null? lst)
            '()
            (cons (fn (car lst))
                (map fn (cdr lst)))))

    (define (filter fn lst)
        (if (null? lst)
            '()
            (if (fn (car lst))
                (cons (car lst)
                      (filter fn (cdr lst)))
                (filter fn (cdr lst)))))

    (define (foldl fn acc lst)
        (if (null? lst)
            acc
            (foldl fn
                   (fn acc (car lst))
                   (cdr lst))))

    (define (foldr fn acc lst)
        (if (null? lst)
            acc
            (fn (car lst)
                (foldr fn acc (cdr lst)))))

    (define (reduce fn lst)
        (foldl fn (car lst) (cdr lst)))
    
    (define (test a b)
        (write
            (eq? a b)))


    'stdlib)

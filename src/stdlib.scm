; An example std library
(begin
    (define (caar x) (car (car x)))
    (define (cadr x) (car (cdr x)))
    (define (cdar x) (cdr (car x)))
    (define (cddr x) (cdr (cdr x)))

    (define (caaar x) (car (car (car x))))
    (define (caadr x) (car (car (cdr x))))
    (define (cadar x) (car (cdr (car x))))
    (define (caddr x) (car (cdr (cdr x))))
    (define (cdaar x) (cdr (car (car x))))
    (define (cdadr x) (cdr (car (cdr x))))
    (define (cddar x) (cdr (cdr (car x))))
    (define (cdddr x) (cdr (cdr (cdr x))))

    (define pair? cons?)

    (define true #t)
    (define false #f)

    (define (not x)
        (if x #f #t))

    (define (bool? x)
        (if (eq? x #f)
            #t
            (if (eq? x #t)
                #t
                #f)))

    (define (void? x)
        (eq? x (void)))

    (define (undefined? x)
        (eq? x undefined))

    (define (eof-object? x)
        (eq? x eof))

    (define boolean? bool?)

    (define (nan? x)
        (if (number? x)
            (if (not (= x x))
                #t
                #f) 
            #f))

    (define inf (/ 1 0))

    (define (infinite? x)
        (if (number? x)
            (if (or (= x inf) (= x (- inf)))
                #t
                #f)
            #f))

    (define null '())
    (define (null? x)
        (eq? x '()))

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

    (define-macro (when test . then)
        (list 'if test
              (cons 'begin then)))

    (define-macro (unless test . then)
        (list 'if
              (cons 'not test)
              (cons 'begin branch)))

    (define (test a b)
        (write
            (eq? a b)))


    'stdlib)

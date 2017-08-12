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

    (define (list . args) args)

    (define (writeln x)
        (write x)
        (newline))

    (define (displayln x)
        (display x)
        (newline))

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
        (eq? x (undefined)))

    (define (eof-object? x)
        (eq? x eof))

    (define boolean? bool?)

    (define (nan? x)
        (if (number? x)
            (if (not (builtin= x x))
                #t
                #f) 
            #f))

    (define inf (builtin/ 1 0))

    (define (infinite? x)
        (if (number? x)
            (if (or (builtin= x inf) (builtin= x (builtin- 0 inf)))
                #t
                #f)
            #f))

    (define null '())
    (define (null? x)
        (eq? x '()))

    (define pair? cons?)
    (define (list? x)
        (if (not (or (cons? x)
                     (null? x)))
            #f
            (define len (builtin-length x))
            (>= len 0)))

    (define (length lst)
        (define len (builtin-length lst))
        (if (>= len 0)
            len
            (error "length: argument is not 'list?'")))

    (define (list-copy lst)
        (if (list? lst)
            (apply list lst)
            (error "list-copy: argument is not 'list?'")))
            
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

    (define (+ . args) (foldl builtin+ 0 args))
    (define (- a . args)
        (if (null? args)
            (builtin- 0 a)
            (builtin- a (apply + args))))
    (define (* . args) (foldl builtin* 1 args))
    (define (/ a . args)
        (if (null? args)
            (builtin/ 1 a)
            (builtin/ a (apply * args))))

    (define (pairs lst)
        (if (null? (cdr lst))
            '()
            (cons (list (car lst) (cadr lst))
                  (pairs (cdr lst)))))

    (define (pairs-op fn)
        (lambda args
            (if (or (null? args) (null? (cdr args)))
                #t
                (reduce and
                        (map (lambda (pair) (apply fn pair))
                             (pairs args))))))

    (define = (pairs-op builtin=))
    (define > (pairs-op builtin>))
    (define < (pairs-op builtin<))
    (define (>= . args) (not (apply < args)))
    (define (<= . args) (not (apply > args)))

    (define-macro (when test . then)
        (list 'if test
              (cons 'begin then)))

    (define-macro (unless test . then)
        (list 'if
              (cons 'not test)
              (cons 'begin branch)))

    (define (drop n lst)
        (if (null? lst)
            '()
            (if (= n 0)
                lst
                (drop (- n 1) (cdr lst)))))

    (define (list-ref lst i)
        (car (drop i lst)))

    (define (vector . args)
        (define len (length args))
        (define vec (make-vector len 0))
        (define (vec-set i rest)
            (if (= i len)
                vec
                (begin
                    (vector-set! vec i (car rest))
                    (vec-set (+ i 1) (cdr rest)))))
        (vec-set 0 args))

    (define (list->vector lst) (apply vector lst))

    (define (vector->list vec)
        (define len (vector-length vec))
        (define (helper i)
            (if (= i len)
                '()
                (cons (vector-ref vec i) (proc (+ i 1)))))
        (helper 0))

    (define (vector-map fn vec)
        (define len (vector-length vec))
        (define new-vec (make-vector len 0))
        (define (helper i)
            (if (= i len)
                new-vec
                (begin
                    (define elem (fn (vector-ref vec i)))
                    (vector-set! new-vec i elem)
                    (helper (+ i 1)))))
        (helper 0))

    (define (vector-fill! vec fill)
        (define len (vector-length vec))
        (define (helper i)
            (if (= i len)
                (void)
                (begin
                    (vector-set! vec i fill)
                    (helper (+ i 1)))))
        (helper 0))

    (define (vector-empty? vec)
        (= (vector-length vec) 0))

    (define (test a b)
        (writeln
            (eq? a b)))

    (define loaded-time (current-time))

    'stdlib)

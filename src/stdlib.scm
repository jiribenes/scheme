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
        (or (eq? x #t) (eq? x #f)))

    (define (void? x)
        (eq? x (void)))

    (define (undefined? x)
        (eq? x (undefined)))

    (define (eof-object? x)
        (eq? x eof))

    (define boolean? bool?)

    (define (nan? x)
        (and (number? x) (not (builtin= x x))))

    (define inf (builtin/ 1 0))

    (define (infinite? x)
        (and (number? x)
             (or (builtin= x inf) (builtin= x (builtin- 0 inf)))))

    (define null '())
    (define (null? x)
        (eq? x '()))

    (define pair? cons?)
    (define (list? x)
        (if (not (or (cons? x)
                     (null? x)))
            #f
            (let ((len (builtin-length x)))
                 (>= len 0))))

    (define (length lst)
        (let ((len (builtin-length lst)))
             (if (>= len 0)
                 len
                 (error "length: argument is not 'list?'"))))

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
              (list 'not test)
              (cons 'begin then)))

    (define (drop n lst)
        (if (null? lst)
            '()
            (if (= n 0)
                lst
                (drop (- n 1) (cdr lst)))))

    (define (list-ref lst i)
        (car (drop i lst)))

    (define (member? lst x)
        (if (null? lst)
            #f
            (if (equal? x (car lst))
                #t
                (member? (cdr lst) x))))

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

    (define (vector-for-each fn vec)
        (define len (vector-length vec))
        (define (helper i)
            (if (= i len)
                (void)
                (fn (vector-ref vec i))
                (helper (+ i 1))))
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

    (define (make-hash eq)
        (let ((empty (cons (void) #f))
              (capacity 0)
              (count 0))
            (define vec (make-vector capacity empty))
            (define ht (vector eq count capacity vec))
            ht))

    (define (hash-eq ht)
        (vector-ref ht 0))
    (define (hash-count ht)
        (vector-ref ht 1))
    (define (hash-capacity ht)
        (vector-ref ht 2))
    (define (hash-data ht)
        (vector-ref ht 3))

    (define (hash? ht)
        (and (vector? ht)
             (= (vector-length ht) 4)
             (procedure? (hash-eq ht))
             (integer? (hash-count ht))
             (integer? (hash-capacity ht))
             (vector? (hash-data ht))
             (= (vector-length (hash-data ht)) (hash-capacity ht))))

    (define (hash-entry ht key)
        (let ((capacity (hash-capacity ht))
              (vec (hash-data ht))
              (key-hash (hash key))
              (tombstone-index -1)
              (result '()))
             (define start-index (remainder (hash key) capacity))
             (define (ht-helper index init)
                (if (and (builtin= index start-index) (not init))
                    (cons #f tombstone-index)
                    (let ((elem (vector-ref vec index))
                          (next-index (remainder (+ 1 index) capacity)))
                         (if (void? (car elem))
                             (if (eq? #f (cdr elem))
                                 (if (= tombstone-index -1)
                                     (set! result (cons #f index))
                                     (set! result (cons #f tombstone-index)))
                                 (if (and (eq? #t (cdr elem))
                                          (builtin= tombstone-index -1))
                                     (set! tombstone-index index))))
                             (if ((hash-eq ht) (car elem) key)
                                 (set! result (cons #t index)))
                             (if (null? result)
                                 (ht-helper next-index #f)
                                 result))))
             (if (builtin= capacity 0)
                (cons #f -1)
                (ht-helper start-index #t))))

    (define (hash-exists? ht key)
        (car (hash-entry ht key)))

    (define (hash-ref ht key)
        (let ((entry (hash-entry ht key)))
             (if (car entry)
                 (cdr (vector-ref (hash-data ht) (cdr entry)))
                 (error "Key is not present in hash!"))))

    (define (hash-set! ht key value)
        (if (> (+ 1 (hash-count ht))
               (* 0.75 (hash-capacity ht)))
          (hash-maybe-resize! ht))
        (let ((entry (hash-entry ht key))
              (vec (hash-data ht))
              (count (hash-count ht)))
          (if (car entry)
              (vector-set! vec (cdr entry) (cons key value))
              (if (= -1 (cdr entry))
                  (error "Cannot insert to hash!")
                  (begin
                    (vector-set! vec (cdr entry) (cons key value))
                    (vector-set! ht 1 (+ 1 count)))))))

    (define (hash-maybe-resize! ht)
        (let ((count (hash-count ht))
              (capacity (hash-capacity ht)))
            (if (> (+ 1 count) (* 0.75 capacity))
                (begin
                    (define new-capacity (* 2 capacity))
                    (if (< new-capacity 8)
                        (set! new-capacity 8))
                    (hash-resize! ht new-capacity))
                (if (and (> capacity 8) (< count (/ capacity 4)))
                    (begin
                        (define new-capacity (/ capacity 4))
                        (if (< new-capacity 8)
                            (set! new-capacity 8))
                        (hash-resize! ht new-capacity))))))

    (define (hash-resize! ht capacity)
        (let ((old-vec (hash-data ht))
              (empty (cons (void) #f)))
          (define vec (make-vector capacity empty))
          (vector-set! ht 1 0)
          (vector-set! ht 2 capacity)
          (vector-set! ht 3 vec)
          (if (> capacity 0)
              (vector-for-each
                  (lambda (elem)
                      (if (not (void? (car elem)))
                          (hash-set! ht (car elem) (cdr elem))))
                  old-vec))
          (void)))

    (define (hash-delete! ht key)
        (let ((entry (hash-entry ht key))
              (vec (hash-data ht)))
          (if (not (car entry))
              (void)
              (vector-set! vec (cdr entry) (cons (void) #t))
              (vector-set! ht 1 (- (hash-count ht) 1))
              (hash-maybe-resize! ht)
              (void))))

    (define (hash-walk ht fn)
        (vector-for-each
            (lambda (elem)
                (if (not (void? (car elem)))
                    (fn (car elem) (cdr elem))))
            (hash-data ht))
        (void))

    (define (hash->alist ht)
        (define result '())
        (hash-walk
            (lambda (key value)
                (set! result (cons key value)))))

    (define loaded-time (current-time))

    'stdlib)

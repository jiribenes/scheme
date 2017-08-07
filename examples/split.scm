; Splits a list into members of odd and even position
(begin

  (define (alt lst)
    (if (null? lst)
        lst
        (if (null? (cdr lst))
            lst
            (cons (car lst)
                  (alt (cddr lst))))))

  (define (split lst)
    (list (alt lst) (alt (cdr lst))))

  (displayln (split (list 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16))))

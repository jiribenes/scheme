(begin
    ; True/False table
    (test (eq? #t #t) #t)
    (test (eq? #f #t) #f)
    (test (eq? #t #f) #f)
    (test (eq? #f #f) #t)

    (test (eq? 1 1) #t)
    (test (eq? 0 1) #f)
    (test (eq? -1 -1) #t)
    (test (eq? 0 -0) #f)
    (test (eq? 42.5 42.5) #t)
    (test (eq? 99.999 99.999) #t)
    
    (test (eq? 'a 'a) #t)
    (test (eq? 'a 'b) #f)

    (test (eq? "aaa" "aaa") #f)
)

(begin
    (test (>= 0 0) #t)
    (test (>= 2 1) #t)
    (test (>= 1 2) #f)
    (test (>= 1 -1) #t)
    (test (>= -1 -1) #t)
    (test (>= -1 -2) #t)
    (test (>= -2 -1) #f)
)

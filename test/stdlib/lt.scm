(begin
    (test (< 1 2) #t)
    (test (< 0 0) #f)
    (test (< 1 1) #f)
    (test (< -1 1) #t)
    (test (< -1 -1) #f)
    (test (< -2 -1) #t)
)

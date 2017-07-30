(begin
    (test (nan? 1) #f)
    (test (nan? 0) #f)
    (test (nan? -0.5) #f)
    (test (nan? (/ -1 0)) #f)
    (test (nan? (/ 0 0)) #t)
    (test (nan? (/ 0.5 0)) #f)
)

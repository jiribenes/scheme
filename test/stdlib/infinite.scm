(begin
    (test (infinite? 0) #f)
    (test (infinite? 1) #f)
    (test (infinite? 0.5) #f)
    (test (infinite? (/ 1 0)) #t)
    (test (infinite? (/ -42 0)) #t)
)

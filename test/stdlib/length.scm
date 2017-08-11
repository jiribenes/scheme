(begin
    (test (length '()) 0)
    (test (length '(1)) 1)
    (test (length '(1 2)) 2)
    (test (length (list 1 2 3 4 5)) 5)
    (test (length '(1 (2 3) 4)) 3)
    (test (length '((1 . 2) (3 . 4))) 2)
)

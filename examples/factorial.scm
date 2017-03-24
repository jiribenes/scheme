(begin (define (factorial n) (if (eq? n 1) 1 (* n (factorial (- n 1))))) (factorial 50))

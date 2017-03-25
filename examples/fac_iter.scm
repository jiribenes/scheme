(begin
  (define (fac x acc)
    (if (eq? x 0)
      acc
      (fac (- x 1) (* x acc))))
  (define (factorial n)
    (fac n 1))
  (factorial 10))


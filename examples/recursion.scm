(begin ; sums numbers from 0 .. x
  (define (sum x)
    (if (= x 0)
        0
        (+ x (sum (- x 1)))))

  (sum 10)) ; should be 55

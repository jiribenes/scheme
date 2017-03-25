(begin ; sums numbers from 0 .. x 
  (define (sum x) 
    (if (eq? x 0) 0 
      (+ (sum (- x 1)) x))) 
  (sum 10)) ; should be 55

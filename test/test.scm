(begin
(define *tests-run-total* 0)
(define *tests-passed-total* 0)
(define *tests-run* 0)
(define *tests-passed* 0)

(define (test-new) (set! *tests-run* (+ *tests-run* 1)))
(define (test-passed) (set! *tests-passed* (+ *tests-passed* 1)))

(define-macro (test expr expected)
              (list 'begin
                    (list 'test-new)
                    (list 'display '*tests-run*)
                    (list 'display ". ")
                    (list 'write (list 'quote expr))
                    (list 'if (list 'equal? expr expected)
                              (list 'begin
                                    (list 'test-passed)
                                    (list 'displayln " [PASS]"))
                              (list 'displayln " [FAIL]")
                              (list 'display "   expected ")
                              (list 'write (list 'quote expected))
                              (list 'display " but got ")
                              (list 'write expr)
                              (list 'displayln " instead!"))))

(define (test-begin name)
    (display "TEST: ")
    (displayln name))

(define (test-end)
    (write *tests-passed*)
    (display " out of ")
    (write *tests-run*)
    (displayln " passed!")
    (set! *tests-run-total* (+ *tests-run-total* *tests-run*))
    (set! *tests-passed-total* (+ *tests-passed-total* *tests-passed*))
    (set! *tests-run* 0)
    (set! *tests-passed* 0))

(define (tests-start)
    (displayln "Testing in progress!"))

(define (tests-end)
    (display "Total: ")
    (write *tests-passed-total*)
    (display " out of ")
    (write *tests-run-total*)
    (displayln " passed!")
    (if (not (= *tests-passed-total* *tests-run-total*))
        (exit 1)))

(define (test-run filename)
    (test-begin filename)
    (load filename)
    (test-end))

(begin
    (tests-start)

    (test-run "test/load/test.scm")
    
    (test-run "test/func/variadic_lambda.scm")
    (test-run "test/func/anon.scm")
    (test-run "test/func/variadic.scm")
    (test-run "test/func/anon_no_args.scm")
    (test-run "test/func/closure.scm")
    
    (test-run "test/core/multiply.scm")
    (test-run "test/core/car.scm")
    (test-run "test/core/apply.scm")
    (test-run "test/core/remainder.scm")
    (test-run "test/core/div.scm")
    (test-run "test/core/cdr.scm")
    (test-run "test/core/equal.scm")
    (test-run "test/core/let.scm")
    (test-run "test/core/eq.scm")
    (test-run "test/core/subtract.scm")
    (test-run "test/core/type_pred.scm")
    (test-run "test/core/add.scm")
    
    (test-run "test/macro/basic.scm")
    (test-run "test/macro/variadic.scm")
    (test-run "test/macro/gensym.scm")
    (test-run "test/macro/ifpos.scm")

    (test-run "test/stdlib/vector_fill.scm")
    (test-run "test/stdlib/le.scm")
    (test-run "test/stdlib/hash_table.scm")
    (test-run "test/stdlib/list_pred.scm")
    (test-run "test/stdlib/infinite.scm")
    (test-run "test/stdlib/vector_append.scm")
    (test-run "test/stdlib/nan.scm")
    (test-run "test/stdlib/gt.scm")
    (test-run "test/stdlib/lt.scm")
    (test-run "test/stdlib/length.scm")
    (test-run "test/stdlib/numeq.scm")
    (test-run "test/stdlib/ge.scm")
    (test-run "test/syntax/begin.scm")
    (test-run "test/syntax/cons.scm")
    (test-run "test/syntax/vector.scm")
    (tests-end)))
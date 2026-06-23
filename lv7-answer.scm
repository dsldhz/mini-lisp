(define data '(12 71 2 15 29 82 87 8 18 66 81 25 63 97 40 3 93 58 53 31 47))

(define (quick-sort items)
  (if (null? items)
      '()
      (let ((pivot (car items))
            (rest (cdr items)))
        (append (quick-sort (filter (lambda (x) (< x pivot)) rest))
                (list pivot)
                (quick-sort (filter (lambda (x) (>= x pivot)) rest))))))

(print (quick-sort data))

; This is case 1 program.
; This case is used to recognize language L1 = {a^ib^ja^ib^j|i,j > 0}
; Input: a string of a's and b's, e.g. 'aabbbaabbb'

; the finite set of states
#Q = {q10,q11,q12,q20,q21,q22,q_rej,q_rej_tail,q_acc,q_true,q_t,q_tr,q_tru,q_f,q_fa,q_fal,q_fals,q_false}

; the finite set of input symbols
#S = { a, b}

; the complete set of tape symbols
#G = { a, b, _, t, r, u, e, f, a, l, s}

; the start state
#q0 = q10

; the blank symbol
#B = _

; the set of final states
#F = {q_true}

; the number of tapes
#N = 2

; the transition function

;step1. Store the first half a+b+ into the second tape and delete it on the first tape
q10 a_ _a rr q11
q10 b_ __ r* q_rej
q10 __ __ ** q_rej_tail
q11 a_ _a rr q11
q11 b_ _b rr q12
q11 __ __ ** q_rej_tail
q12 a_ a_ *l q20
q12 b_ _b rr q12
q12 __ __ ** q_rej_tail

;step2. Compare each bit of two tapes
q20 aa aa *l q20
q20 ab ab *l q20
q20 a_ a_ *r q21
q21 aa __ rr q21
q21 ab __ r* q_rej
q21 ba __ r* q_rej
q21 bb __ rr q22
q21 _a __ ** q_rej_tail
q21 _b __ ** q_rej_tail
q22 ab __ r* q_rej
q22 a_ __ r* q_rej
q22 bb __ rr q22
q22 b_ __ r* q_rej
q22 _b __ ** q_rej_tail
q22 __ __ ** q_acc

;step3.q_rej=>q_rej_tail=>q_false q_acc=>qtrue
q_rej __ __ ** q_rej_tail
q_rej a_ __ r* q_rej
q_rej b_ __ r* q_rej
q_rej_tail __ f_ r* q_f
q_f __ a_ r* q_fa
q_fa __ l_ r* q_fal
q_fal __ s_ r* q_fals
q_fals __ e_ r* q_false

q_acc __ t_ r* q_t
q_t __ r_ r* q_tr
q_tr __ u_ r* q_tru
q_tru __ e_ r* q_true




; old solution - too trivial
; step1. Traverse the input string to ensure its type meets a+b+a+b+, otherwise turn to reject
;q10 a_ a_ r* q11
;q10 b_ b_ ** q_rej
;q10 __ __ ** q_rej_tail
;q11 a_ a_ r* q11
;q11 b_ b_ r* q12
;q11 __ __ l* q_rej_tail
;q12 a_ a_ r* q13
;q12 b_ b_ r* q12
;q12 __ __ l* q_rej_tail
;q13 a_ a_ r* q13
;q13 b_ b_ r* q14
;q13 __ __ l* q_rej_tail
;q14 a_ a_ ** q_rej
;q14 b_ b_ r* q14
;q14 __ __ l* q15
;q15 a_ a_ l* q15
;q15 b_ b_ l* q15
;q15 __ __ r* q20

; step2. store the first half a+b+ into the second tape and delete it on the first tape
;q20 a_ _a rr q21
;q21 a_ _a rr q21
;q21 b_ _b rr q22
;q22 b_ _b rr q22
;q22 a_ a_ *l q23
;q23 aa aa *l q23
;q23 ab ab *l q23
;q23 a_ a_ *r q30

; step3. cpmpare each bit of two tapes
;q30 aa __ rr q31
;q31 aa __ rr q31
;q31 ab __ rr q_rej
;q31 ba __ rr q_rej
;q31 bb __ rr q32
;q32 bb __ rr q32
;q32 __ __ ** q_acc